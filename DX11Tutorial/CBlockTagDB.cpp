#include "pch.h"
#include "CBlockTagDB.h"
#include "CBlockDefDB.h"
#include "IFileWrapper.h"

void CBlockTagDB::Initialize(const char* resourceRoot, const CBlockDefDB* pBlockDefDB)
{
	m_strResourceRoot = (resourceRoot ? resourceRoot : "");
	m_pBlockDefDB = pBlockDefDB;
}

bool CBlockTagDB::Load()
{
	Clear();

	if (m_pBlockDefDB == nullptr)
	{
		assert(false && "CBlockTagDB::Load - BlockDefDB is null");
		return false;
	}

	if (!_LoadTagFiles())
	{
		assert(false && "CBlockTagDB::_LoadTagFiles failed");
		return false;
	}

	if (!_BuildRuntimeTags())
	{
		assert(false && "CBlockTagDB::_BuildRuntimeTags failed");
		return false;
	}

	_BuildReverseLookup();
	return true;
}

void CBlockTagDB::Clear()
{
	m_mapTagRaws.clear();
	m_mapTagDefs.clear();
	m_mapHashToTagName.clear();
	m_vecBlockTagHashes.clear();
}

bool CBlockTagDB::HasTag(BLOCK_ID blockID, const char* tagName) const
{
	if (!tagName)
		return false;

	return HasTagHash(blockID, _HashTag(tagName));
}

bool CBlockTagDB::HasTagHash(BLOCK_ID blockID, BLOCK_TAG_HASH tagHash) const
{
	if (blockID == INVALID_BLOCK_ID || blockID >= m_vecBlockTagHashes.size())
		return false;

	const auto& tagSet = m_vecBlockTagHashes[blockID];
	return (tagSet.find(tagHash)) != tagSet.end();
}

const BlockTagDef* CBlockTagDB::FindTagDef(const char* tagName) const
{
	if (!tagName)
		return nullptr;

	auto it = m_mapTagDefs.find(tagName);
	if (it == m_mapTagDefs.end())
		return nullptr;

	return &it->second;
}

const BlockTagRaw* CBlockTagDB::FindTagRaw(const char* tagName) const
{
	if (!tagName)
		return nullptr;

	auto it = m_mapTagRaws.find(tagName);
	if (it == m_mapTagRaws.end())
		return nullptr;

	return &it->second;
}

BLOCK_TAG_HASH CBlockTagDB::FindTagHash(const char* tagName) const
{
	if (!tagName)
		return 0;

	return _HashTag(tagName);
}

bool CBlockTagDB::_LoadTagFiles()
{
	const filesystem::path dirPath = filesystem::path(m_strResourceRoot) / "data" / "minecraft" / "tags" / "blocks";

	if (!filesystem::exists(dirPath))
		return false;

	for (const auto& entry : filesystem::recursive_directory_iterator(dirPath))
	{
		if (!entry.is_regular_file())
			continue;

		const filesystem::path filePath = entry.path();
		if (filePath.extension() != ".json")
			continue;

		if (!_LoadOneTagFile(filePath))
			return false;
	}

	return true;
}

bool CBlockTagDB::_LoadOneTagFile(const filesystem::path& filePath)
{
	rapidjson::Document doc;
	if (!_ReadJson(filePath, doc))
		return false;

	const filesystem::path rootPath = filesystem::path(m_strResourceRoot) / "data" / "minecraft" / "tags" / "blocks";

	filesystem::path relativePath = filesystem::relative(filePath, rootPath);
	string tagName = _NormalizeTagPathToName(relativePath);

	BlockTagRaw raw;
	if (!_ParseTagRaw(doc, tagName, raw))
		return false;

	raw.bLoaded = true;
	m_mapTagRaws[raw.name] = std::move(raw);
	return true;
}

bool CBlockTagDB::_ReadJson(const filesystem::path& path, rapidjson::Document& outDoc) const
{
	stringstream ss;
	if (!IFileStreamWrapper::ReadAllStream(path.string(), ss))
		return false;

	const string jsonText = ss.str();
	outDoc.Parse(jsonText.c_str());

	if (outDoc.HasParseError())
		return false;

	if (!outDoc.IsObject())
		return false;

	return true;
}

bool CBlockTagDB::_ParseTagRaw(const rapidjson::Value& root, const string& tagName, BlockTagRaw& outRaw)
{
	outRaw = BlockTagRaw{};
	outRaw.name = tagName;

	if (!root.IsObject())
		return false;

	if (!root.HasMember("values") || !root["values"].IsArray())
		return false;

	const rapidjson::Value& values = root["values"];
	outRaw.values.reserve(values.Size());

	for (rapidjson::SizeType i = 0; i < values.Size(); ++i)
	{
		if (!values[i].IsString())
			continue;

		outRaw.values.push_back(values[i].GetString());
	}

	return true;
}

bool CBlockTagDB::_BuildRuntimeTags()
{
	unordered_set<string> visiting;

	for (const auto& pair : m_mapTagRaws)
	{
		const string& tagName = pair.first;

		BlockTagDef def;
		if (!_ResolveTag(tagName, def, visiting))
			return false;

		def.bResolved = true;
		m_mapTagDefs[tagName] = std::move(def);
		m_mapHashToTagName[_HashTag(tagName.c_str())] = tagName;
	}

	return true;
}

bool CBlockTagDB::_ResolveTag(const string& tagName, BlockTagDef& outDef, unordered_set<string>& visiting)
{
	auto itResolved = m_mapTagDefs.find(tagName);
	if (itResolved != m_mapTagDefs.end() && itResolved->second.bResolved)
	{
		outDef = itResolved->second;
		return true;
	}

	if (visiting.find(tagName) != visiting.end())
	{
		assert(false && "BlockTag cycle detected");
		return false;
	}

	auto itRaw = m_mapTagRaws.find(tagName);
	if (itRaw == m_mapTagRaws.end())
	{
		assert(false && "BlockTag raw not found");
		return false;
	}

	visiting.insert(tagName);

	BlockTagDef result;
	result.name = tagName;

	unordered_set<BLOCK_ID> uniqueIDs;

	for (const string& value : itRaw->second.values)
	{
		if (_IsTagReference(value))
		{
			const string refTagName = _StripTagPrefix(value);

			BlockTagDef refDef;
			if (!_ResolveTag(refTagName, refDef, visiting))
			{
				visiting.erase(tagName);
				return false;

				for (BLOCK_ID id : refDef.blockIDs)
					uniqueIDs.insert(id);
			}
		}
		else
		{
			const BLOCK_ID blockID = m_pBlockDefDB->FindBlockID(value.c_str());
			if (blockID == INVALID_BLOCK_ID)
			{
				// 원본 태그에 아직 registry에 없는 블럭이 있을 수 있는데 이건 스킵
				continue;
			}

			uniqueIDs.insert(blockID);
		}
	}

	result.blockIDs.reserve(uniqueIDs.size());
	for (BLOCK_ID id : uniqueIDs)
		result.blockIDs.push_back(id);

	std::sort(result.blockIDs.begin(), result.blockIDs.end());

	visiting.erase(tagName);

	result.bResolved = true;
	outDef = result;
	m_mapTagDefs[tagName] = result;
	return true;
}

void CBlockTagDB::_BuildReverseLookup()
{
	const vector<BlockDef>& blockDefs = m_pBlockDefDB->GetBlockDefs();
	m_vecBlockTagHashes.clear();
	m_vecBlockTagHashes.resize(blockDefs.size());

	for (const auto& pair : m_mapTagDefs)
	{
		const string& tagName = pair.first;
		const BlockTagDef& def = pair.second;
		const BLOCK_TAG_HASH tagHash = _HashTag(tagName.c_str());

		for (BLOCK_ID blockID : def.blockIDs)
		{
			if (blockID == INVALID_BLOCK_ID || blockID >= m_vecBlockTagHashes.size())
				continue;

			m_vecBlockTagHashes[blockID].insert(tagHash);
		}
	}
}

bool CBlockTagDB::_IsTagReference(const string& value)
{
	return (!value.empty() && value[0] == '#');
}

string CBlockTagDB::_NormalizeTagPathToName(const filesystem::path& filePath)
{
	filesystem::path normalized = filePath;
	normalized.replace_extension("");

	string s = normalized.generic_string();
	return "minecraft:" + s;
}

string CBlockTagDB::_StripTagPrefix(const string& value)
{
	if (_IsTagReference(value))
		return value.substr(1);

	return value;
}

BLOCK_TAG_HASH CBlockTagDB::_HashTag(const char* tag)
{
	if (!tag)
		return 0;

	uint32_t hash = 2166136261u;

	while (*tag)
	{
		hash ^= static_cast<uint8_t>(*tag);
		hash *= 16777619u;
		++tag;
	}

	return hash;
}