#pragma once
#include "singleton.h"

#include "BlockMiningTypes.h"

#include "CBlockDefDB.h"
#include "CBlockTagDB.h"
#include "CBlockStateDB.h"
#include "CModelDB.h"

class CBlockDB : public singleton<CBlockDB>
{
public:
	void Initialize(const char* resourceRoot);
	bool Load();
	void Clear();

	inline bool IsLoadedComplete() const { return m_bLoadComplete; }

public: // Block Def
	const BlockDef* GetBlockDef(BLOCK_ID blockID) const;
	const BlockDef* FindBlockDef(const char* blockName) const;

	BLOCK_ID FindBlockID(const char* blockName) const;
	const char* FindBlockName(BLOCK_ID blockID) const;


	const bool IsValidBlockID(BLOCK_ID blockID) const;
	const bool IsAir(BLOCK_ID blockID) const;
	const bool IsOpaque(BLOCK_ID blockID) const;
	const bool IsSolid(BLOCK_ID blockID) const;
	const bool IsFullCube(BLOCK_ID blockID) const;

	const float GetHardness(BLOCK_ID blockID) const;
	const char* GetSoundProfile(BLOCK_ID blockID) const;

	const BLOCK_RENDER_LAYER GetRenderLayer(BLOCK_ID blockID) const;
	const BLOCK_COLLISION_TYPE GetCollisionType(BLOCK_ID blockID) const;

	const bool HasCollision(BLOCK_ID blockID) const;
	const bool UseAmbientOcclusion(BLOCK_ID blockID) const;
	const bool CanCullSameBlockFace(BLOCK_ID blockID) const;
	const uint8_t GetLightEmission(BLOCK_ID blockID) const;
	const bool IsFaceOccluder(BLOCK_ID blockID) const;

public: // Block Tag
	const bool HasBlockTag(BLOCK_ID blockID, const char* tagName) const;
	const bool HasBlockTagHash(BLOCK_ID blockID, BLOCK_TAG_HASH tagHash) const;

	const BlockTagDef* FindBlockTagDef(const char* tagName) const;
	const BLOCK_TAG_HASH FindBlockTagHash(const char* tagName) const;

public: // Mining Rule
	const bool CanBreak(BLOCK_ID blockID) const;
	const bool IsPreferredTool(BLOCK_ID blockID, BLOCK_TOOL_KIND toolKind) const;
	const bool CanHarvestDrops(BLOCK_ID blockID, BLOCK_TOOL_KIND toolKind, BLOCK_TOOL_TIER toolTier) const;
	const float GetMiningSpeedMultiplier(BLOCK_ID blockID, BLOCK_TOOL_KIND toolKind) const;

public: // Block State
	bool EncodeStateIndex(BLOCK_ID blockID, const BlockPropHashMap& props, STATE_INDEX& outStateIndex) const;
	bool GetAppliedModels(BLOCK_ID blockID, STATE_INDEX stateIndex, vector<AppliedModel>& outModels) const;
	bool GetAppliedModels(BLOCK_ID blockID, STATE_INDEX stateIndex, const vector<AppliedModel>*& outModels) const;
	bool TryGetStateValueHash(BLOCK_ID blockID, STATE_INDEX stateIndex, PROP_HASH propHash, VALUE_HASH& outValueHash) const;

public: // Block Model
	const BakedModel* FindBakedModel(const char* modelKey) const;
	const BakedModel* FindBakedModel(uint64_t modelHash) const;
	const BakedModel* GetBakedModel(BLOCK_ID blockID, STATE_INDEX stateIndex) const;

	inline const unordered_set<string>& GetUsedTextureKeys() const { return m_modelDB.GetUsedTextureKeys(); }

private:
	const char* _GetMineableTagName(BLOCK_TOOL_KIND toolKind) const;
	const char* _GetIncorrectTierTagName(BLOCK_TOOL_TIER toolTier) const;

private:
	bool _ValidateLinks() const;

private:
	string m_strRoot;
	bool m_bInit = false;
	bool m_bLoadComplete = false;

	CBlockDefDB m_blockDefDB;
	CBlockTagDB m_blockTagDB;
	CBlockStateDB m_blockStateDB;
	CModelDB m_modelDB;
};