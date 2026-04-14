#include "pch.h"
#include "CBlockBreakParticleSystem.h"

#include "CRenderWorld.h"
#include "CCamera.h"
#include "CTransform.h"
#include "CMeshManager.h"
#include "CShaderManager.h"
#include "CInputLayerManager.h"
#include "CPipelineManager.h"
#include "CMaterialManager.h"
#include "CSamplerManager.h"
#include "CMesh.h"
#include "CPipeline.h"
#include "CMaterial.h"
#include "CWorld.h"

#include "ChunkTypes.h"
#include "ModelUtil.h"

void CBlockBreakParticleSystem::Initialize(CRenderWorld& rw)
{
	m_uMeshKey = fnv1a_64("BlockBreakParticleDynamicMesh");

	m_vecParticles.clear();
	m_vecParticles.resize(m_uMaxParticles);

	_CreateRenderResource(rw);

	srand(GetTickCount64());
}

void CBlockBreakParticleSystem::Update(float fDelta)
{
	for (BlockBreakParticle& p : m_vecParticles)
	{
		if (!p.bAlive)
			continue;

		p.fLife -= fDelta;
		if (p.fLife <= 0.f)
		{
			p.bAlive = false;
			continue;
		}

		p.vel.y -= 9.8f * 1.35f * fDelta;

		p.pos.x += p.vel.x * fDelta;
		p.pos.y += p.vel.y * fDelta;
		p.pos.z += p.vel.z * fDelta;
	}
}

void CBlockBreakParticleSystem::SpawnHitChip(const XMINT3& blockPos, const BlockCell& cell, const XMINT3& hitNormal)
{
	const BakedModel* model = BlockDB.GetBakedModel(cell.blockID, cell.stateIndex);
	if (!model || model->quads.empty())
		return;

	uint8_t eFaceDir = static_cast<uint8_t>(NormalToFaceDir(hitNormal));

	const BakedQuad* picked = nullptr;
	for (const BakedQuad& q : model->quads)
	{
		if (q.dir == eFaceDir)
		{
			picked = &q;
			break;
		}
	}

	if (!picked)
		picked = &model->quads.front();

	AtlasRegion region{};
	if (!BlockResDB.TryGetRegion(picked->debugTextureKey.c_str(), region))
		return;

	const XMFLOAT2 uvMin = { region.u0, region.v0 };
	const XMFLOAT2 uvMax = { region.u1, region.v1 };

	const XMFLOAT4 particleColor = _ResolveParticleColor(*picked, blockPos);

	const XMFLOAT3 spawnPos =
	{
		blockPos.x + 0.5f + hitNormal.x * 0.45f,
		blockPos.y + 0.5f + hitNormal.y * 0.45f,
		blockPos.z + 0.5f + hitNormal.z * 0.45f,
	};

	const XMFLOAT3 normal =
	{
		static_cast<float>(hitNormal.x),
		static_cast<float>(hitNormal.y),
		static_cast<float>(hitNormal.z)
	};

	int iTartgetParticle = 3 + (rand() % 4);
	for (int i = 0; i < iTartgetParticle; ++i)
	{
		const XMFLOAT3 randDir =
		{
			_RandomRange(-0.5f, 0.5f),
			_RandomRange(0.f, 0.5f),
			_RandomRange(-0.5f, 0.5f),
		};

		const XMFLOAT3 vel =
		{
			normal.x * 0.35f + randDir.x * 0.45f,
			normal.y * 0.25f + randDir.y * 0.55f,
			normal.z * 0.35f + randDir.z * 0.45f,
		};

		const float size = _RandomRange(0.04f, 0.07f);
		const float life = _RandomRange(0.12f, 0.22f);

		_EmitOne(spawnPos, vel, size, life, uvMin, uvMax, particleColor);
	}
}

void CBlockBreakParticleSystem::SpawnBreakBurst(const XMINT3& blockPos, const BlockCell& cell, const XMINT3& hitNormal)
{
	const BakedModel* model = BlockDB.GetBakedModel(cell.blockID, cell.stateIndex);
	if (!model || model->quads.empty())
		return;

	uint8_t eFaceDir = static_cast<uint8_t>(NormalToFaceDir(hitNormal));

	const BakedQuad* picked = nullptr;
	for (const BakedQuad& q : model->quads)
	{
		if (q.dir == eFaceDir)
		{
			picked = &q;
			break;
		}
	}

	if (!picked)
		picked = &model->quads.front();

	XMFLOAT2 uvMin{}, uvMax{};
	if (!BlockResDB.TryGetBlockParticleRegion(picked->debugTextureKey.c_str(), uvMin, uvMax))
		return;

	const float uLen = uvMax.x - uvMin.x;
	const float vLen = uvMax.y - uvMin.y;

	float uSize = _RandomRange(0.f, uLen);
	float vSize = _RandomRange(0.f, vLen);

	float startU = _RandomRange(uvMin.x, uvMax.x - uSize);
	float startV = _RandomRange(uvMin.y, uvMax.y - vSize);

	const XMFLOAT3 center =
	{
		static_cast<float>(blockPos.x) + 0.5f,
		static_cast<float>(blockPos.y) + 0.5f,
		static_cast<float>(blockPos.z) + 0.5f
	};

	const XMFLOAT3 normal =
	{
		static_cast<float>(hitNormal.x),
		static_cast<float>(hitNormal.y),
		static_cast<float>(hitNormal.z)
	};

	const XMFLOAT4 particleColor = _ResolveParticleColor(*picked, blockPos);

	int iTargetParticle = 3 + (rand() % 3);
	for (int i = 0; i < iTargetParticle; ++i)
	{
		const XMFLOAT3 randDir =
		{
			_RandomRange(-1.f, 1.f),
			_RandomRange(0.f, 2.f),
			_RandomRange(-1.f, 1.f),
		};

		const XMFLOAT3 vel =
		{
			normal.x * 0.9f + randDir.x * 1.4f,
			normal.y * 0.6f + randDir.y * 1.7f,
			normal.z * 0.9f + randDir.z * 1.4f,
		};

		const float size = _RandomRange(0.08f, 0.14f);
		const float life = _RandomRange(0.35f, 0.55f);

		_EmitOne(center, vel, size, life, { startU, startV }, { startU + uSize, startV + vSize }, particleColor);
	}
}

void CBlockBreakParticleSystem::SubmitRender(CRenderWorld& rw, const CCamera& camera)
{
	_BuildDynamicMesh(rw, camera);

	if (nullptr == m_pMesh || nullptr == m_pPipeline || nullptr == m_pMaterial)
		return;

	if (m_vecVertices.empty() || m_vecIndices.empty())
		return;

	RenderItem item{};
	item.eRenderPass = ERenderPass::TRANSPARENT_PASS;
	item.pMesh = m_pMesh;
	item.pPipeline = m_pPipeline;
	item.pMaterial = m_pMaterial;
	XMStoreFloat4x4(&item.world, XMMatrixTranspose(XMMatrixIdentity()));

	rw.Submit(item);
}

void CBlockBreakParticleSystem::_CreateRenderResource(CRenderWorld& rw)
{
	auto& shaderManager = rw.GetShaderManager();
	auto& ilManager = rw.GetIALayoutManager();
	auto& pipelineManager = rw.GetPipelineManager();
	auto& materialManager = rw.GetMaterialManager();
	auto& samplerManager = rw.GetSamplerManager();
	auto& meshManager = rw.GetMeshManager();

	const uint64_t shaderID = fnv1a_64("BlockParticleBillboard");
	auto* shader = shaderManager.CreateShader(shaderID, 0);
	shaderManager.Compile();

	const uint64_t layoutID = ilManager.Create(VERTEX_POSITION_UV_COLOR::GetLayout(), { shaderID, 0 }, shader->GetVertexBlob());

	const uint64_t pipeID = pipelineManager.Create(fnv1a_64("BlockBreakParticlePipeline"));
	m_pPipeline = pipelineManager.Get(pipeID);
	m_pPipeline->SetShader(shaderManager.Get(shaderID, 0));
	m_pPipeline->SetInputLayout(ilManager.Get(layoutID));
	m_pPipeline->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pPipeline->CreateTransparentAlphaState(rw.GetDevice(), true);

	const uint64_t materialID = materialManager.Create(fnv1a_64("BlockBreakParticleMaterial"));
	m_pMaterial = materialManager.Get(materialID);

	const uint64_t samplerID = samplerManager.Create(SAMPLER_TYPE::POINT_WRAP);
	m_pMaterial->SetSampler(0, samplerManager.Get(samplerID)->Get());
	m_pMaterial->SetTexture(0, BlockResDB.GetAtlasTextureView());

	meshManager.Create(m_uMeshKey);
	m_pMesh = meshManager.Get(m_uMeshKey);
}

void CBlockBreakParticleSystem::_BuildDynamicMesh(CRenderWorld& rw, const CCamera& camera)
{
	m_vecVertices.clear();
	m_vecIndices.clear();

	const CTransform* pCamTr = camera.GetTransform();
	if (nullptr == pCamTr)
		return;

	XMFLOAT3 camRight = pCamTr->GetRight();
	XMFLOAT3 camUp = pCamTr->GetUp();

	XMVECTOR vRightBase = XMVector3Normalize(XMLoadFloat3(&camRight));
	XMVECTOR vUpBase = XMVector3Normalize(XMLoadFloat3(&camUp));

	uint32_t baseIndex = 0;

	for (const BlockBreakParticle& p : m_vecParticles)
	{
		if (!p.bAlive) continue;

		const float half = p.fSize * 0.5f;

		XMVECTOR vCenter = XMLoadFloat3(&p.pos);
		XMVECTOR vRight = vRightBase * half;
		XMVECTOR vUp = vUpBase * half;

		XMVECTOR v0 = vCenter - vRight + vUp;
		XMVECTOR v1 = vCenter + vRight + vUp;
		XMVECTOR v2 = vCenter - vRight - vUp;
		XMVECTOR v3 = vCenter + vRight - vUp;

		XMFLOAT3 p0{}, p1{}, p2{}, p3{};
		XMStoreFloat3(&p0, v0);
		XMStoreFloat3(&p1, v1);
		XMStoreFloat3(&p2, v2);
		XMStoreFloat3(&p3, v3);

		m_vecVertices.push_back({ p0, { p.uvMin.x, p.uvMin.y }, p.color });
		m_vecVertices.push_back({ p1, { p.uvMax.x, p.uvMin.y }, p.color });
		m_vecVertices.push_back({ p2, { p.uvMin.x, p.uvMax.y }, p.color });
		m_vecVertices.push_back({ p3, { p.uvMax.x, p.uvMax.y }, p.color });

		m_vecIndices.push_back(baseIndex + 0);
		m_vecIndices.push_back(baseIndex + 1);
		m_vecIndices.push_back(baseIndex + 2);

		m_vecIndices.push_back(baseIndex + 2);
		m_vecIndices.push_back(baseIndex + 1);
		m_vecIndices.push_back(baseIndex + 3);

		baseIndex += 4;
	}

	if (m_vecVertices.empty() || m_vecIndices.empty())
		return;

	m_pMesh = rw.GetMeshManager().CreateOrUpdateDynamicMesh(
		rw.GetContext(),
		m_uMeshKey,
		m_vecVertices.data(),
		sizeof(VERTEX_POSITION_UV_COLOR),
		static_cast<uint32_t>(m_vecVertices.size()),
		m_vecIndices.data(),
		static_cast<uint32_t>(m_vecIndices.size())
	);
}

void CBlockBreakParticleSystem::_EmitOne(const XMFLOAT3& pos, const XMFLOAT3& vel, float size, float life, const XMFLOAT2& uvMin, const XMFLOAT2& uvMax, const XMFLOAT4& color)
{
	for (BlockBreakParticle& p : m_vecParticles)
	{
		if (p.bAlive) 
			continue;

		p.bAlive = true;
		p.pos = pos;
		p.vel = vel;
		p.fSize = size;
		p.fLife = life;
		p.fMaxLife = life;
		p.uvMin = uvMin;
		p.uvMax = uvMax;
		p.color = color;
		return;
	}
}

float CBlockBreakParticleSystem::_RandomRange(float minV, float maxV) const
{
	const float t = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	return minV + (maxV - minV) * t;
}

uint8_t CBlockBreakParticleSystem::_ResolveQuadBlockLight(const BakedQuad& quad, const XMINT3& blockPos) const
{
	if (!m_pWorld)
		return 15;

	const CChunkWorld& world = m_pWorld->GetChunkWorld();

	if (quad.bHasCullFace)
	{
		const XMINT3 n = FaceToNormalInt3(static_cast<FACE_DIR>(quad.cullFaceDir));
		return world.GetBlockLight(blockPos.x + n.x, blockPos.y + n.y, blockPos.z + n.z);
	}

	return world.GetBlockLight(blockPos.x, blockPos.y, blockPos.z);
}

XMFLOAT4 CBlockBreakParticleSystem::_ResolveBlockTint(const BakedQuad& quad) const
{
	if (quad.tintIndex < 0)
		return { 1.f, 1.f, 1.f, 1.f };

	// 현재 chunk mesh builder와 동일 기준
	return { 0.55f, 0.74f, 0.32f, 1.f };
}

XMFLOAT4 CBlockBreakParticleSystem::_ResolveParticleColor(const BakedQuad& quad, const XMINT3& blockPos) const
{
	const uint8_t light = _ResolveQuadBlockLight(quad, blockPos);
	const float blockLight = static_cast<float>(light) / 15.0f;
	const float brightness = 0.1f + (0.9f * blockLight);

	const XMFLOAT4 tint = _ResolveBlockTint(quad);

	return {
		tint.x * brightness,
		tint.y * brightness,
		tint.z * brightness,
		tint.w
	};
}