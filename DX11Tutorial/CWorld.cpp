#include "pch.h"
#include "CWorld.h"
#include "CChunkMesherSystem.h"
#include "ChunkMath.h"
#include "BlockRaycastUtil.h"

using namespace ChunkMath;

CWorld::CWorld()
    : m_pChunkWorld(make_unique<CChunkWorld>())
{
}

void CWorld::Initialize(CScene& scene, CPipeline* pOpaquePipeline, CMaterial* pOpaqueMaterial
	, CPipeline* pCutoutPipeline, CMaterial* pCutoutMaterial
	, CPipeline* pTranslucentPipeline, CMaterial* pTranslucentMaterial)
{
	m_pChunkWorld->Initialize(scene, pOpaquePipeline, pOpaqueMaterial
		, pCutoutPipeline, pCutoutMaterial
		, pTranslucentPipeline, pTranslucentMaterial
	);

	m_worldTime.Initialize(0.25f);
}

void CWorld::Update(float fDelta, XMFLOAT3 pos)
{
	m_worldTime.Update(fDelta);

	m_pChunkWorld->DebugProcessReloadRequest();
	m_pChunkWorld->UpdateStreaming(fDelta, pos);
}

void CWorld::LateUpdate(CScene& scene)
{
	CChunkMesherSystem::RebuildDirtyChunks(scene, *m_pChunkWorld);
}

bool CWorld::RaycastBlock(IN const XMFLOAT3& origin, const XMFLOAT3& dirNorm, float maxDist, OUT BlockHitResult& outHitResult) const
{
	return BlockRaycastUtil::RaycastVoxelDDA(*m_pChunkWorld, origin, dirNorm, maxDist, BlockRaycastOptions(), outHitResult);
}

bool CWorld::TryPlaceBlock(int wx, int wy, int wz, const XMINT3& hitNormal, const BlockCell& cell)
{
	const BlockCell cur = m_pChunkWorld->GetBlock(wx, wy, wz);
	if (!cur.IsAir()) return false;

	BlockCell placed{};
	if (!_ResolvePlaceBlock(cell, hitNormal, placed))
		return false;

	if (!_CanPlaceResolvedBlock(wx, wy, wz, placed))
		return false;

	m_pChunkWorld->SetBlock(wx, wy, wz, placed);
	return true;
}

bool CWorld::TryBreakBlock(int wx, int wy, int wz)
{
	const BlockCell cur = m_pChunkWorld->GetBlock(wx, wy, wz);
	if (cur.IsAir()) return false;

	m_pChunkWorld->SetBlock(wx, wy, wz, BlockCell{ 0, 0 });
	return true;
}

BlockCell CWorld::GetBlockCell(int wx, int wy, int wz) const
{
	return m_pChunkWorld->GetBlock(wx, wy, wz);
}

bool CWorld::IsSolidBlockAt(int wx, int wy, int wz) const
{
	const BlockCell cell = GetBlockCell(wx, wy, wz);
	return m_pChunkWorld->IsSolid(cell);
}

bool CWorld::CheckAABBBlocked(const XMFLOAT3& center, const XMFLOAT3& halfExtents) const
{
	const XMFLOAT3 fMin =
	{
		center.x - halfExtents.x,
		center.y - halfExtents.y,
		center.z - halfExtents.z
	};

	const XMFLOAT3 fMax =
	{
		center.x + halfExtents.x,
		center.y + halfExtents.y,
		center.z + halfExtents.z
	};

	const XMINT3 iMin =
	{
		static_cast<int>(std::floor(fMin.x)),
		static_cast<int>(std::floor(fMin.y)),
		static_cast<int>(std::floor(fMin.z))
	};

	const XMINT3 iMax =
	{
		static_cast<int>(std::floor(fMax.x)),
		static_cast<int>(std::floor(fMax.y)),
		static_cast<int>(std::floor(fMax.z))
	};

	for (int y = iMin.y; y <= iMax.y; ++y)
	{
		for (int z = iMin.z; z <= iMax.z; ++z)
		{
			for (int x = iMin.x; x <= iMax.x; ++x)
			{
				if (IsSolidBlockAt(x, y, z)) 
					return true;
			}
		}
	}
	return false;
}

bool CWorld::FindSpawnFootY(int wx, int wz, const XMFLOAT3& halfExtents, float& outFootY) const
{
	outFootY = 0.f;

	for (int y = CHUNK_SIZE_Y - 2; y >= 0; --y)
	{
		if (!IsSolidBlockAt(wx, y, wz))
			continue;

		const float footY = static_cast<float>(y + 1) + 0.001f;
		const XMFLOAT3 center =
		{
			static_cast<float>(wx) + 0.5f,
			footY + halfExtents.y,
			static_cast<float>(wz) + 0.5f,
		};

		if (!CheckAABBBlocked(center, halfExtents))
		{
			outFootY = footY;
			return true;
		}
	}

	return false;
}

bool CWorld::_ResolvePlaceBlock(const BlockCell& selected, const XMINT3& hitNormal, BlockCell& outPlaced) const
{
	const BLOCK_ID torchID = BlockDB.FindBlockID("minecraft:torch");
	if (selected.blockID == torchID)
		return _ResolveTorchPlacement(hitNormal, outPlaced);

	outPlaced = selected;
	return true;
}

bool CWorld::_ResolveTorchPlacement(const XMINT3& hitNormal, BlockCell& outPlaced) const
{
	if (hitNormal.y < 0)
		return false;

	if (hitNormal.y > 0)
	{
		BlockPropHashMap props;
		BLOCK_ID id = BlockDB.FindBlockID("minecraft:torch");
		STATE_INDEX sidx{};
		if (!BlockDB.EncodeStateIndex(id, props, sidx))
			return false;

		outPlaced = { id, sidx };
		return true;
	}

	if (hitNormal.x != 0 || hitNormal.z != 0)
	{
		BlockPropHashMap props;
		BLOCK_ID id = BlockDB.FindBlockID("minecraft:wall_torch");

		if (hitNormal.x > 0)      props[fnv1a_64("facing")] = fnv1a_64("east");
		else if (hitNormal.x < 0) props[fnv1a_64("facing")] = fnv1a_64("west");
		else if (hitNormal.z > 0) props[fnv1a_64("facing")] = fnv1a_64("south");
		else if (hitNormal.z < 0) props[fnv1a_64("facing")] = fnv1a_64("north");

		STATE_INDEX sidx{};
		if (!BlockDB.EncodeStateIndex(id, props, sidx))
			return false;

		outPlaced = { id, sidx };
		return true;
	}

	return false;
}

bool CWorld::_CanPlaceResolvedBlock(int wx, int wy, int wz, const BlockCell& placed) const
{
	const BLOCK_ID torchID = BlockDB.FindBlockID("minecraft:torch");
	const BLOCK_ID wallTorchID = BlockDB.FindBlockID("minecraft:wall_torch");

	if (placed.blockID == torchID)
		return _CanSupportTorchFloor(wx, wy, wz);

	if (placed.blockID == wallTorchID)
		return _CanSupportWallTorch(wx, wy, wz, placed);

	return true;
}

bool CWorld::_CanSupportTorchFloor(int wx, int wy, int wz) const
{
	const BlockCell below = m_pChunkWorld->GetBlock(wx, wy - 1, wz);
	if (below.IsAir())
		return false;

	return BlockDB.IsFaceOccluder(below.blockID);
}

bool CWorld::_CanSupportWallTorch(int wx, int wy, int wz, const BlockCell& placed) const
{
	uint64_t facingHash = 0;
	if (!BlockDB.TryGetStateValueHash(placed.blockID, placed.stateIndex, fnv1a_64("facing"), facingHash))
		return false;

	XMINT3 supportDir{};

	if (!GetWallTorchSupportDirFromFacing(facingHash, supportDir))
		return false;

	const BlockCell support = m_pChunkWorld->GetBlock(wx + supportDir.x, wy + supportDir.y, wz + supportDir.z);
	if (support.IsAir())
		return false;

	return BlockDB.IsFaceOccluder(support.blockID);
}