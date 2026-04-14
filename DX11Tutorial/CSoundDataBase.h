#pragma once
#include "AudioType.h"

enum class EBlockSoundUsage : uint8_t
{
	BREAK = 0,
	PLACE,
	STEP,
	HIT,
	FALL,
	COUNT,
};
constexpr size_t BLOCK_SOUND_USAGE_COUNT = static_cast<size_t>(EBlockSoundUsage::COUNT);

struct SoundClipDef
{
	SoundID soundID = 0;

	string logicalPath; // ex) minecraft/sounds/block/stone/break1.ogg

	string objectPath; // ex) <objectsRoot>/ab/hash...

	uint8_t weight = 1;
	float volumeMul = 1.f;
	float pitchMul = 1.f;

	float attenuationDistance = 16.f;
	bool bHasAttenuationDistance = false;

	bool bStream = false;
};

struct SoundEventDef
{
	SoundEventID eventID = 0;
	string eventName;

	AudioPlayDesc playDesc;
	vector<SoundClipDef> clips;
};

struct ResolvedSound
{
	SoundID soundID = 0;
	AudioPlayDesc playDesc{};
};

struct CachedBlockSoundEvents
{
	bool bCached = false;
	array<SoundEventID, BLOCK_SOUND_USAGE_COUNT> eventIDs{};
};

class CSoundDataBase
{
public:
	CSoundDataBase() = default;
	~CSoundDataBase() = default;

	void Initialize(const char* resourceRoot);
	bool Load(const char* assetIndexPath, const char* objectsRoot);
	void Clear();

	const SoundEventDef* FindEvent(SoundEventID eventID) const;
	const SoundEventDef* FindEvent(const char* eventName) const;

	bool ResolveEvent(SoundEventID eventID, ResolvedSound& outResolved) const;
	bool ResolveEvent(const char* eventName, ResolvedSound& outResolved) const;

	bool ResolveBlock(BLOCK_ID blockID, EBlockSoundUsage usage, ResolvedSound& outResolved) const;

public:
	inline bool IsLoaded() const { return m_bLoadedComplete; }
	inline queue<AudioLoadElemDesc>& GetPreLoadQueue() { return m_queuePreLoad; }

private:
	bool _LoadAssetIndex(const char* assetIndexPath);
	bool _LoadSoundsJson();
	
	bool _ParseEvent(const char* eventName, const Value& eventValue);
	bool _ParseClip(const Value& soundValue, SoundClipDef& outClip) const;

	bool _MakeObjectPathFromHash(const char* hash, string& outPath) const;
	bool _MakeLogicalSoundPath(const char* rawName, string& outLogicalPath) const;

	AudioPlayDesc _MakeDefaultPlayDesc(const char* eventName) const;
	bool _PickWeightedClip(const SoundEventDef& eventDef, const SoundClipDef*& outClip) const;
	
	bool _GetCachedBlockEventIDs(BLOCK_ID blockID, CachedBlockSoundEvents& outCache) const;
	SoundEventID _MakeBlockEventID(const char* profileName, EBlockSoundUsage usage) const;
	const char* _GetBlockUseSuffix(EBlockSoundUsage usage) const;

private:
	string m_strRoot;
	string m_strObjectsRoot;
	bool m_bInit = false;
	bool m_bLoadedComplete = false;

	unordered_map<string, string> m_mapObjectPathByLogicalPath;
	unordered_map<SoundEventID, SoundEventDef> m_mapEvents;
	unordered_map<SoundEventID, string> m_mapEventNameByID;

	queue<AudioLoadElemDesc> m_queuePreLoad;

	mutable unordered_map<BLOCK_ID, CachedBlockSoundEvents> m_mapBlockEventCache;
};