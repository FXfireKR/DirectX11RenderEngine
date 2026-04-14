#include "pch.h"
#include "CBlockDB.h"

void CBlockDB::Initialize(const char* resourceRoot)
{
	m_strRoot = resourceRoot;
	m_bInit = true;
	m_bLoadComplete = false;

	m_blockDefDB.Initialize(resourceRoot);
	m_blockTagDB.Initialize(resourceRoot, &m_blockDefDB);
	m_blockStateDB.Initialize(resourceRoot, &m_blockDefDB, &m_modelDB);
	m_modelDB.Initialize(resourceRoot);

	if (!m_blockDefDB.Load())
		return;
		//return false;

	if (!m_blockTagDB.Load()) 
		return;
		//return false;
}

bool CBlockDB::Load()
{
	m_blockStateDB.Load();

	if (m_blockStateDB.IsLoadComplete())
	{
		m_bLoadComplete = true;
		m_modelDB.Load();
	}

	return true;
}

void CBlockDB::Clear()
{

}

const BlockDef* CBlockDB::GetBlockDef(BLOCK_ID blockID) const
{
	return m_blockDefDB.GetBlockDef(blockID);
}

const BlockDef* CBlockDB::FindBlockDef(const char* blockName) const
{
	return m_blockDefDB.FindBlockDef(blockName);
}

BLOCK_ID CBlockDB::FindBlockID(const char* blockName) const
{
	return m_blockDefDB.FindBlockID(blockName);
}

const char* CBlockDB::FindBlockName(BLOCK_ID blockID) const
{
	return m_blockDefDB.FindBlockName(blockID);
}

const bool CBlockDB::IsValidBlockID(BLOCK_ID blockID) const
{
	return m_blockDefDB.IsValidBlockID(blockID);
}

const bool CBlockDB::IsAir(BLOCK_ID blockID) const
{
	return m_blockDefDB.IsAir(blockID);
}

const bool CBlockDB::IsOpaque(BLOCK_ID blockID) const
{
	return m_blockDefDB.IsOpaque(blockID);
}

const bool CBlockDB::IsSolid(BLOCK_ID blockID) const
{
	return m_blockDefDB.IsSolid(blockID);
}

const bool CBlockDB::IsFullCube(BLOCK_ID blockID) const
{
	return m_blockDefDB.IsFullCube(blockID);
}

const float CBlockDB::GetHardness(BLOCK_ID blockID) const
{
	return m_blockDefDB.GetHardness(blockID);
}

const char* CBlockDB::GetSoundProfile(BLOCK_ID blockID) const
{
	return m_blockDefDB.GetsoundProfile(blockID);
}

const BLOCK_RENDER_LAYER CBlockDB::GetRenderLayer(BLOCK_ID blockID) const
{
	return m_blockDefDB.GetRenderLayer(blockID);
}

const BLOCK_COLLISION_TYPE CBlockDB::GetCollisionType(BLOCK_ID blockID) const
{
	return m_blockDefDB.GetCollisionType(blockID);
}

const bool CBlockDB::HasCollision(BLOCK_ID blockID) const
{
	return m_blockDefDB.HasCollision(blockID);
}

const bool CBlockDB::UseAmbientOcclusion(BLOCK_ID blockID) const
{
	return m_blockDefDB.UseAmbientOcclusion(blockID);
}

const bool CBlockDB::CanCullSameBlockFace(BLOCK_ID blockID) const
{
	return m_blockDefDB.CanCullSameBlockFace(blockID);
}

const uint8_t CBlockDB::GetLightEmission(BLOCK_ID blockID) const
{
	return m_blockDefDB.GetLightEmission(blockID);
}

const bool CBlockDB::IsFaceOccluder(BLOCK_ID blockID) const
{
	return m_blockDefDB.IsFaceOccluder(blockID);
}

const bool CBlockDB::HasBlockTag(BLOCK_ID blockID, const char* tagName) const
{
	return m_blockTagDB.HasTag(blockID, tagName);
}

const bool CBlockDB::HasBlockTagHash(BLOCK_ID blockID, BLOCK_TAG_HASH tagHash) const
{
	return m_blockTagDB.HasTagHash(blockID, tagHash);
}

const BlockTagDef* CBlockDB::FindBlockTagDef(const char* tagName) const
{
	return m_blockTagDB.FindTagDef(tagName);
}

const BLOCK_TAG_HASH CBlockDB::FindBlockTagHash(const char* tagName) const
{
	return m_blockTagDB.FindTagHash(tagName);
}

const bool CBlockDB::CanBreak(BLOCK_ID blockID) const
{
	if (!IsValidBlockID(blockID))
		return false;

	if (IsAir(blockID))
		return false;

	return true;
}

const bool CBlockDB::IsPreferredTool(BLOCK_ID blockID, BLOCK_TOOL_KIND toolKind) const
{


	return false;
}

const bool CBlockDB::CanHarvestDrops(BLOCK_ID blockID, BLOCK_TOOL_KIND toolKind, BLOCK_TOOL_TIER toolTier) const
{
	if (!CanBreak(blockID))
		return false;

	const char* mineableTag = _GetMineableTagName(toolKind);
	if (!mineableTag)
		return false;

	if (!HasBlockTag(blockID, mineableTag))
		return false;

	const char* incorrectTag = _GetIncorrectTierTagName(toolTier);
	if (incorrectTag && HasBlockTag(blockID, incorrectTag))
		return false;

	return true;
}

const float CBlockDB::GetMiningSpeedMultiplier(BLOCK_ID blockID, BLOCK_TOOL_KIND toolKind) const
{
	return 0.0f;
}

bool CBlockDB::EncodeStateIndex(BLOCK_ID blockID, const BlockPropHashMap& props, STATE_INDEX& outStateIndex) const
{
	return m_blockStateDB.EncodeStateIndex(blockID, props, outStateIndex);
}

bool CBlockDB::GetAppliedModels(BLOCK_ID blockID, STATE_INDEX stateIndex, vector<AppliedModel>& outModels) const
{
	return m_blockStateDB.GetAppliedModels(blockID, stateIndex, outModels);
}

bool CBlockDB::GetAppliedModels(BLOCK_ID blockID, STATE_INDEX stateIndex, const vector<AppliedModel>*& outModels) const
{
	return m_blockStateDB.GetAppliedModels(blockID, stateIndex, outModels);
}

bool CBlockDB::TryGetStateValueHash(BLOCK_ID blockID, STATE_INDEX stateIndex, PROP_HASH propHash, VALUE_HASH& outValueHash) const
{
	return m_blockStateDB.TryGetStateValueHash(blockID, stateIndex, propHash, outValueHash);
}

const BakedModel* CBlockDB::FindBakedModel(const char* modelKey) const
{
	return m_modelDB.FindBakedModel(modelKey);
}

const BakedModel* CBlockDB::FindBakedModel(uint64_t modelHash) const
{
	return m_modelDB.FindBakedModel(modelHash);
}

const BakedModel* CBlockDB::GetBakedModel(BLOCK_ID blockID, STATE_INDEX stateIndex) const
{
	vector<AppliedModel> vecModels;
	if (!m_blockStateDB.GetAppliedModels(blockID, stateIndex, vecModels))
		return nullptr;

	if (vecModels.empty())
		return nullptr;

	return m_modelDB.FindBakedModel(vecModels[0].modelKey.c_str());
}

const char* CBlockDB::_GetMineableTagName(BLOCK_TOOL_KIND toolKind) const
{
	switch (toolKind)
	{
		case BLOCK_TOOL_KIND::PICKAXE: return "minecraft:mineable/pickaxe";
		case BLOCK_TOOL_KIND::SHOVEL: return "minecraft:mineable/shovel";
		case BLOCK_TOOL_KIND::AXE: return "minecraft:mineable/axe";
		case BLOCK_TOOL_KIND::HOE: return "minecraft:mineable/hoe";
		default: break;
	}
	return nullptr;
}

const char* CBlockDB::_GetIncorrectTierTagName(BLOCK_TOOL_TIER toolTier) const
{
	switch (toolTier)
	{
		case BLOCK_TOOL_TIER::WOOD: return "minecraft:incorrect_for_wooden_tool";
		case BLOCK_TOOL_TIER::STONE: return "minecraft:incorrect_for_stone_tool";
		case BLOCK_TOOL_TIER::IRON: return "minecraft:incorrect_for_iron_tool";
		case BLOCK_TOOL_TIER::DIAMOND: return "minecraft:incorrect_for_diamond_tool";
		case BLOCK_TOOL_TIER::NETHERITE: return nullptr;
		default: break;
	}
	return nullptr;
}

bool CBlockDB::_ValidateLinks() const
{
	BLOCK_ID airID = m_blockDefDB.FindBlockID("minecraft:air");
	if (airID != 0)
	{
		assert(false && "minecraft:air must be block id 0");
		return false;
	}

	const auto& vecDefs = m_blockDefDB.GetBlockDefs();
	for (const BlockDef& def : vecDefs)
	{
		//if (!def.bLoaded) continue;
		if (def.properties.bAir) continue;
		if (def.stateSource.empty())
		{
#ifdef DEBUG_LOG
			cout << "[BlockDB] stateSource empty : " << def.name << "\n";
#endif // DEBUG_LOG
		}
	}

	return true;
}
