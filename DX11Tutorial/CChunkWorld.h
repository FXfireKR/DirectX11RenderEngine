#pragma once
#include "CChunkColumn.h"
#include "IBlockAccessor.hpp"
#include "IChunkGenerator.h"

class CScene;
class CObject;
class CPipeline;
class CMaterial;

/*

CWorld
 └─ CChunkWorld : public IBlockAccessor
	 └─ unordered_map<ColumnKey, CChunkColumn>
		 └─ array<unique_ptr<CChunkSection>, CHUNK_SECTION_COUNT>

CChunkSection
 ├─ array<BlockCell, CHUNK_SECTION_VOLUME> m_cells
 ├─ bool m_bDirty
 ├─ bool m_bBuildQueued
 └─ OBJECT_ID m_renderObjectID

*/

enum class EDebugReloadPhase : uint8_t
{
	NONE = 0,
	WAIT_LOAD,
};

struct ModifiedColumnOverlay
{
	unordered_map<uint16_t, BlockCell> cells;
};

struct BlockLightNode
{
	XMINT3 w { 0, 0, 0 };
	uint8_t light = 0;
	uint8_t inDir = 0xFF;
};

const XMINT3 g_BlockLightDirs[6] =
{
	{  1,  0,  0 },
	{ -1,  0,  0 },
	{  0,  1,  0 },
	{  0, -1,  0 },
	{  0,  0,  1 },
	{  0,  0, -1 },
};

class CChunkWorld : public IBlockAccessor
{
public:
	CChunkWorld() = default;
	~CChunkWorld() = default;

	void Initialize(CScene& scene, CPipeline* pOpaquePipeline, CMaterial* pOpaqueMaterial
		, CPipeline* pCutoutPipeline, CMaterial* pCutoutMaterial
		, CPipeline* pTranslucentPipeline, CMaterial* pTranslucentMaterial);
	void UpdateStreaming(float fDelta, const XMFLOAT3& playerWorldPos);
	bool PopDirty(SectionCoord& outSectionCoord);

	// IBlockAccessor
	bool CanRaycastHit(const BlockCell& cell) const override;
	BlockCell GetBlock(int wx, int wy, int wz) const override;
	bool IsSolid(const BlockCell& cell) const override;

	bool SetBlock(int wx, int wy, int wz, const BlockCell& newCell);

	uint8_t GetBlockLight(int wx, int wy, int wz) const;
	void SetBlockLight(int wx, int wy, int wz, uint8_t level);

	CChunkLightSection* FindBlockLightSectionMutable(int cx, int sy, int cz);
	const CChunkLightSection* FindBlockLightSection(int cx, int sy, int cz) const;
	CChunkLightSection* EnsureBlockLightSection(int cx, int sy, int cz);

	CChunkSection* FindSectionDataMutable(int cx, int sy, int cz);
	const CChunkSection* FindSectionData(int cx, int sy, int cz) const;
	CObject* FindRenderObject(int cx, int sy, int cz, EChunkSectionRenderSlot slot);

	bool IsSpawnAreaReady(const XMFLOAT3& playerWorldPos) const;

	void DebugRequestReloadActiveColumns();
	void DebugProcessReloadRequest();

	// range-based accessor
	template<typename Fn>
	void ForEachResidentColumn(Fn&& fn) const {
		for (const auto& kv : m_columns)
			fn(kv.second);
	}

	template<typename Fn>
	void ForEachActiveColumn(Fn&& fn) const {
		for (const auto& kv : m_columns)
			if (kv.second.IsActive())
				fn(kv.second);
	}

private:
	CChunkColumn* _FindColumn(int cx, int cz);
	const CChunkColumn* _FindColumn(int cx, int cz) const;
	CChunkColumn& _GetOrCreateColumn(int cx, int cz);

	void _LoadColumn(int cx, int cz);
	void _UnloadColumn(int cx, int cz);

#ifdef OPTIMIZATION_2
	void _PreloadColumn(int cx, int cz);
	void _HotloadColumn(int cx, int cz);
	void _PreUnloadColumn(int cx, int cz);
	void _ColdUnloadColumn(int cx, int cz);
#endif // OPTIMIZATION_2

	void _EnsureRenderObjects(CChunkSection& section, int cx, int sy, int cz);
	void _EnsureRenderObject(CChunkSection& section, int cx, int sy, int cz, EChunkSectionRenderSlot slot);
	void _DestoryRenderObjects(CChunkSection& section);
	
#ifdef OPTIMIZATION_2
	void _MarkMeshDirty(int cx, int sy, int cz);
	void _MarkLightDirty(int cx, int sy, int cz);
#else // OPTIMIZATION_2
	void _MarkDirty(int cx, int sy, int cz);
#endif // OPTIMIZATION_2

	bool _WorldToSectionLocal(int wx, int wy, int wz,
		int& outCx, int& outSy, int& outCz,
		int& outLx, int& outLy, int& outLz) const;

	string _MakeSectionName(int cx, int sy, int cz, EChunkSectionRenderSlot slot);

	void _GenerateBaseColumn(CChunkColumn& column);
	BlockCell _GetBaseBlock(int wx, int wy, int wz) const;
	bool _TryGetModifiedBlock(int wx, int wy, int wz, BlockCell& outCell) const;

	void _WriteModifiedBlock(int wx, int wy, int wz, const BlockCell& cell);
	void _EraseModifiedBlock(int wx, int wy, int wz);

	void _ApplyModifiedOverlayToColumn(CChunkColumn& column);
	
	bool _CanBlockLightPassThrough(const BlockCell& cell) const;
	void _UpdateBlockLightOnBlockChanged(int wx, int wy, int wz, const BlockCell& oldCell, const BlockCell& newCell);
	void _PropagateBlockLightAdd(int wx, int wy, int wz, uint8_t emission);
	void _PropagateBlockLightRemove(int wx, int wy, int wz);
	void _RelightBlockLightAround(int wx, int wy, int wz);
	void _RebuildActiveBlockLightCache();
	void _ValidateAttachmentAround(int wx, int wy, int wz);
	bool _IsUnsupportedAttachedBlock(int wx, int wy, int wz) const;

	using LightDirtyTouchSet = std::unordered_set<uint64_t>;

	static uint64_t _MakeLightDirtySectionKey(int cx, int sy, int cz);
	static void _DecodeLightDirtySectionKey(uint64_t key, int& cx, int& sy, int& cz);

	void _SetBlockLightRawNoDirty(int wx, int wy, int wz, uint8_t level);
	void _TouchLightDirtyByWorld(LightDirtyTouchSet& touched, int wx, int wy, int wz);
	void _FlushTouchedLightDirty(const LightDirtyTouchSet& touched);

	void _PropagateBlockLightAdd(int wx, int wy, int wz, uint8_t emission, LightDirtyTouchSet& touched);
	void _PropagateBlockLightRemove(int wx, int wy, int wz, LightDirtyTouchSet& touched);
	void _RelightBlockLightAround(int wx, int wy, int wz, LightDirtyTouchSet& touched);

	void _UpdateDebugStats();

public:
	inline const int GetStreamRadius() const { return m_iStreamRadius; }
	inline void SetStreamRadius(int iRadius) { m_iStreamRadius = iRadius > 0 ? iRadius : m_iStreamRadius; }
	inline size_t GetDirtyQueueSize() const { return m_vecDirtyQueue.size(); }

private:
	CScene* m_pScene = nullptr;

	// Opaque
	CPipeline* m_pOpaquePipeline = nullptr;
	CMaterial* m_pOpaqueMaterial = nullptr;

	// Cutout
	CPipeline* m_pCutoutPipeline = nullptr;
	CMaterial* m_pCutoutMaterial = nullptr;

	// Translucent
	CPipeline* m_pTranslucentPipeline = nullptr;
	CMaterial* m_pTranslucentMaterial = nullptr;

	unordered_map<uint64_t, CChunkColumn> m_columns;
	unordered_map<uint64_t, ModifiedColumnOverlay> m_modifiedColumns;

	unique_ptr<IChunkGenerator> m_pGenerator;
	WorldGenerateSettings m_genSettings{};

	EDebugReloadPhase m_debugReloadPhase = EDebugReloadPhase::NONE;
	vector<ChunkCoord> m_debugReloadCoords;

	vector<SectionCoord> m_vecDirtyQueue;
	XMINT2 m_currentCenterChunk{ 0, 0 };

	int m_iStreamRadius = 14; // 6 ~ 8

#ifdef OPTIMIZATION_2
	int m_iWarmRadiusOffset = 3;
	int m_iColdRadiusOffset = 5;

	int m_iPreloadBudgetPerFrame = 2;
	int m_iHotloadBudgetPerFrame = 2;
	int m_iPreUnloadBudgetPerFrame = 2;
	int m_iColdUnloadBudgetPerFrame = 1;

	bool m_bPendingFullRelight = true;
#else // OPTIMIZATION_2
	unordered_set<uint64_t> m_tmpWanted;
#endif // OPTIMIZATION_2

	float m_fDebugStatsAccum = 0.f;
};