#include "pch.h"
#include "CChunkWorld.h"
#include "CScene.h"
#include "CPipeline.h"
#include "CMaterial.h"
#include "CWorld.h"
#include "CFlatChunkGenerator.h"
#include "CHeightmapChunkGenerator.h"

#include "ChunkMath.h"

using namespace ChunkMath;

static inline int ChunkDistChebyshev(int ax, int az, int bx, int bz)
{
	return std::max(std::abs(ax - bx), std::abs(az - bz));
}

void CChunkWorld::Initialize(CScene& scene, CPipeline* pOpaquePipeline, CMaterial* pOpaqueMaterial
	, CPipeline* pCutoutPipeline, CMaterial* pCutoutMaterial
	, CPipeline* pTranslucentPipeline, CMaterial* pTranslucentMaterial)
{
	m_pScene = &scene;

	m_pOpaquePipeline = pOpaquePipeline;
	m_pOpaqueMaterial = pOpaqueMaterial;

	m_pCutoutPipeline = pCutoutPipeline;
	m_pCutoutMaterial = pCutoutMaterial;

	m_pTranslucentPipeline = pTranslucentPipeline;
	m_pTranslucentMaterial = pTranslucentMaterial;

	// Chunk generator setting
	{
		m_genSettings.seed = 12345u;
		m_genSettings.baseHeight = 48;
		m_genSettings.heightAmplitude = 12;
		m_genSettings.frequency = 0.01f;
		m_genSettings.seaLevel = 48;
	}

	m_pGenerator = std::make_unique<CHeightmapChunkGenerator>();
	m_pGenerator->Initialize(m_genSettings);

	m_vecDirtyQueue.reserve(10000);
}

void CChunkWorld::UpdateStreaming(float fDelta, const XMFLOAT3& playerWorldPos)
{
	PROFILE_SCOPE();
	const int centerCx = FloorDiv16((int)std::floor(playerWorldPos.x));
	const int centerCz = FloorDiv16((int)std::floor(playerWorldPos.z));

	m_currentCenterChunk = { centerCx, centerCz };

	bool bStreamChanged = false;

#ifdef OPTIMIZATION_2
	const int hotRadius = m_iStreamRadius;
	const int warmRadius = m_iStreamRadius + m_iWarmRadiusOffset;
	const int coldRadius = m_iStreamRadius + m_iColdRadiusOffset;

	int preloadBudget = m_iPreloadBudgetPerFrame;
	int hotloadBudget = m_iHotloadBudgetPerFrame;
	int preUnloadBudget = m_iPreUnloadBudgetPerFrame;
	int coldUnloadBudget = m_iColdUnloadBudgetPerFrame;

	{
		PROFILE_SCOPE("BuildWantedSet");

		for (int dz = -warmRadius; dz <= warmRadius; ++dz)
		{
			for (int dx = -warmRadius; dx <= warmRadius; ++dx)
			{
				const int cx = centerCx + dx;
				const int cz = centerCz + dz;
				const int dist = std::max(std::abs(dx), std::abs(dz));

				CChunkColumn* pColumn = _FindColumn(cx, cz);

				// warm 범위 내에서는 preload
				if (dist <= warmRadius)
				{
					if ((!pColumn || !pColumn->IsGenerated()) && preloadBudget > 0)
					{
						_PreloadColumn(cx, cz);
						--preloadBudget;
						bStreamChanged = true;
						pColumn = _FindColumn(cx, cz);
					}
				}

				// hot 범위 내에서는 render object 활성
				if (dist <= hotRadius)
				{
					if (pColumn && !pColumn->IsActive() && hotloadBudget > 0)
					{
						_HotloadColumn(cx, cz);
						--hotloadBudget;
						bStreamChanged = true;
					}
				}
			}
		}
	}

	{
		PROFILE_SCOPE("UnloadFarColumns");

		for (auto& kv : m_columns)
		{
			CChunkColumn& column = kv.second;
			const ChunkCoord coord = column.GetCoord();
			const int dist = ChunkDistChebyshev(coord.x, coord.z, centerCx, centerCz);

			// hot 범위 밖 + active 라면 render만 내리기
			if (dist > warmRadius && column.IsActive())
			{
				if (preUnloadBudget > 0)
				{
					_PreUnloadColumn(coord.x, coord.z);
					--preUnloadBudget;
					bStreamChanged = true;
				}
				continue;
			}

			// 충분히 멀어졌다면 cold unload
			if (dist > coldRadius && column.IsResident())
			{
				if (coldUnloadBudget > 0)
				{
					_ColdUnloadColumn(coord.x, coord.z);
					--coldUnloadBudget;
					bStreamChanged = true;
				}
			}
		}
	}

	{
		PROFILE_SCOPE("RebuildAndUpdateDebug");
		if (bStreamChanged)
		{
			m_bPendingFullRelight = true;
		}

		if (m_bPendingFullRelight && !bStreamChanged)
		{
			_RebuildActiveBlockLightCache();
			m_bPendingFullRelight = false;
		}

		{
			m_fDebugStatsAccum += fDelta;
			if (m_fDebugStatsAccum > 0.5f) 
			{
				PROFILE_SCOPE("_UpdateDebugStats");
				_UpdateDebugStats();

				m_fDebugStatsAccum = 0.f;
			}
		}
	}

#else // OPTIMIZATION_2
	m_tmpWanted.clear();

	size_t newCap = static_cast<size_t>((m_iStreamRadius * 2 + 1) * (m_iStreamRadius * 2 + 1));

	{
		PROFILE_SCOPE("BuildWantedSet");
		for (int dz = -m_iStreamRadius; dz <= m_iStreamRadius; ++dz)
		{
			for (int dx = -m_iStreamRadius; dx <= m_iStreamRadius; ++dx)
			{
				const int cx = centerCx + dx;
				const int cz = centerCz + dz;
				const uint64_t key = MakeColumnKey(cx, cz);

				m_tmpWanted.emplace(key);

				CChunkColumn* column = _FindColumn(cx, cz);
				if (nullptr == column || !column->IsActive())
				{
					_LoadColumn(cx, cz);
					bStreamChanged = true;
				}
			}
		}
	}

	{
		PROFILE_SCOPE("UnloadFarColumns");
		for (auto& kv : m_columns)
		{
			CChunkColumn& column = kv.second;
			if (!column.IsActive())
				continue;

			const bool keep = m_tmpWanted.find(kv.first) != m_tmpWanted.end();
			if (false == keep)
			{
				const ChunkCoord coord = column.GetCoord();
				_UnloadColumn(coord.x, coord.z);
				bStreamChanged = true;
			}
		}
	}

	if (bStreamChanged)
	{
		_RebuildActiveBlockLightCache();
	}

	_UpdateDebugStats();
#endif // OPTIMIZATION_2
}

bool CChunkWorld::PopDirty(SectionCoord& outSectionCoord)
{
	if (m_vecDirtyQueue.empty())
		return false;

	int bestIndex = -1;
	int bestScore = INT_MAX;

	for (int i = 0; i < (int)m_vecDirtyQueue.size(); ++i)
	{
		const SectionCoord& sc = m_vecDirtyQueue[i];
		CChunkSection* pSection = FindSectionDataMutable(sc.x, sc.y, sc.z);

		if (nullptr == pSection || !pSection->IsDirty())
			continue;

		if (!pSection->IsBuildQueued())
			continue;

		const bool isMeshDirty = pSection->IsMeshDirty();
		const int dist = std::max(
			std::abs(sc.x - m_currentCenterChunk.x),
			std::abs(sc.z - m_currentCenterChunk.y));

		// mesh dirty를 light dirty보다 훨씬 우선
		// 같은 종류면 가까운 거리 우선
		const int score = dist * 10 + (isMeshDirty ? 0 : 1000);
		if (score < bestScore)
		{
			bestScore = score;
			bestIndex = i;
		}
	}

	if (bestIndex < 0)
	{
		m_vecDirtyQueue.clear();
		return false;
	}

	outSectionCoord = m_vecDirtyQueue[bestIndex];
	m_vecDirtyQueue.erase(m_vecDirtyQueue.begin() + bestIndex);
	return true;
}
bool CChunkWorld::CanRaycastHit(const BlockCell& cell) const
{
	if (cell.IsAir())
		return false;

	return BlockDB.GetRenderLayer(cell.blockID) != BLOCK_RENDER_LAYER::INVISIBLE_LAYER;
}

BlockCell CChunkWorld::GetBlock(int wx, int wy, int wz) const
{
	if (wy < 0 || wy >= CHUNK_SIZE_Y)
		return { 0, 0 };

	BlockCell modified{};
	if (_TryGetModifiedBlock(wx, wy, wz, modified))
		return modified;

	return _GetBaseBlock(wx, wy, wz);
}

bool CChunkWorld::IsSolid(const BlockCell& cell) const
{
	if (cell.IsAir())
		return false;

	return BlockDB.HasCollision(cell.blockID);
}

bool CChunkWorld::SetBlock(int wx, int wy, int wz, const BlockCell& newCell)
{
	if (wy < 0 || wy >= CHUNK_SIZE_Y) 
		return false;

	const BlockCell oldFinal = GetBlock(wx, wy, wz);
	if (oldFinal == newCell)
		return false;

	const BlockCell baseCell = _GetBaseBlock(wx, wy, wz);
	if (newCell == baseCell)
		_EraseModifiedBlock(wx, wy, wz);
	else
		_WriteModifiedBlock(wx, wy, wz, newCell);


	// resident cache
	int cx, sy, cz;
	int lx, ly, lz;

	bool bCurrentSectionRemoved = false;

	if (_WorldToSectionLocal(wx, wy, wz, cx, sy, cz, lx, ly, lz))
	{
		if (CChunkColumn* pColumn = _FindColumn(cx, cz))
		{
			if (pColumn->IsActive())
			{
				CChunkSection* pSection = pColumn->GetSection(sy);

				// Section이 존재하지 않으면 생성
				if (nullptr == pSection)
				{
					pSection = pColumn->EnsureSection(sy);
				}

				if (pSection)
				{
					pSection->SetBlock(lx, ly, lz, newCell);

					if (pSection->IsEmpty())
					{
						_DestoryRenderObjects(*pSection);
						pColumn->ResetSection(sy);
						bCurrentSectionRemoved = true;
					}
					else if (!pSection->HasAnyRenderObjectID() && !pSection->IsEmpty())
					{
						_EnsureRenderObjects(*pSection, cx, sy, cz);
					}
				}
			}
		}
	}

	if (!bCurrentSectionRemoved)
	{
		_MarkMeshDirty(cx, sy, cz);
		_MarkLightDirty(cx, sy, cz);
	}

	if (lx == 0)
	{
		_MarkMeshDirty(cx - 1, sy, cz);
		_MarkLightDirty(cx - 1, sy, cz);
	}
	else if (lx == CHUNK_SIZE_X - 1)
	{
		_MarkMeshDirty(cx + 1, sy, cz);
		_MarkLightDirty(cx + 1, sy, cz);
	}

	if (ly == 0 && sy > 0)
	{
		_MarkMeshDirty(cx, sy - 1, cz);
		_MarkLightDirty(cx, sy - 1, cz);
	}
	else if (ly == CHUNK_SECTION_SIZE - 1 && sy < CHUNK_SECTION_COUNT - 1)
	{
		_MarkMeshDirty(cx, sy + 1, cz);
		_MarkLightDirty(cx, sy + 1, cz);
	}

	if (lz == 0)
	{
		_MarkMeshDirty(cx, sy, cz - 1);
		_MarkLightDirty(cx, sy, cz - 1);
	}
	else if (lz == CHUNK_SIZE_Z - 1)
	{
		_MarkMeshDirty(cx, sy, cz + 1);
		_MarkLightDirty(cx, sy, cz + 1);
	}

	_UpdateBlockLightOnBlockChanged(wx, wy, wz, oldFinal, newCell);
	_ValidateAttachmentAround(wx, wy, wz);

	dbg.AddBlockEdit();
	return true;
}

uint8_t CChunkWorld::GetBlockLight(int wx, int wy, int wz) const
{
	if (wy < 0 || wy >= CHUNK_SIZE_Y)
		return 0;

	int cx, sy, cz;
	int lx, ly, lz;
	if (!_WorldToSectionLocal(wx, wy, wz, cx, sy, cz, lx, ly, lz))
		return 0;

	const CChunkLightSection* pLightSection = FindBlockLightSection(cx, sy, cz);
	if (nullptr == pLightSection)
		return 0;

	return pLightSection->GetBlockLight(lx, ly, lz);
}

void CChunkWorld::SetBlockLight(int wx, int wy, int wz, uint8_t level)
{
	LightDirtyTouchSet touched;
	_SetBlockLightRawNoDirty(wx, wy, wz, level);
	_TouchLightDirtyByWorld(touched, wx, wy, wz);
	_FlushTouchedLightDirty(touched);
}

CChunkLightSection* CChunkWorld::FindBlockLightSectionMutable(int cx, int sy, int cz)
{
	CChunkColumn* pColumn = _FindColumn(cx, cz);
	if (nullptr == pColumn)
		return nullptr;

	if (sy < 0 || sy >= CHUNK_SECTION_COUNT)
		return nullptr;

	return pColumn->GetBlockLightSection(sy);
}

const CChunkLightSection* CChunkWorld::FindBlockLightSection(int cx, int sy, int cz) const
{
	const CChunkColumn* pColumn = _FindColumn(cx, cz);
	if (nullptr == pColumn)
		return nullptr;

	if (sy < 0 || sy >= CHUNK_SECTION_COUNT)
		return nullptr;

	return pColumn->GetBlockLightSection(sy);
}

CChunkLightSection* CChunkWorld::EnsureBlockLightSection(int cx, int sy, int cz)
{
	if (sy < 0 || sy >= CHUNK_SECTION_COUNT)
		return nullptr;

	CChunkColumn& column = _GetOrCreateColumn(cx, cz);
	return column.EnsureBlockLightSection(sy);
}

CChunkSection* CChunkWorld::FindSectionDataMutable(int cx, int sy, int cz)
{
	auto* column = _FindColumn(cx, cz);
	if (nullptr == column)
		return nullptr;

	if (sy < 0 || sy >= CHUNK_SECTION_COUNT)
		return nullptr;

	return column->GetSection(sy);
}

const CChunkSection* CChunkWorld::FindSectionData(int cx, int sy, int cz) const
{
	auto* column = _FindColumn(cx, cz);
	if (nullptr == column)
		return nullptr;

	if (sy < 0 || sy >= CHUNK_SECTION_COUNT)
		return nullptr;

	return column->GetSection(sy);
}

CObject* CChunkWorld::FindRenderObject(int cx, int sy, int cz, EChunkSectionRenderSlot slot)
{
	auto* column = _FindColumn(cx, cz);
	if (nullptr == column)
		return nullptr;

	if (sy < 0 || sy >= CHUNK_SECTION_COUNT) 
		return nullptr;

	auto* section = column->GetSection(sy);
	if (nullptr == section)
		return nullptr;

	OBJECT_ID id = section->GetRenderObjectID(slot);
	if (!IsValidObjectID(id)) 
		return nullptr;

	return m_pScene->FindObject(id);
}

bool CChunkWorld::IsSpawnAreaReady(const XMFLOAT3& playerWorldPos) const
{
	const int centerCx = FloorDiv16((int)std::floor(playerWorldPos.x));
	const int centerCz = FloorDiv16((int)std::floor(playerWorldPos.z));

	// flat world 스폰 안정화용: 주변 3x3 컬럼의 section 0만 확인
	for (int dz = -1; dz <= 1; ++dz)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			const int cx = centerCx + dx;
			const int cz = centerCz + dz;

			const CChunkColumn* pColumn = _FindColumn(cx, cz);
			if (nullptr == pColumn || !pColumn->IsActive())
				return false;

			const CChunkSection* pSection = pColumn->GetSection(0);
			if (nullptr == pSection)
				return false;

			if (pSection->IsDirty())
				return false;

			if (pSection->IsBuildQueued())
				return false;

			if (!pSection->HasAnyRenderObjectID())
				return false;
		}
	}

	return true;
}

void CChunkWorld::DebugRequestReloadActiveColumns()
{
	if (m_debugReloadPhase != EDebugReloadPhase::NONE)
		return;

	m_debugReloadCoords.clear();
	m_debugReloadCoords.reserve(m_columns.size());

	for (const auto& kv : m_columns)
	{
		const CChunkColumn& column = kv.second;
		if (!column.IsActive())
			continue;

		m_debugReloadCoords.push_back(column.GetCoord());
	}

	if (m_debugReloadCoords.empty())
		return;

	for (const ChunkCoord& coord : m_debugReloadCoords)
	{
		_UnloadColumn(coord.x, coord.z);
	}

	m_debugReloadPhase = EDebugReloadPhase::WAIT_LOAD;
	_UpdateDebugStats();
}

void CChunkWorld::DebugProcessReloadRequest()
{
	if (m_debugReloadCoords.empty())
		return;

	for (const ChunkCoord& coord : m_debugReloadCoords)
	{
		_LoadColumn(coord.x, coord.z);
	}

	m_debugReloadCoords.clear();
	m_debugReloadPhase = EDebugReloadPhase::NONE;
	_UpdateDebugStats();
}

CChunkColumn* CChunkWorld::_FindColumn(int cx, int cz)
{
	auto it = m_columns.find(MakeColumnKey(cx, cz));
	if (it == m_columns.end())
		return nullptr;

	return &it->second;
}

const CChunkColumn* CChunkWorld::_FindColumn(int cx, int cz) const
{
	auto it = m_columns.find(MakeColumnKey(cx, cz));
	if (it == m_columns.end())
		return nullptr;

	return &it->second;
}

CChunkColumn& CChunkWorld::_GetOrCreateColumn(int cx, int cz)
{
	uint64_t key = MakeColumnKey(cx, cz);

	auto it = m_columns.find(key);
	if (it != m_columns.end())
		return it->second;

	auto [newIt, ok] = m_columns.try_emplace(key);
	newIt->second.Initialize(cx, cz);
	return newIt->second;
}

#ifdef OPTIMIZATION_2
void CChunkWorld::_LoadColumn(int cx, int cz)
{
	_PreloadColumn(cx, cz);
	_HotloadColumn(cx, cz);
}
void CChunkWorld::_UnloadColumn(int cx, int cz)
{
	_PreUnloadColumn(cx, cz);
	_ColdUnloadColumn(cx, cz);
}
void CChunkWorld::_PreloadColumn(int cx, int cz)
{
	CChunkColumn& column = _GetOrCreateColumn(cx, cz);

	if (!column.IsGenerated())
	{
		_GenerateBaseColumn(column);
		column.SetGenerated(true);
	}

	_ApplyModifiedOverlayToColumn(column);

	if (!column.IsActive())
		column.SetResidency(EChunkResidency::RESIDENT);
}

void CChunkWorld::_HotloadColumn(int cx, int cz)
{
	CChunkColumn* pColumn = _FindColumn(cx, cz);
	if (nullptr == pColumn)
		return;

	if (pColumn->IsActive())
		return;

	pColumn->SetResidency(EChunkResidency::ACTIVE);

	for (int sy = 0; sy < CHUNK_SECTION_COUNT; ++sy)
	{
		CChunkSection* pSection = pColumn->GetSection(sy);
		if (nullptr == pSection)
			continue;

		_EnsureRenderObjects(*pSection, cx, sy, cz);
		_MarkMeshDirty(cx, sy, cz);
	}

	dbg.AddChunkLoad();
}

void CChunkWorld::_PreUnloadColumn(int cx, int cz)
{
	CChunkColumn* pColumn = _FindColumn(cx, cz);
	if (nullptr == pColumn)
		return;

	if (!pColumn->IsActive())
		return;

	for (int sy = 0; sy < CHUNK_SECTION_COUNT; ++sy)
	{
		CChunkSection* pSection = pColumn->GetSection(sy);
		if (nullptr == pSection)
			continue;

		_DestoryRenderObjects(*pSection);   // 이제 즉시 destroy가 아니라 queue 처리
		//pSection->SetBuildQueued(false);
	}

	pColumn->SetResidency(EChunkResidency::RESIDENT);

	dbg.AddChunkUnload();
}

void CChunkWorld::_ColdUnloadColumn(int cx, int cz)
{
	CChunkColumn* pColumn = _FindColumn(cx, cz);
	if (nullptr == pColumn)
		return;

	// 아직 active면 먼저 preunload만 하고 다음 프레임 cold unload
	if (pColumn->IsActive())
		return;

	for (int sy = 0; sy < CHUNK_SECTION_COUNT; ++sy)
	{
		pColumn->ResetSection(sy);
		pColumn->ResetBlockLightSection(sy);
	}

	pColumn->SetGenerated(false);
	pColumn->SetResidency(EChunkResidency::RESIDENT);
}

#else // OPTIMIZATION_2
void CChunkWorld::_LoadColumn(int cx, int cz)
{
	CChunkColumn& column = _GetOrCreateColumn(cx, cz);

	if (!column.IsGenerated())
	{
		_GenerateBaseColumn(column);
		column.SetGenerated(true);
	}

	_ApplyModifiedOverlayToColumn(column);

	column.SetResidency(EChunkResidency::ACTIVE);

	for (int sy = 0; sy < CHUNK_SECTION_COUNT; ++sy)
	{
		CChunkSection* pSection = column.GetSection(sy);
		if (nullptr == pSection)
			continue;

		_EnsureRenderObjects(*pSection, cx, sy, cz);
		_MarkDirty(cx, sy, cz);
	}

	dbg.AddChunkLoad();
}

void CChunkWorld::_UnloadColumn(int cx, int cz)
{
	auto* column = _FindColumn(cx, cz);
	if (nullptr == column)
		return;

	if (!column->IsActive())
		return;

	for (int sy = 0; sy < CHUNK_SECTION_COUNT; ++sy)
	{
		CChunkSection* pSection = column->GetSection(sy);
		if (nullptr == pSection)
			continue;

		_DestoryRenderObjects(*pSection);

		pSection->SetBuildQueued(false);
	}

	for (int sy = 0; sy < CHUNK_SECTION_COUNT; ++sy)
	{
		column->ResetSection(sy);
		column->ResetBlockLightSection(sy);
	}

	column->SetResidency(EChunkResidency::RESIDENT);
	column->SetGenerated(false);

	dbg.AddChunkUnload();
}
#endif // OPTIMIZATION_2

void CChunkWorld::_EnsureRenderObjects(CChunkSection& section, int cx, int sy, int cz)
{
	_EnsureRenderObject(section, cx, sy, cz, EChunkSectionRenderSlot::OPAQUE_SLOT);
	_EnsureRenderObject(section, cx, sy, cz, EChunkSectionRenderSlot::CUTOUT_SLOT);
	_EnsureRenderObject(section, cx, sy, cz, EChunkSectionRenderSlot::TRANSLUCENT_SLOT);
}

void CChunkWorld::_EnsureRenderObject(CChunkSection& section, int cx, int sy, int cz, EChunkSectionRenderSlot slot)
{
	if (section.HasRenderObjectID(slot))
		return;

	string name = _MakeSectionName(cx, sy, cz, slot);
	CObject* obj = m_pScene->AddAndGetObject(name);

	auto* tr = obj->AddComponent<CTransform>();
	auto* mr = obj->AddComponent<CMeshRenderer>();

	tr->SetLocalTrans(XMFLOAT3(
		(float)(cx * CHUNK_SIZE_X), 
		(float)(sy * CHUNK_SECTION_SIZE), 
		(float)(cz * CHUNK_SIZE_Z))
	);

	switch (slot)
	{
		case EChunkSectionRenderSlot::OPAQUE_SLOT:
		{
			mr->SetPipeline(m_pOpaquePipeline);
			mr->SetMaterial(m_pOpaqueMaterial);
			mr->SetRenderPass(ERenderPass::OPAQUE_PASS);
		} break;

		case EChunkSectionRenderSlot::CUTOUT_SLOT:
		{
			mr->SetPipeline(m_pCutoutPipeline);
			mr->SetMaterial(m_pCutoutMaterial);
			mr->SetRenderPass(ERenderPass::CUTOUT_PASS);
		} break;

		case EChunkSectionRenderSlot::TRANSLUCENT_SLOT:
		{
			mr->SetPipeline(m_pTranslucentPipeline);
			mr->SetMaterial(m_pTranslucentMaterial);
			mr->SetRenderPass(ERenderPass::TRANSPARENT_PASS);
		} break;
	}

	constexpr float kHalfX = CHUNK_SIZE_X * 0.5f;
	constexpr float kHalfY = CHUNK_SECTION_SIZE * 0.5f;
	constexpr float kHalfZ = CHUNK_SIZE_Z * 0.5f;

	mr->SetFrustumCullEnabled(true);
	mr->SetLocalBounds(
		XMFLOAT3(kHalfX, kHalfY, kHalfZ)
		, XMFLOAT3(kHalfX, kHalfY, kHalfZ)
	);

	obj->SetEnable(false);
	section.SetRenderObjectID(slot, obj->GetID());
}

void CChunkWorld::_DestoryRenderObjects(CChunkSection& section)
{
	for (size_t i = 0; i < static_cast<size_t>(EChunkSectionRenderSlot::COUNT); ++i)
	{
		const EChunkSectionRenderSlot slot = static_cast<EChunkSectionRenderSlot>(i);

		if (!section.HasRenderObjectID(slot))
			continue;

		m_pScene->DestroyObject(section.GetRenderObjectID(slot));
		section.ClearRenderObjectID(slot);
	}
}

#ifdef OPTIMIZATION_2
void CChunkWorld::_MarkMeshDirty(int cx, int sy, int cz)
{
	CChunkSection* pSection = FindSectionDataMutable(cx, sy, cz);
	if (nullptr == pSection)
		return;

	pSection->MarkMeshDirty();

	if (pSection->IsBuildQueued())
		return;

	pSection->SetBuildQueued(true);
	m_vecDirtyQueue.push_back({ cx, sy, cz });
}

void CChunkWorld::_MarkLightDirty(int cx, int sy, int cz)
{
	CChunkSection* pSection = FindSectionDataMutable(cx, sy, cz);
	if (nullptr == pSection)
		return;

	pSection->MarkLightDirty();

	if (pSection->IsBuildQueued())
		return;

	pSection->SetBuildQueued(true);
	m_vecDirtyQueue.push_back({ cx, sy, cz });
}
#else // OPTIMIZATION_2
void CChunkWorld::_MarkDirty(int cx, int sy, int cz)
{
	CChunkSection* pSection = FindSectionDataMutable(cx, sy, cz);
	if (nullptr == pSection)
		return;

	pSection->MarkDirty();

	if (pSection->IsBuildQueued())
		return;

	pSection->SetBuildQueued(true);
	m_vecDirtyQueue.push_back({ cx, sy, cz });
}
#endif // OPTIMIZATION_2

bool CChunkWorld::_WorldToSectionLocal(int wx, int wy, int wz, int& outCx, int& outSy, int& outCz, int& outLx, int& outLy, int& outLz) const
{
	if (wy < 0 || wy >= CHUNK_SIZE_Y) 
		return false;

	outCx = FloorDiv16(wx);
	outSy = FloorDiv16(wy);
	outCz = FloorDiv16(wz);

	outLx = Mod16(wx);
	outLy = Mod16(wy);
	outLz = Mod16(wz);

	return true;
}

string CChunkWorld::_MakeSectionName(int cx, int sy, int cz, EChunkSectionRenderSlot slot)
{
	std::string suffix = "";
	switch (slot)
	{
		case EChunkSectionRenderSlot::OPAQUE_SLOT:
		{
			suffix = "opauqe";
		} break;
		case EChunkSectionRenderSlot::CUTOUT_SLOT:
		{
			suffix = "cutout";
		} break;
		case EChunkSectionRenderSlot::TRANSLUCENT_SLOT:
		{
			suffix = "translucent";
		} break;
	}

	char buf[96];
	sprintf(buf, "%d_%d_%d_%s", cx, sy, cz, suffix.c_str());
	return string(buf);
}

void CChunkWorld::_GenerateBaseColumn(CChunkColumn& column)
{
	if (!m_pGenerator)
		return;

	m_pGenerator->GenerateColumn(column);
}

BlockCell CChunkWorld::_GetBaseBlock(int wx, int wy, int wz) const
{
	if (!m_pGenerator)
		return {0,0};

	return m_pGenerator->SampleBlock(wx, wy, wz);
}

bool CChunkWorld::_TryGetModifiedBlock(int wx, int wy, int wz, BlockCell& outCell) const
{
	if (wy < 0 || wy >= CHUNK_SIZE_Y)
		return false;

	int cx, sy, cz;
	int lx, ly, lz;

	if (!_WorldToSectionLocal(wx, wy, wz, cx, sy, cz, lx, ly, lz))
		return false;

	const uint64_t columnKey = MakeColumnKey(cx, cz);
	auto itCol = m_modifiedColumns.find(columnKey);
	if (itCol == m_modifiedColumns.end())
		return false;

	const uint16_t localIndex = MakeColumnLocalIndex(lx, wy, lz);
	auto itCell = itCol->second.cells.find(localIndex);
	if (itCell == itCol->second.cells.end())
		return false;

	outCell = itCell->second;
	return true;
}

void CChunkWorld::_WriteModifiedBlock(int wx, int wy, int wz, const BlockCell& cell)
{
	int cx, sy, cz;
	int lx, ly, lz;

	if (!_WorldToSectionLocal(wx, wy, wz, cx, sy, cz, lx, ly, lz))
		return;

	const uint64_t columnKey = MakeColumnKey(cx, cz);
	const uint64_t localIndex = MakeColumnLocalIndex(lx, wy, lz);

	m_modifiedColumns[columnKey].cells[localIndex] = cell;
}

void CChunkWorld::_EraseModifiedBlock(int wx, int wy, int wz)
{
	int cx, sy, cz;
	int lx, ly, lz;

	if (!_WorldToSectionLocal(wx, wy, wz, cx, sy, cz, lx, ly, lz))
		return;

	const uint64_t columnKey = MakeColumnKey(cx, cz);
	auto itColumn = m_modifiedColumns.find(columnKey);
	if (itColumn == m_modifiedColumns.end())
		return;

	const uint16_t localIndex = MakeColumnLocalIndex(lx, wy, lz);
	itColumn->second.cells.erase(localIndex);

	if (itColumn->second.cells.empty())
		m_modifiedColumns.erase(itColumn);
}

void CChunkWorld::_ApplyModifiedOverlayToColumn(CChunkColumn& column)
{
	const ChunkCoord coord = column.GetCoord();
	const uint64_t columnKey = MakeColumnKey(coord.x, coord.z);

	auto itCol = m_modifiedColumns.find(columnKey);
	if (itCol == m_modifiedColumns.end())
		return;

	for (const auto& kv : itCol->second.cells)
	{
		const uint16_t idx = kv.first;
		const BlockCell& cell = kv.second;

		const int wy = idx / (CHUNK_SIZE_X * CHUNK_SIZE_Z);
		const int rem = idx % (CHUNK_SIZE_X * CHUNK_SIZE_Z);
		const int lz = rem / CHUNK_SIZE_X;
		const int lx = rem % CHUNK_SIZE_X;

		const int sy = wy / CHUNK_SECTION_SIZE;
		const int ly = wy % CHUNK_SECTION_SIZE;

		CChunkSection* pSection = column.EnsureSection(sy);
		if (nullptr == pSection)
			continue;

		pSection->SetBlock(lx, ly, lz, cell);
	}

	column.SetModified(true);
}

bool CChunkWorld::_CanBlockLightPassThrough(const BlockCell& cell) const
{
	if (cell.IsAir())
		return true;

	return !BlockDB.IsFaceOccluder(cell.blockID);
}

void CChunkWorld::_UpdateBlockLightOnBlockChanged(int wx, int wy, int wz,
	const BlockCell& oldCell, const BlockCell& newCell)
{
	LightDirtyTouchSet touched;

	const uint8_t oldEmission = BlockDB.GetLightEmission(oldCell.blockID);
	const uint8_t newEmission = BlockDB.GetLightEmission(newCell.blockID);

	const bool oldPass = _CanBlockLightPassThrough(oldCell);
	const bool newPass = _CanBlockLightPassThrough(newCell);

	if (oldEmission > 0)
	{
		_PropagateBlockLightRemove(wx, wy, wz, touched);
	}
	else if (oldPass && !newPass)
	{
		_PropagateBlockLightRemove(wx, wy, wz, touched);
	}

	if (!oldPass && newPass)
	{
		_RelightBlockLightAround(wx, wy, wz, touched);
	}

	if (newEmission > 0)
	{
		_PropagateBlockLightAdd(wx, wy, wz, newEmission, touched);
	}

	_FlushTouchedLightDirty(touched);
}

void CChunkWorld::_PropagateBlockLightAdd(int wx, int wy, int wz, uint8_t emission)
{
	if (emission == 0)
		return;

	if (wy < 0 || wy >= CHUNK_SIZE_Y)
		return;

	queue<BlockLightNode> q;

	if (GetBlockLight(wx, wy, wz) < emission)
		SetBlockLight(wx, wy, wz, emission);

	q.push({ {wx, wy, wz}, emission });

	while (!q.empty())
	{
		BlockLightNode cur = q.front();
		q.pop();

		if (cur.light <= 1)
			continue;

		const uint8_t nextLight = cur.light - 1;

		for (const XMINT3& dir : g_BlockLightDirs)
		{
			const int nx = cur.w.x + dir.x;
			const int ny = cur.w.y + dir.y;
			const int nz = cur.w.z + dir.z;

			if (ny < 0 || ny >= CHUNK_SIZE_Y)
				continue;

			const BlockCell neighborCell = GetBlock(nx, ny, nz);
			if (!_CanBlockLightPassThrough(neighborCell))
				continue;

			if (GetBlockLight(nx, ny, nz) >= nextLight)
				continue;

			SetBlockLight(nx, ny, nz, nextLight);
			q.push({ {nx, ny, nz} , nextLight });
		}
	}

}

void CChunkWorld::_PropagateBlockLightRemove(int wx, int wy, int wz)
{
	const uint8_t startLight = GetBlockLight(wx, wy, wz);
	if (startLight == 0)
		return;

	std::queue<BlockLightNode> removeQ;
	std::vector<BlockLightNode> relightSeeds;
	relightSeeds.reserve(64);

	SetBlockLight(wx, wy, wz, 0);
	removeQ.push({ {wx, wy, wz}, startLight });

	while (!removeQ.empty())
	{
		BlockLightNode cur = removeQ.front();
		removeQ.pop();

		for (const XMINT3& dir : g_BlockLightDirs)
		{
			const int nx = cur.w.x + dir.x;
			const int ny = cur.w.y + dir.y;
			const int nz = cur.w.z + dir.z;

			if (ny < 0 || ny >= CHUNK_SIZE_Y)
				continue;

			const uint8_t neighborLight = GetBlockLight(nx, ny, nz);
			if (neighborLight == 0)
				continue;

			const BlockCell neighborCell = GetBlock(nx, ny, nz);
			const uint8_t neighborEmission = BlockDB.GetLightEmission(neighborCell.blockID);

			// 다른 source 후보는 나중에 다시 퍼뜨리기
			if (neighborEmission > 0)
			{
				relightSeeds.push_back({ {nx, ny, nz}, neighborEmission });
			}

			if (neighborLight < cur.light)
			{
				SetBlockLight(nx, ny, nz, 0);
				removeQ.push({ {nx, ny, nz}, neighborLight });
			}
			else
			{
				relightSeeds.push_back({ {nx, ny, nz}, neighborLight });
			}
		}
	}

	for (const BlockLightNode& seed : relightSeeds)
	{
		if (seed.light == 0)
			continue;

		_PropagateBlockLightAdd(seed.w.x, seed.w.y, seed.w.z, seed.light);
	}
}

void CChunkWorld::_RelightBlockLightAround(int wx, int wy, int wz)
{
	for (const XMINT3& dir : g_BlockLightDirs)
	{
		const int nx = wx + dir.x;
		const int ny = wy + dir.y;
		const int nz = wz + dir.z;

		if (ny < 0 || ny >= CHUNK_SIZE_Y)
			continue;

		const BlockCell neighborCell = GetBlock(nx, ny, nz);

		const uint8_t emission = BlockDB.GetLightEmission(neighborCell.blockID);
		if (emission > 0)
		{
			_PropagateBlockLightAdd(nx, ny, nz, emission);
		}

		const uint8_t neighborLight = GetBlockLight(nx, ny, nz);
		if (neighborLight > 1)
		{
			_PropagateBlockLightAdd(nx, ny, nz, neighborLight);
		}
	}
}

void CChunkWorld::_RebuildActiveBlockLightCache()
{
	PROFILE_SCOPE();

	// 1) active 컬럼 light cache clear
	{
		PROFILE_SCOPE("LightCacheClear");

		for (auto& kv : m_columns)
		{
			CChunkColumn& col = kv.second;
			if (!col.IsActive())
				continue;

			const ChunkCoord coord = col.GetCoord();

			for (int sy = 0; sy < CHUNK_SECTION_COUNT; ++sy)
			{
				col.ResetBlockLightSection(sy);

				CChunkSection* sec = col.GetSection(sy);
				if (!sec)
					continue;

				_MarkLightDirty(coord.x, sy, coord.z);
			}
		}
	}
	

	// 2) emissive source scan + flood
	{
		PROFILE_SCOPE("EmissiveSourceScan");

		for (auto& kv : m_columns)
		{
			CChunkColumn& col = kv.second;
			if (!col.IsActive())
				continue;

			const ChunkCoord coord = col.GetCoord();

			for (int sy = 0; sy < CHUNK_SECTION_COUNT; ++sy)
			{
				CChunkSection* sec = col.GetSection(sy);
				if (!sec)
					continue;

				const int baseWx = coord.x * CHUNK_SIZE_X;
				const int baseWy = sy * CHUNK_SECTION_SIZE;
				const int baseWz = coord.z * CHUNK_SIZE_Z;

				for (int ly = 0; ly < CHUNK_SECTION_SIZE; ++ly)
				{
					for (int lz = 0; lz < CHUNK_SIZE_Z; ++lz)
					{
						for (int lx = 0; lx < CHUNK_SIZE_X; ++lx)
						{
							const BlockCell cell = sec->GetBlock(lx, ly, lz);
							if (cell.IsAir())
								continue;

							const uint8_t emission = BlockDB.GetLightEmission(cell.blockID);
							if (emission == 0)
								continue;

							const int wx = baseWx + lx;
							const int wy = baseWy + ly;
							const int wz = baseWz + lz;

							_PropagateBlockLightAdd(wx, wy, wz, emission);
						}
					}
				}
			}
		}
	}
}

void CChunkWorld::_ValidateAttachmentAround(int wx, int wy, int wz)
{
	static const XMINT3 dirs[6] =
	{
		{ 1, 0, 0 }, { -1, 0, 0 },
		{ 0, 1, 0 }, { 0,-1, 0 },
		{ 0, 0, 1 }, { 0, 0,-1 },
	};

	for (const XMINT3& d : dirs)
	{
		const int nx = wx + d.x;
		const int ny = wy + d.y;
		const int nz = wz + d.z;

		if (ny < 0 || ny >= CHUNK_SIZE_Y)
			continue;

		if (_IsUnsupportedAttachedBlock(nx, ny, nz))
		{
			SetBlock(nx, ny, nz, { 0, 0 });
		}
	}
}

bool CChunkWorld::_IsUnsupportedAttachedBlock(int wx, int wy, int wz) const
{
	const BlockCell cell = GetBlock(wx, wy, wz);
	if (cell.IsAir())
		return false;

	const BLOCK_ID torchID = BlockDB.FindBlockID("minecraft:torch");
	const BLOCK_ID wallTorchID = BlockDB.FindBlockID("minecraft:wall_torch");

	if (cell.blockID == torchID)
	{
		const BlockCell below = GetBlock(wx, wy - 1, wz);
		if (below.IsAir())
			return true;

		return !BlockDB.IsFaceOccluder(below.blockID);
	}

	if (cell.blockID == wallTorchID)
	{
		uint64_t facingHash = 0;
		if (!BlockDB.TryGetStateValueHash(cell.blockID, cell.stateIndex, fnv1a_64("facing"), facingHash))
			return true;

		XMINT3 supportDir{};

		if (!GetWallTorchSupportDirFromFacing(facingHash, supportDir))
			return true;

		const BlockCell support = GetBlock(wx + supportDir.x, wy + supportDir.y, wz + supportDir.z);
		if (support.IsAir())
			return true;

		return !BlockDB.IsFaceOccluder(support.blockID);
	}

	return false;
}

uint64_t CChunkWorld::_MakeLightDirtySectionKey(int cx, int sy, int cz)
{
	uint64_t x = static_cast<uint16_t>(cx & 0xFFFF);
	uint64_t y = static_cast<uint16_t>(sy & 0xFFFF);
	uint64_t z = static_cast<uint16_t>(cz & 0xFFFF);
	return x | (y << 16) | (z << 32);
}

void CChunkWorld::_DecodeLightDirtySectionKey(uint64_t key, int& cx, int& sy, int& cz)
{
	cx = static_cast<int16_t>(key & 0xFFFF);
	sy = static_cast<int16_t>((key >> 16) & 0xFFFF);
	cz = static_cast<int16_t>((key >> 32) & 0xFFFF);
}

void CChunkWorld::_TouchLightDirtyByWorld(LightDirtyTouchSet& touched, int wx, int wy, int wz)
{
	int cx, sy, cz;
	int lx, ly, lz;

	if (!_WorldToSectionLocal(wx, wy, wz, cx, sy, cz, lx, ly, lz))
		return;

	touched.insert(_MakeLightDirtySectionKey(cx, sy, cz));

	if (lx == 0)
		touched.insert(_MakeLightDirtySectionKey(cx - 1, sy, cz));
	else if (lx == CHUNK_SIZE_X - 1)
		touched.insert(_MakeLightDirtySectionKey(cx + 1, sy, cz));

	if (ly == 0 && sy > 0)
		touched.insert(_MakeLightDirtySectionKey(cx, sy - 1, cz));
	else if (ly == CHUNK_SECTION_SIZE - 1 && sy < CHUNK_SECTION_COUNT - 1)
		touched.insert(_MakeLightDirtySectionKey(cx, sy + 1, cz));

	if (lz == 0)
		touched.insert(_MakeLightDirtySectionKey(cx, sy, cz - 1));
	else if (lz == CHUNK_SIZE_Z - 1)
		touched.insert(_MakeLightDirtySectionKey(cx, sy, cz + 1));
}

void CChunkWorld::_FlushTouchedLightDirty(const LightDirtyTouchSet& touched)
{
	for (uint64_t key : touched)
	{
		int cx, sy, cz;
		_DecodeLightDirtySectionKey(key, cx, sy, cz);
		_MarkLightDirty(cx, sy, cz);
	}
}

void CChunkWorld::_SetBlockLightRawNoDirty(int wx, int wy, int wz, uint8_t level)
{
	if (wy < 0 || wy >= CHUNK_SIZE_Y)
		return;

	if (level > 15)
		level = 15;

	int cx, sy, cz;
	int lx, ly, lz;
	if (!_WorldToSectionLocal(wx, wy, wz, cx, sy, cz, lx, ly, lz))
		return;

	if (level == 0)
	{
		CChunkLightSection* pLightSection = FindBlockLightSectionMutable(cx, sy, cz);
		if (nullptr == pLightSection)
			return;

		pLightSection->SetBlockLight(lx, ly, lz, 0);

		if (pLightSection->IsAllZero())
		{
			if (CChunkColumn* pColumn = _FindColumn(cx, cz))
				pColumn->ResetBlockLightSection(sy);
		}
	}
	else
	{
		CChunkLightSection* pLightSection = EnsureBlockLightSection(cx, sy, cz);
		if (nullptr == pLightSection)
			return;

		pLightSection->SetBlockLight(lx, ly, lz, level);
	}
}

void CChunkWorld::_PropagateBlockLightAdd(int wx, int wy, int wz, uint8_t emission, LightDirtyTouchSet& touched)
{
	if (emission == 0)
		return;

	if (wy < 0 || wy >= CHUNK_SIZE_Y)
		return;

	queue<BlockLightNode> q;

	if (GetBlockLight(wx, wy, wz) < emission)
	{
		_SetBlockLightRawNoDirty(wx, wy, wz, emission);
		_TouchLightDirtyByWorld(touched, wx, wy, wz);
	}

	q.push({ {wx, wy, wz}, emission });

	while (!q.empty())
	{
		BlockLightNode cur = q.front();
		q.pop();

		if (cur.light <= 1)
			continue;

		const uint8_t nextLight = cur.light - 1;

		for (const XMINT3& dir : g_BlockLightDirs)
		{
			const int nx = cur.w.x + dir.x;
			const int ny = cur.w.y + dir.y;
			const int nz = cur.w.z + dir.z;

			if (ny < 0 || ny >= CHUNK_SIZE_Y)
				continue;

			const BlockCell neighborCell = GetBlock(nx, ny, nz);
			if (!_CanBlockLightPassThrough(neighborCell))
				continue;

			if (GetBlockLight(nx, ny, nz) >= nextLight)
				continue;

			_SetBlockLightRawNoDirty(nx, ny, nz, nextLight);
			_TouchLightDirtyByWorld(touched, nx, ny, nz);
			q.push({ {nx, ny, nz}, nextLight });
		}
	}
}

void CChunkWorld::_PropagateBlockLightRemove(int wx, int wy, int wz, LightDirtyTouchSet& touched)
{
	const uint8_t startLight = GetBlockLight(wx, wy, wz);
	if (startLight == 0)
		return;

	std::queue<BlockLightNode> removeQ;
	std::vector<BlockLightNode> relightSeeds;
	relightSeeds.reserve(64);

	_SetBlockLightRawNoDirty(wx, wy, wz, 0);
	_TouchLightDirtyByWorld(touched, wx, wy, wz);
	removeQ.push({ {wx, wy, wz}, startLight });

	while (!removeQ.empty())
	{
		BlockLightNode cur = removeQ.front();
		removeQ.pop();

		for (const XMINT3& dir : g_BlockLightDirs)
		{
			const int nx = cur.w.x + dir.x;
			const int ny = cur.w.y + dir.y;
			const int nz = cur.w.z + dir.z;

			if (ny < 0 || ny >= CHUNK_SIZE_Y)
				continue;

			const uint8_t neighborLight = GetBlockLight(nx, ny, nz);
			if (neighborLight == 0)
				continue;

			const BlockCell neighborCell = GetBlock(nx, ny, nz);
			const uint8_t neighborEmission = BlockDB.GetLightEmission(neighborCell.blockID);

			if (neighborEmission > 0)
				relightSeeds.push_back({ {nx, ny, nz}, neighborEmission });

			if (neighborLight < cur.light)
			{
				_SetBlockLightRawNoDirty(nx, ny, nz, 0);
				_TouchLightDirtyByWorld(touched, nx, ny, nz);
				removeQ.push({ {nx, ny, nz}, neighborLight });
			}
			else
			{
				relightSeeds.push_back({ {nx, ny, nz}, neighborLight });
			}
		}
	}

	for (const BlockLightNode& seed : relightSeeds)
	{
		if (seed.light == 0)
			continue;

		_PropagateBlockLightAdd(seed.w.x, seed.w.y, seed.w.z, seed.light, touched);
	}
}

void CChunkWorld::_RelightBlockLightAround(int wx, int wy, int wz, LightDirtyTouchSet& touched)
{
	for (const XMINT3& dir : g_BlockLightDirs)
	{
		const int nx = wx + dir.x;
		const int ny = wy + dir.y;
		const int nz = wz + dir.z;

		if (ny < 0 || ny >= CHUNK_SIZE_Y)
			continue;

		const BlockCell neighborCell = GetBlock(nx, ny, nz);

		const uint8_t emission = BlockDB.GetLightEmission(neighborCell.blockID);
		if (emission > 0)
		{
			_PropagateBlockLightAdd(nx, ny, nz, emission, touched);
		}

		const uint8_t neighborLight = GetBlockLight(nx, ny, nz);
		if (neighborLight > 1)
		{
			_PropagateBlockLightAdd(nx, ny, nz, neighborLight, touched);
		}
	}
}

void CChunkWorld::_UpdateDebugStats()
{
	int loadedColumns = 0;
	int loadedSections = 0;
	int dirtySections = 0;

	for (auto& kv : m_columns)
	{
		CChunkColumn& col = kv.second;
		if (!col.IsActive())
			continue;

		++loadedColumns;

		for (int sy = 0; sy < CHUNK_SECTION_COUNT; ++sy)
		{
			CChunkSection* sec = col.GetSection(sy);
			if (!sec) 
				continue;

			++loadedSections;
			if (sec->IsDirty())
				++dirtySections;
		}
	}

	int modifiedColumns = static_cast<int>(m_modifiedColumns.size());
	int modifiedCells = 0;

	for (const auto& kv : m_modifiedColumns)
	{
		modifiedCells += static_cast<int>(kv.second.cells.size());
	}

	dbg.SetLoadedColumnCount(loadedColumns);
	dbg.SetLoadedSectionCount(loadedSections);
	dbg.SetDirtySectionCount(dirtySections);
	dbg.SetRebuildQueuedCount((int)m_vecDirtyQueue.size());

	dbg.SetModifiedColumnCount(modifiedColumns);
	dbg.SetModifiedCellCount(modifiedCells);
}