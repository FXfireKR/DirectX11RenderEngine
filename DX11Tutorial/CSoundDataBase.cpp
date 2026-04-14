#include "pch.h"
#include "CSoundDataBase.h"

void CSoundDataBase::Initialize(const char* resourceRoot)
{
	m_strRoot = resourceRoot ? resourceRoot : "";
	m_bInit = true;
	m_bLoadedComplete = false;
}

bool CSoundDataBase::Load(const char* assetIndexPath, const char* objectsRoot)
{
	if (!m_bInit)
		return false;

	Clear();

	m_strObjectsRoot = m_strRoot + (objectsRoot ? objectsRoot : "");
	string strAssetIndexPath = m_strRoot + (assetIndexPath ? assetIndexPath : "");

	if (m_strObjectsRoot.empty())
		return false;

	if (!_LoadAssetIndex(strAssetIndexPath.c_str()))
		return false;

	if (!_LoadSoundsJson())
		return false;

	unordered_set<SoundID> duplicateChecker;
	for (auto event : m_mapEvents)
	{
		for (const SoundClipDef& clip : event.second.clips)
		{
			if (clip.bStream)
				continue;

			if (duplicateChecker.count(clip.soundID))
				continue;

			AudioLoadElemDesc desc{};
			desc.id = clip.soundID;
			desc.objectPath = clip.objectPath;
			desc.desc.b3D = event.second.playDesc.b3D;
			desc.desc.bLoop = event.second.playDesc.bLoop;
			desc.desc.bStream = false;

			m_queuePreLoad.push(desc);
			duplicateChecker.emplace(desc.id);
		}
	}

	m_bLoadedComplete = true;
	return true;
}

void CSoundDataBase::Clear()
{
	m_mapObjectPathByLogicalPath.clear();
	m_mapEvents.clear();
	m_mapEventNameByID.clear();
	m_mapBlockEventCache.clear();
}

const SoundEventDef* CSoundDataBase::FindEvent(SoundEventID eventID) const
{
	auto it = m_mapEvents.find(eventID);
	if (it == m_mapEvents.end())
		return nullptr;

	return &(it->second);
}

const SoundEventDef* CSoundDataBase::FindEvent(const char* eventName) const
{
	if (!eventName || !eventName[0])
		return nullptr;

	return FindEvent(fnv1a_64(eventName));
}

bool CSoundDataBase::ResolveEvent(SoundEventID eventID, ResolvedSound& outResolved) const
{
	outResolved = {};

	const SoundEventDef* pEvent = FindEvent(eventID);
	if (nullptr == pEvent)
		return false;

	const SoundClipDef* pClip = nullptr;
	if (!_PickWeightedClip(*pEvent, pClip) || !pClip)
		return false;

	outResolved.soundID = pClip->soundID;
	outResolved.playDesc = pEvent->playDesc;
	outResolved.playDesc.volume *= pClip->volumeMul;
	outResolved.playDesc.pitch *= pClip->pitchMul;

	if (pClip->bHasAttenuationDistance)
		outResolved.playDesc.maxDistance = pClip->attenuationDistance;

	return true;
}

bool CSoundDataBase::ResolveEvent(const char* eventName, ResolvedSound& outResolved) const
{
	if (!eventName || !eventName[0]) 
		return false;

	return ResolveEvent(fnv1a_64(eventName), outResolved);
}

bool CSoundDataBase::ResolveBlock(BLOCK_ID blockID, EBlockSoundUsage usage, ResolvedSound& outResolved) const
{
	outResolved = {};

	CachedBlockSoundEvents cache{};
	if (!_GetCachedBlockEventIDs(blockID, cache)) 
		return false;

	const size_t idx = static_cast<size_t>(usage);
	const SoundEventID eventID = cache.eventIDs[idx];
	if (eventID == 0) 
		return false;

	if (!ResolveEvent(eventID, outResolved)) 
		return false;

	// block sound는 강제로 3D / SFX
	outResolved.playDesc.b3D = true;
	outResolved.playDesc.bus = EAudioBus::SFX;
	return true;
}

bool CSoundDataBase::_LoadAssetIndex(const char* assetIndexPath)
{
	if (!assetIndexPath || !assetIndexPath[0]) 
		return false;

	if (!filesystem::exists(assetIndexPath)) 
		return false;

	ifstream ifs(assetIndexPath, ios::binary);
	if (!ifs.is_open()) 
		return false;

	string jsonText((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());

	Document doc;
	doc.Parse(jsonText.c_str());
	if (!doc.IsObject()) 
		return false;

	auto itObjects = doc.FindMember("objects");
	if (itObjects == doc.MemberEnd() || !itObjects->value.IsObject()) 
		return false;

	for (auto it = itObjects->value.MemberBegin(); it != itObjects->value.MemberEnd(); ++it)
	{
		const char* logicalPath = it->name.GetString();
		if (!logicalPath || !logicalPath[0]) 
			continue;

		const bool bSoundCatalog = (strcmp(logicalPath, "minecraft/sounds.json") == 0);
		const bool bSoundAsset = (strncmp(logicalPath, "minecraft/sounds/", 17) == 0);

		if (!bSoundCatalog && !bSoundAsset)
			continue;

		if (!it->value.IsObject()) 
			continue;

		auto itHash = it->value.FindMember("hash");
		if (itHash == it->value.MemberEnd() || !itHash->value.IsString()) 
			continue;

		string objectPath;
		if (!_MakeObjectPathFromHash(itHash->value.GetString(), objectPath)) 
			continue;

		m_mapObjectPathByLogicalPath.emplace(logicalPath, std::move(objectPath));
	}

	return !m_mapObjectPathByLogicalPath.empty();
}

bool CSoundDataBase::_LoadSoundsJson()
{
	auto it = m_mapObjectPathByLogicalPath.find("minecraft/sounds.json");
	if (it == m_mapObjectPathByLogicalPath.end()) 
		return false;

	if (!filesystem::exists(it->second)) 
		return false;

	ifstream ifs(it->second, ios::binary);
	if (!ifs.is_open()) 
		return false;

	string jsonText((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());

	Document doc;
	doc.Parse(jsonText.c_str());
	if (!doc.IsObject()) 
		return false;

	for (auto mem = doc.MemberBegin(); mem != doc.MemberEnd(); ++mem)
	{
		if (!mem->value.IsObject()) 
			continue;

		if (!_ParseEvent(mem->name.GetString(), mem->value))
			continue;
	}

	return !m_mapEvents.empty();
}

bool CSoundDataBase::_ParseEvent(const char* eventName, const Value& eventValue)
{
	auto itSounds = eventValue.FindMember("sounds");
	if (itSounds == eventValue.MemberEnd() || !itSounds->value.IsArray())
		return false;

	SoundEventDef newEvent{};
	newEvent.eventID = fnv1a_64(eventName);
	newEvent.eventName = eventName;
	newEvent.playDesc = _MakeDefaultPlayDesc(eventName);

	for (auto it = itSounds->value.Begin(); it != itSounds->value.End(); ++it)
	{
		SoundClipDef clip{};
		if (!_ParseClip(*it, clip))
			continue;

		newEvent.clips.push_back(std::move(clip));
	}

	if (newEvent.clips.empty())
		return false;

	m_mapEventNameByID.emplace(newEvent.eventID, newEvent.eventName);
	m_mapEvents.emplace(newEvent.eventID, std::move(newEvent));
	return true;
}

bool CSoundDataBase::_ParseClip(const Value& soundValue, SoundClipDef& outClip) const
{
	outClip = {};

	string rawName;

	if (soundValue.IsString())
	{
		rawName = soundValue.GetString();
	}
	else if (soundValue.IsObject())
	{
		auto itName = soundValue.FindMember("name");
		if (itName == soundValue.MemberEnd() || !itName->value.IsString())
			return false;

		rawName = itName->value.GetString();

		auto itVolume = soundValue.FindMember("volume");
		if (itVolume != soundValue.MemberEnd() && itVolume->value.IsNumber())
			outClip.volumeMul = itVolume->value.GetFloat();

		auto itPitch = soundValue.FindMember("pitch");
		if (itPitch != soundValue.MemberEnd() && itPitch->value.IsNumber())
			outClip.pitchMul = itPitch->value.GetFloat();

		auto itWeight = soundValue.FindMember("weight");
		if (itWeight != soundValue.MemberEnd() && itWeight->value.IsInt())
			outClip.weight = static_cast<uint16_t>(max(1, itWeight->value.GetInt()));

		auto itStream = soundValue.FindMember("stream");
		if (itStream != soundValue.MemberEnd() && itStream->value.IsBool())
			outClip.bStream = itStream->value.GetBool();

		auto itAttn = soundValue.FindMember("attenuation_distance");
		if (itAttn != soundValue.MemberEnd() && itAttn->value.IsNumber())
		{
			outClip.attenuationDistance = itAttn->value.GetFloat();
			outClip.bHasAttenuationDistance = true;
		}
	}
	else
	{
		return false;
	}

	if (!_MakeLogicalSoundPath(rawName.c_str(), outClip.logicalPath))
		return false;

	auto itObject = m_mapObjectPathByLogicalPath.find(outClip.logicalPath);
	if (itObject == m_mapObjectPathByLogicalPath.end())
		return false;

	outClip.objectPath = itObject->second;
	outClip.soundID = fnv1a_64(outClip.logicalPath);
	return true;
}

bool CSoundDataBase::_MakeObjectPathFromHash(const char* hash, string& outPath) const
{
	outPath.clear();
	if (!hash || strlen(hash) < 2) 
		return false;

	outPath = m_strObjectsRoot;
	if (!outPath.empty() && outPath.back() != '/' && outPath.back() != '\\')
		outPath += '/';

	outPath += hash[0];
	outPath += hash[1];
	outPath += '/';
	outPath += hash;
	return true;
}

bool CSoundDataBase::_MakeLogicalSoundPath(const char* rawName, string& outLogicalPath) const
{
	outLogicalPath.clear();
	if (!rawName || !rawName[0]) 
		return false;

	string name = rawName;
	string ns = "minecraft";
	string rel = name;

	const size_t colonPos = name.find(':');
	if (colonPos != string::npos)
	{
		ns = name.substr(0, colonPos);
		rel = name.substr(colonPos + 1);
	}

	if (rel.rfind("sounds/", 0) != 0)
		rel = "sounds/" + rel;

	if (rel.size() < 4 || rel.substr(rel.size() - 4) != ".ogg")
		rel += ".ogg";

	outLogicalPath = ns + "/" + rel;
	return true;
}

AudioPlayDesc CSoundDataBase::_MakeDefaultPlayDesc(const char* eventName) const
{
	AudioPlayDesc desc{};

	const string name = eventName ? eventName : "";

	if (name.rfind("music.", 0) == 0 || name.rfind("bgm.", 0) == 0)
	{
		desc.bus = EAudioBus::BGM;
		desc.b3D = false;
		desc.bLoop = true;
	}
	else if (name.rfind("ambient.", 0) == 0)
	{
		desc.bus = EAudioBus::AMBIENT;
		desc.b3D = false;
		desc.bLoop = (name.find(".loop") != string::npos);
	}
	else if (name.rfind("ui.", 0) == 0)
	{
		desc.bus = EAudioBus::SFX;
		desc.b3D = false;
		desc.bLoop = false;
	}
	else
	{
		desc.bus = EAudioBus::SFX;
		desc.b3D = true;
		desc.bLoop = false;
	}

	return desc;
}

bool CSoundDataBase::_PickWeightedClip(const SoundEventDef& eventDef, const SoundClipDef*& outClip) const
{
	outClip = nullptr;
	if (eventDef.clips.empty()) 
		return false;

	uint32_t totalWeight = 0;
	for (const SoundClipDef& clip : eventDef.clips)
		totalWeight += max<uint32_t>(1, clip.weight);

	if (totalWeight == 0)
	{
		outClip = &eventDef.clips.front();
		return true;
	}

	const uint32_t pick = 1 + (rand() % totalWeight);
	uint32_t acc = 0;

	for (const SoundClipDef& clip : eventDef.clips)
	{
		acc += max<uint32_t>(1, clip.weight);
		if (pick <= acc)
		{
			outClip = &clip;
			return true;
		}
	}

	outClip = &eventDef.clips.back();
	return true;
}

bool CSoundDataBase::_GetCachedBlockEventIDs(BLOCK_ID blockID, CachedBlockSoundEvents& outCache) const
{
	auto itCache = m_mapBlockEventCache.find(blockID);
	if (itCache != m_mapBlockEventCache.end())
	{
		outCache = itCache->second;
		return true;
	}

	CachedBlockSoundEvents newCache{};
	newCache.bCached = true;

	const char* profile = BlockDB.GetSoundProfile(blockID);
	if (!profile || !profile[0])
	{
		m_mapBlockEventCache.emplace(blockID, newCache);
		outCache = newCache;
		return false;
	}

	for (size_t i = 0; i < BLOCK_SOUND_USAGE_COUNT; ++i)
	{
		const auto use = static_cast<EBlockSoundUsage>(i);
		const SoundEventID eventID = _MakeBlockEventID(profile, use);

		if (FindEvent(eventID))
			newCache.eventIDs[i] = eventID;
		else
			newCache.eventIDs[i] = 0;
	}

	m_mapBlockEventCache.emplace(blockID, newCache);
	outCache = newCache;
	return true;
}

SoundEventID CSoundDataBase::_MakeBlockEventID(const char* profileName, EBlockSoundUsage usage) const
{
	if (!profileName || !profileName[0]) 
		return 0;

	string eventName = "block.";
	eventName += profileName;
	eventName += ".";
	eventName += _GetBlockUseSuffix(usage);

	return fnv1a_64(eventName);
}

const char* CSoundDataBase::_GetBlockUseSuffix(EBlockSoundUsage usage) const
{
	switch (usage)
	{
		case EBlockSoundUsage::BREAK: return "break";
		case EBlockSoundUsage::PLACE: return "place";
		case EBlockSoundUsage::STEP:  return "step";
		case EBlockSoundUsage::HIT:   return "hit";
		case EBlockSoundUsage::FALL:  return "fall";
		default: return "";
	}
}
