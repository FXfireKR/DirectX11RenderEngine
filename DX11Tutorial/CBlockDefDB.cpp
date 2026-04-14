#include "pch.h"
#include "CBlockDefDB.h"
#include "IFileWrapper.h"

void CBlockDefDB::Initialize(const char* resourceRoot)
{
	m_strResourceRoot = resourceRoot;
}

bool CBlockDefDB::Load()
{
	Clear();

	if (!_LoadRegistry())
	{
		assert(false && "CBlockDefDB::_LoadRegistry failed");
		return false;
	}

	if (!_LoadTemplates())
	{
		assert(false && "CBlockDefDB::_LoadTemplates failed");
		return false;
	}

	if (!_LoadBlocks())
	{
		assert(false && "CBlockDefDB::_LoadBlocks failed");
		return false;
	}

	if (!_BuildRuntimeDefs())
	{
		assert(false && "CBlockDefDB::_BuildRuntimeDefs failed");
		return false;
	}

	// burn over!
	m_mapTemplates.clear();
	m_mapNamedDefs.clear();

	return true;
}

void CBlockDefDB::Clear()
{
	m_mapNameToID.clear();
	m_vecIDToName.clear();

	m_mapTemplates.clear();
	m_mapNamedDefs.clear();

	m_vecBlockDefs.clear();
}

const BlockDef* CBlockDefDB::GetBlockDef(BLOCK_ID blockID) const
{
	if (!IsValidBlockID(blockID)) 
		return nullptr;

	const BlockDef& def = m_vecBlockDefs[blockID];
	return def.bValid ? &def : nullptr;
}

const BlockDef* CBlockDefDB::FindBlockDef(const char* blockName) const
{
	BLOCK_ID blockID = FindBlockID(blockName);
	if (blockID == INVALID_BLOCK_ID)
		return nullptr;

	return GetBlockDef(blockID);
}

const BlockDefRaw* CBlockDefDB::FindNameDef(const char* defName) const
{
	if (!defName)
		return nullptr;

	auto itBlock = m_mapNamedDefs.find(defName);
	if (itBlock != m_mapNamedDefs.end())
		return &itBlock->second;

	auto itTemplate = m_mapTemplates.find(defName);
	if (itTemplate != m_mapTemplates.end())
		return &itTemplate->second;

	return nullptr;
}

const BLOCK_ID CBlockDefDB::FindBlockID(const char* blockName) const
{
	if (!blockName)
		return INVALID_BLOCK_ID;

	auto it = m_mapNameToID.find(blockName);
	if (it == m_mapNameToID.end())
		return INVALID_BLOCK_ID;

	return it->second;
}

const char* CBlockDefDB::FindBlockName(BLOCK_ID blockID) const
{
	if (!IsValidBlockID(blockID))
		return nullptr;

	return m_vecIDToName[blockID].c_str();
}

const bool CBlockDefDB::IsValidBlockID(BLOCK_ID blockID) const
{
	return (blockID != INVALID_BLOCK_ID && blockID < m_vecBlockDefs.size());
}

const bool CBlockDefDB::IsAir(BLOCK_ID blockID) const
{
	const BlockDef* pBlockDef = GetBlockDef(blockID);
	return pBlockDef ? pBlockDef->properties.bAir : false;
}

const bool CBlockDefDB::IsOpaque(BLOCK_ID blockID) const
{
	const BlockDef* pBlockDef = GetBlockDef(blockID);
	return pBlockDef ? pBlockDef->properties.bOpaque : false;
}

const bool CBlockDefDB::IsSolid(BLOCK_ID blockID) const
{
	const BlockDef* pBlockDef = GetBlockDef(blockID);
	return pBlockDef ? pBlockDef->properties.bSolid : false;
}

const bool CBlockDefDB::IsFullCube(BLOCK_ID blockID) const
{
	const BlockDef* pBlockDef = GetBlockDef(blockID);
	return pBlockDef ? pBlockDef->properties.bFullCube : false;
}

float CBlockDefDB::GetHardness(BLOCK_ID blockID) const
{
	const BlockDef* pBlockDef = GetBlockDef(blockID);
	return pBlockDef ? pBlockDef->gameplay.fHardness : 0.f;
}

const char* CBlockDefDB::GetsoundProfile(BLOCK_ID blockID) const
{
	const BlockDef* pBlockDef = GetBlockDef(blockID);
	if (!pBlockDef)
		return nullptr;

	if (pBlockDef->sound.profile.empty())
		return nullptr;

	return pBlockDef->sound.profile.c_str();
}

const BLOCK_RENDER_LAYER CBlockDefDB::GetRenderLayer(BLOCK_ID blockID) const
{
	const BlockDef* pBlockDef = GetBlockDef(blockID);
	if (!pBlockDef)
		return BLOCK_RENDER_LAYER::INVISIBLE_LAYER;

	return pBlockDef->render.layer;
}

const BLOCK_COLLISION_TYPE CBlockDefDB::GetCollisionType(BLOCK_ID blockID) const
{
	const BlockDef* pBlockDef = GetBlockDef(blockID);
	if (!pBlockDef)
		return BLOCK_COLLISION_TYPE::NONE;

	return pBlockDef->collision.colType;
}

const bool CBlockDefDB::HasCollision(BLOCK_ID blockID) const
{
	const BlockDef* pBlockDef = GetBlockDef(blockID);
	if (!pBlockDef)
		return false;

	return pBlockDef->collision.colType != BLOCK_COLLISION_TYPE::NONE;
}

const bool CBlockDefDB::UseAmbientOcclusion(BLOCK_ID blockID) const
{
	const BlockDef* pBlockDef = GetBlockDef(blockID);
	if (!pBlockDef)
		return false;

	return pBlockDef->render.bAmbientOcclusion;
}

const bool CBlockDefDB::CanCullSameBlockFace(BLOCK_ID blockID) const
{
	const BlockDef* pBlockDef = GetBlockDef(blockID);
	if (!pBlockDef)
		return false;

	return pBlockDef->render.bCullSameBlockFace;
}

const uint8_t CBlockDefDB::GetLightEmission(BLOCK_ID blockID) const
{
	const BlockDef* pBlockDef = GetBlockDef(blockID);
	if (!pBlockDef)
		return false;

	return pBlockDef->render.lightEmissions;
}

const bool CBlockDefDB::IsFaceOccluder(BLOCK_ID blockID) const
{
	const BlockDef* pBlockDef = GetBlockDef(blockID);
	if (!pBlockDef)
		return false;

	if (pBlockDef->properties.bAir)
		return false;

	if (!pBlockDef->properties.bFullCube)
		return false;

	switch (pBlockDef->render.layer)
	{
		case BLOCK_RENDER_LAYER::CUTOUT_LAYER:
		case BLOCK_RENDER_LAYER::TRANSLUCENT_LAYER:
		case BLOCK_RENDER_LAYER::INVISIBLE_LAYER:
			return false;
	}

	return true;
}

bool CBlockDefDB::_LoadRegistry()
{
	const filesystem::path filePath 
		= filesystem::path(m_strResourceRoot) / "assets" / "minecraft" / "blocks" / "block_registry.json";
	
	rapidjson::Document doc;
	if (!_ReadJson(filePath, doc))
		return false;

	if (!doc.IsObject())
		return false;

	if (!doc.HasMember("entries") || !doc["entries"].IsObject())
		return false;

	const rapidjson::Value& entries = doc["entries"];

	BLOCK_ID maxID = 0;

	for (auto it = entries.MemberBegin(); it != entries.MemberEnd(); ++it)
	{
		if (!it->name.IsString())
			continue;

		if (!it->value.IsUint())
			continue;

		const string blockName = it->name.GetString();
		const BLOCK_ID blockID = static_cast<BLOCK_ID>(it->value.GetUint());

		if (m_mapNameToID.find(blockName) != m_mapNameToID.end())
		{
			assert(false && "duplicate block registry name");
			return false;
		}

		m_mapNameToID.emplace(blockName, blockID);
		maxID = std::max(maxID, blockID);
	}

	m_vecIDToName.resize(static_cast<size_t>(maxID) + 1);
	m_vecBlockDefs.resize(static_cast<size_t>(maxID) + 1);

	for (const auto& pair : m_mapNameToID)
	{
		m_vecIDToName[pair.second] = pair.first;
		m_vecBlockDefs[pair.second].name = pair.first;
		m_vecBlockDefs[pair.second].blockID = pair.second;
	}

	return true;
}

bool CBlockDefDB::_LoadTemplates()
{
	const filesystem::path dirPath =
		filesystem::path(m_strResourceRoot) / "assets" / "minecraft" / "blocks";

	if (!filesystem::exists(dirPath))
		return false;

	for (const auto& entry : filesystem::recursive_directory_iterator(dirPath))
	{
		if (!entry.is_regular_file())
			continue;

		const filesystem::path filePath = entry.path();
		
		if (filePath.extension() != ".json")
			continue;

		if (filePath.filename() == "block_registry.json")
			continue;

		rapidjson::Document doc;
		if (!_ReadJson(filePath, doc))
			return false;

		if (!doc.IsObject())
			return false;

		string type = "block";
		if (doc.HasMember("type") && doc["type"].IsString())
			type = doc["type"].GetString();

		const bool bIsTemplate = (type == "template" || type == "block_template");
		if (!bIsTemplate)
			continue;

		if (!_LoadOneDefFile(filePath, doc))
			return false;
	}

	return true;
}

bool CBlockDefDB::_LoadBlocks()
{
	const filesystem::path dirPath = filesystem::path(m_strResourceRoot) / "assets" / "minecraft" / "blocks";

	if (!filesystem::exists(dirPath))
		return false;

	for (const auto& entry : filesystem::recursive_directory_iterator(dirPath))
	{
		if (!entry.is_regular_file())
			continue;

		const filesystem::path filePath = entry.path();

		if (filePath.extension() != ".json")
			continue;

		if (filePath.filename() == "block_registry.json")
			continue;

		rapidjson::Document doc;
		if (!_ReadJson(filePath, doc))
			return false;

		if (!doc.IsObject())
			return false;

		string type = "block";
		if (doc.HasMember("type") && doc["type"].IsString())
			type = doc["type"].GetString();

		const bool bIsTemplate = (type == "template" || type == "block_template");
		if (bIsTemplate)
			continue;

		if (!_LoadOneDefFile(filePath, doc))
			return false;
	}

	return true;
}

bool CBlockDefDB::_LoadOneDefFile(const filesystem::path& filePath, rapidjson::Document& doc)
{
	BlockDefRaw raw;
	if (!_ParseBlockDefRaw(doc, raw))
		return false;

	raw.bLoaded = true;

	if (raw.name.empty())
	{
		assert(false && "BlockDefRaw name is empty");
		return false;
	}

	if (raw.bIsTemplate)
	{
		m_mapTemplates[raw.name] = std::move(raw);
	}
	else // (!raw.bIsTemplate)
	{
		if (FindBlockID(raw.name.c_str()) == INVALID_BLOCK_ID)
		{
			assert(false && "block def not found in registry");
			return false;
		}

		m_mapNamedDefs[raw.name] = std::move(raw);
	}

	return true;
}

const bool CBlockDefDB::_ReadJson(const filesystem::path& path, rapidjson::Document& outDoc) const
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

const bool CBlockDefDB::_ParseBlockDefRaw(const rapidjson::Value& root, BlockDefRaw& outRaw) const
{
	outRaw = BlockDefRaw{};

	if (!root.IsObject())
		return false;

	string type = "block";
	if (root.HasMember("type") && root["type"].IsString())
		type = root["type"].GetString();

	outRaw.bIsTemplate = (type == "template" || type == "block_template");

	if (!root.HasMember("name") || !root["name"].IsString())
		return false;

	outRaw.name = root["name"].GetString();

	if (root.HasMember("parent") && root["parent"].IsString())
		outRaw.parent = root["parent"].GetString();

	if (root.HasMember("stateSource") && root["stateSource"].IsString())
		outRaw.stateSource = root["stateSource"].GetString();

	if (root.HasMember("soundProfile") && root["soundProfile"].IsString())
		outRaw.sound.profile = root["soundProfile"].GetString();

	if (root.HasMember("properties") && root["properties"].IsObject())
	{
		const rapidjson::Value& props = root["properties"];

		if (props.HasMember("air") && props["air"].IsBool())
			outRaw.properties.air = props["air"].GetBool();

		if (props.HasMember("opaque") && props["opaque"].IsBool())
			outRaw.properties.opaque = props["opaque"].GetBool();

		if (props.HasMember("solid") && props["solid"].IsBool())
			outRaw.properties.solid = props["solid"].GetBool();

		if (props.HasMember("fullCube") && props["fullCube"].IsBool())
			outRaw.properties.fullCube = props["fullCube"].GetBool();
		else if (props.HasMember("fullcube") && props["fullcube"].IsBool())
			outRaw.properties.fullCube = props["fullcube"].GetBool();
	}

	if (root.HasMember("render") && root["render"].IsObject())
	{
		const rapidjson::Value& render = root["render"];

		if (render.HasMember("layer") && render["layer"].IsString())
		{
			BLOCK_RENDER_LAYER layer{};
			if (_ParseRenderLayer(render["layer"].GetString(), layer))
				outRaw.render.layer = layer;
		}

		if (render.HasMember("ambientOcclusion") && render["ambientOcclusion"].IsBool())
			outRaw.render.ambientOcclusion = render["ambientOcclusion"].GetBool();

		if (render.HasMember("cullSameBlockFace") && render["cullSameBlockFace"].IsBool())
			outRaw.render.cullSameBlockFace = render["cullSameBlockFace"].GetBool();

		if (render.HasMember("lightEmissions") && render["lightEmissions"].IsInt())
			outRaw.render.lightEmissions = static_cast<uint8_t>(render["lightEmissions"].GetUint());
	}

	if (root.HasMember("collision") && root["collision"].IsObject())
	{
		const rapidjson::Value& collision = root["collision"];

		if (collision.HasMember("type") && collision["type"].IsString())
		{
			BLOCK_COLLISION_TYPE colType{};
			if (_ParseCollisionType(collision["type"].GetString(), colType))
				outRaw.collision.colType = colType;
		}
	}

	if (root.HasMember("gameplay") && root["gameplay"].IsObject())
	{
		const rapidjson::Value& gameplay = root["gameplay"];

		if (gameplay.HasMember("hardness") && gameplay["hardness"].IsNumber())
			outRaw.gameplay.hardness = static_cast<float>(gameplay["hardness"].GetDouble());

		if (gameplay.HasMember("blastResistance") && gameplay["blastResistance"].IsNumber())
			outRaw.gameplay.blastResistance = static_cast<float>(gameplay["blastResistance"].GetDouble());

		if (gameplay.HasMember("requiresCorrectTool") && gameplay["requiresCorrectTool"].IsBool())
			outRaw.gameplay.bRequiresCorrectTool = gameplay["requiresCorrectTool"].GetBool();

		if (gameplay.HasMember("requiredToolTag") && gameplay["requiredToolTag"].IsString())
			outRaw.gameplay.requiredToolTag = string(gameplay["requiredToolTag"].GetString());

		if (gameplay.HasMember("requiredToolTier") && gameplay["requiredToolTier"].IsInt())
			outRaw.gameplay.requiredToolTier = gameplay["requiredToolTier"].GetInt();
	}

	return true;
}

bool CBlockDefDB::_BuildRuntimeDefs()
{
	unordered_set<string> visiting;

	for (const auto& pair : m_mapNameToID)
	{
		const string& blockName = pair.first;
		const BLOCK_ID blockID = pair.second;

		BlockDef built;
		if (!_ResolveOne(blockName, built, visiting))
			return false;

		built.blockID = blockID;
		built.name = blockName;
		built.bValid = true;

		m_vecBlockDefs[blockID] = std::move(built);
	}

	return true;
}

const bool CBlockDefDB::_ResolveOne(const string& blockName, BlockDef& outDef, unordered_set<string>& outVisiting) const
{
	if (outVisiting.find(blockName) != outVisiting.end())
	{
		assert(false && "BlockDef inheritance cycle detected");
		return false;
	}

	const BlockDefRaw* pBlockDefRaw = _FindRaw(blockName);
	if (!pBlockDefRaw)
	{
		assert(false && "BlockDef raw not found");
		return false;
	}

	outVisiting.insert(blockName);

	BlockDef result;
	result.name = pBlockDefRaw->name;

	if (!pBlockDefRaw->parent.empty())
	{
		if (!_ResolveOne(pBlockDefRaw->parent, result, outVisiting))
		{
			outVisiting.erase(blockName);
			return false;
		}
	}

	_ApplyRawToRuntime(*pBlockDefRaw, result);
	_ApplyDefaultFixup(result);

	outVisiting.erase(blockName);

	outDef = std::move(result);
	return true;
}

void CBlockDefDB::_ApplyRawToRuntime(const BlockDefRaw& raw, BlockDef& outDef)
{
	outDef.name = raw.name;

	if (raw.stateSource.has_value())
		outDef.stateSource = raw.stateSource.value();

	if (raw.sound.profile.has_value())
		outDef.sound.profile = raw.sound.profile.value();

	if (raw.properties.air.has_value())
		outDef.properties.bAir = raw.properties.air.value();

	if (raw.properties.opaque.has_value())
		outDef.properties.bOpaque = raw.properties.opaque.value();

	if (raw.properties.solid.has_value())
		outDef.properties.bSolid = raw.properties.solid.value();

	if (raw.properties.fullCube.has_value())
		outDef.properties.bFullCube = raw.properties.fullCube.value();

	if (raw.render.layer.has_value())
		outDef.render.layer = raw.render.layer.value();

	if (raw.render.ambientOcclusion.has_value())
		outDef.render.bAmbientOcclusion = raw.render.ambientOcclusion.value();

	if (raw.render.cullSameBlockFace.has_value())
		outDef.render.bCullSameBlockFace = raw.render.cullSameBlockFace.value();

	if (raw.render.lightEmissions.has_value())
		outDef.render.lightEmissions = raw.render.lightEmissions.value();

	if (raw.collision.colType.has_value())
		outDef.collision.colType = raw.collision.colType.value();

	if (raw.gameplay.hardness.has_value())
		outDef.gameplay.fHardness = raw.gameplay.hardness.value();

	if (raw.gameplay.blastResistance.has_value())
		outDef.gameplay.fBlastResistance = raw.gameplay.blastResistance.value();

	if (raw.gameplay.bRequiresCorrectTool.has_value())
		outDef.gameplay.bRequiresCorrectTool = raw.gameplay.bRequiresCorrectTool.value();

	if (raw.gameplay.requiredToolTag.has_value())
		outDef.gameplay.requiredToolTag = raw.gameplay.requiredToolTag.value();

	if (raw.gameplay.requiredToolTier.has_value())
		outDef.gameplay.requiredToolTier = raw.gameplay.requiredToolTier.value();
}

void CBlockDefDB::_ApplyDefaultFixup(BlockDef& outDef)
{
	if (outDef.properties.bAir)
	{
		outDef.properties.bOpaque = false;
		outDef.properties.bSolid = false;
		outDef.properties.bFullCube = false;

		outDef.render.layer = BLOCK_RENDER_LAYER::INVISIBLE_LAYER;
		outDef.render.bAmbientOcclusion = false;

		outDef.collision.colType = BLOCK_COLLISION_TYPE::NONE;
	}

	if (outDef.properties.bFullCube && outDef.collision.colType == BLOCK_COLLISION_TYPE::NONE)
	{
		outDef.collision.colType = BLOCK_COLLISION_TYPE::FULL_CUBE;
	}

	if (!outDef.properties.bSolid && outDef.collision.colType == BLOCK_COLLISION_TYPE::FULL_CUBE)
	{
		outDef.collision.colType = BLOCK_COLLISION_TYPE::NONE;
	}

	if (outDef.properties.bOpaque && !outDef.properties.bFullCube)
	{
		outDef.properties.bOpaque = false;
	}
}

bool CBlockDefDB::_ParseRenderLayer(const char* strLayer, BLOCK_RENDER_LAYER& outLayer)
{
	if (!strLayer)
		return false;

	if (strcmp(strLayer, "opaque") == 0)
	{
		outLayer = BLOCK_RENDER_LAYER::OPAQUE_LAYER;
		return true;
	}

	if (strcmp(strLayer, "cutout") == 0)
	{
		outLayer = BLOCK_RENDER_LAYER::CUTOUT_LAYER;
		return true;
	}

	if (strcmp(strLayer, "translucent") == 0)
	{
		outLayer = BLOCK_RENDER_LAYER::TRANSLUCENT_LAYER;
		return true;
	}

	if (strcmp(strLayer, "invisible") == 0)
	{
		outLayer = BLOCK_RENDER_LAYER::INVISIBLE_LAYER;
		return true;
	}

	return false;
}

bool CBlockDefDB::_ParseCollisionType(const char* strType, BLOCK_COLLISION_TYPE& outType)
{
	if (!strType)
		return false;

	if (strcmp(strType, "none") == 0)
	{
		outType = BLOCK_COLLISION_TYPE::NONE;
		return true;
	}

	if (strcmp(strType, "full_cube") == 0)
	{
		outType = BLOCK_COLLISION_TYPE::FULL_CUBE;
		return true;
	}

	if (strcmp(strType, "slab") == 0)
	{
		outType = BLOCK_COLLISION_TYPE::SLAB;
		return true;
	}

	if (strcmp(strType, "custom") == 0)
	{
		outType = BLOCK_COLLISION_TYPE::CUSTOM;
		return true;
	}

	return false;
}

const BlockDefRaw* CBlockDefDB::_FindRaw(const string& name) const
{
	auto itBlock = m_mapNamedDefs.find(name);
	if (itBlock != m_mapNamedDefs.end())
		return &itBlock->second;

	auto itTemplate = m_mapTemplates.find(name);
	if (itTemplate != m_mapTemplates.end())
		return &itTemplate->second;

	return nullptr;
}