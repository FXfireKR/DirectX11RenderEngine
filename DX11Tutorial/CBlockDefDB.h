#pragma once
#include "BlockDefTypes.h"

class CBlockDefDB
{
public:
	CBlockDefDB() = default;
	~CBlockDefDB() = default;

	void Initialize(const char* resourceRoot);
	bool Load();
	void Clear();

public:
	const BlockDef* GetBlockDef(BLOCK_ID blockID) const;
	const BlockDef* FindBlockDef(const char* blockName) const;
	const BlockDefRaw* FindNameDef(const char* defName) const;

	const BLOCK_ID FindBlockID(const char* blockName) const;
	const char* FindBlockName(BLOCK_ID blockID) const;

	const bool IsValidBlockID(BLOCK_ID blockID) const;
	const bool IsAir(BLOCK_ID blockID) const;
	const bool IsOpaque(BLOCK_ID blockID) const;
	const bool IsSolid(BLOCK_ID blockID) const;
	const bool IsFullCube(BLOCK_ID blockID) const;

	float GetHardness(BLOCK_ID blockID) const;
	const char* GetsoundProfile(BLOCK_ID blockID) const;

	const BLOCK_RENDER_LAYER GetRenderLayer(BLOCK_ID blockID) const;
	const BLOCK_COLLISION_TYPE GetCollisionType(BLOCK_ID blockID) const;

	const bool HasCollision(BLOCK_ID blockID) const;
	const bool UseAmbientOcclusion(BLOCK_ID blockID) const;
	const bool CanCullSameBlockFace(BLOCK_ID blockID) const;
	const uint8_t GetLightEmission(BLOCK_ID blockID) const;
	const bool IsFaceOccluder(BLOCK_ID blockID) const;

public:
	inline const vector<BlockDef>& GetBlockDefs() const { return m_vecBlockDefs; }

private:
	bool _LoadRegistry();
	bool _LoadTemplates();
	bool _LoadBlocks();
	bool _LoadOneDefFile(const filesystem::path& filePath, rapidjson::Document& doc);

	const bool _ReadJson(const filesystem::path& path, rapidjson::Document& outDoc) const;
	const bool _ParseBlockDefRaw(const rapidjson::Value& root, BlockDefRaw& outRaw) const;

	bool _BuildRuntimeDefs();
	const bool _ResolveOne(const string& blockName, BlockDef& outDef, unordered_set<string>& outVisiting) const;

	static void _ApplyRawToRuntime(const BlockDefRaw& raw, BlockDef& outDef);
	static void _ApplyDefaultFixup(BlockDef& outDef);

	static bool _ParseRenderLayer(const char* strLayer, BLOCK_RENDER_LAYER& outLayer);
	static bool _ParseCollisionType(const char* strType, BLOCK_COLLISION_TYPE& outType);

	const BlockDefRaw* _FindRaw(const string& name) const;

private:
	string m_strResourceRoot;

	// registry
	unordered_map<string, BLOCK_ID> m_mapNameToID;
	vector<string> m_vecIDToName;

	// all defs (template + concrete)
	unordered_map<string, BlockDefRaw> m_mapTemplates; // template only
	unordered_map<string, BlockDefRaw> m_mapNamedDefs; // actual blocks only

	// runtime concrete blocks only, index == BLOCK_ID
	vector<BlockDef> m_vecBlockDefs;
};