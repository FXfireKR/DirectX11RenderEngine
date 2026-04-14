#pragma once
#include "CChunkWorld.h"
#include "CWorldTime.h"
#include "CWorldLight.h"
#include "BlockRaycastTypes.h"

/*
CWorld
 └─ CChunkWorld : public IBlockAccessor
	  └─ ChunkColumn
		   └─ ChunkSection
				└─ BlockCell[]

*/

class CScene;
class CPipeline;
class CMaterial;

class CWorld
{
public:
	CWorld();
	~CWorld() = default;

	void Initialize(CScene& scene, CPipeline* pOpaquePipeline, CMaterial* pOpaqueMaterial
		, CPipeline* pCutoutPipeline, CMaterial* pCutoutMaterial
		, CPipeline* pTranslucentPipeline, CMaterial* pTranslucentMaterial);
	void Update(float fDelta, XMFLOAT3 pos);
	void LateUpdate(CScene& scene);

	bool RaycastBlock(IN const XMFLOAT3& origin, const XMFLOAT3& dirNorm, float maxDist, OUT BlockHitResult& outHitResult) const;
	bool TryPlaceBlock(int wx, int wy, int wz, const XMINT3& hitNormal, const BlockCell& cell);
	bool TryBreakBlock(int wx, int wy, int wz);

	BlockCell GetBlockCell(int wx, int wy, int wz) const;
	bool IsSolidBlockAt(int wx, int wy, int wz) const;
	bool CheckAABBBlocked(const XMFLOAT3& center, const XMFLOAT3& halfExtents) const;
	bool FindSpawnFootY(int wx, int wz, const XMFLOAT3& halfExtents, float& outFootY) const;

	inline WorldTimeParams BuildWorldTime() const { return m_worldTime.Evaluate(); }
	inline WorldLightingParams BuildWorldLighting() const { return m_worldLight.Evaluate(BuildWorldTime()); }

private:
	bool _ResolvePlaceBlock(const BlockCell& selected, const XMINT3& hitNormal, BlockCell& outPlaced) const;
	bool _ResolveTorchPlacement(const XMINT3& hitNormal, BlockCell& outPlaced) const;

	bool _CanPlaceResolvedBlock(int wx, int wy, int wz, const BlockCell& placed) const;
	bool _CanSupportTorchFloor(int wx, int wy, int wz) const;
	bool _CanSupportWallTorch(int wx, int wy, int wz, const BlockCell& placed) const;

public:
	inline CChunkWorld& GetChunkWorld() { return *(m_pChunkWorld.get()); }
	inline const CChunkWorld& GetChunkWorld() const { return *(m_pChunkWorld.get()); }

	inline CWorldTime& GetWorldTime() { return m_worldTime; }
	inline const CWorldTime& GetWorldTime() const { return m_worldTime; }

private:
	unique_ptr<CChunkWorld> m_pChunkWorld;
	CWorldTime m_worldTime;
	CWorldLight m_worldLight;
};