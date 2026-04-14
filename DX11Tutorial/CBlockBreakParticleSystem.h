#pragma once
#include "VertexTypes.h"
#include "RenderPassTypes.h"

class CRenderWorld;
class CCamera;
class CMesh;
class CPipeline;
class CMaterial;
class CWorld;

struct BlockCell;
struct BakedQuad;

struct BlockBreakParticle
{
	bool bAlive = false;
	XMFLOAT3 pos{};
	XMFLOAT3 vel{};

	XMFLOAT2 uvMin{};
	XMFLOAT2 uvMax{};

	XMFLOAT4 color{ 1.f, 1.f, 1.f, 1.f };

	float fSize = 0.1f;
	float fLife = 0.f;
	float fMaxLife = 0.f;
};

class CBlockBreakParticleSystem
{
public:
	void Initialize(CRenderWorld& rw);
	void Update(float fDelta);

	void SpawnHitChip(const XMINT3& blockPos, const BlockCell& cell, const XMINT3& hitNormal);
	void SpawnBreakBurst(const XMINT3& blockPos, const BlockCell& cell, const XMINT3& hitNormal);
	void SubmitRender(CRenderWorld& rw, const CCamera& camera);

public:
	inline void SetWorld(const CWorld* pWorld) { m_pWorld = pWorld; }

private:
	void _CreateRenderResource(CRenderWorld& rw);
	void _BuildDynamicMesh(CRenderWorld& rw, const CCamera& camera);
	void _EmitOne(const XMFLOAT3& pos, const XMFLOAT3& vel, float size, float life
		, const XMFLOAT2& uvMin, const XMFLOAT2& uvMax, const XMFLOAT4& color);
	float _RandomRange(float minV, float maxV) const;

	uint8_t _ResolveQuadBlockLight(const BakedQuad& quad, const XMINT3& blockPos) const;
	XMFLOAT4 _ResolveBlockTint(const BakedQuad& quad) const;
	XMFLOAT4 _ResolveParticleColor(const BakedQuad& quad, const XMINT3& blockPos) const;

private:
	vector<BlockBreakParticle> m_vecParticles;
	vector<VERTEX_POSITION_UV_COLOR> m_vecVertices;
	vector<uint32_t> m_vecIndices;

	CMesh* m_pMesh = nullptr;
	CPipeline* m_pPipeline = nullptr;
	CMaterial* m_pMaterial = nullptr;

	const CWorld* m_pWorld = nullptr;

	uint64_t m_uMeshKey = 0;
	uint32_t m_uMaxParticles = 256;
};