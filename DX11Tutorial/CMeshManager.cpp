#include "pch.h"
#include "CMeshManager.h"
#include "CRuntimeAtlas.h"

void CMeshManager::Initialize(ID3D11Device& refDevice_)
{
	m_pDevice = &refDevice_;
}

uint64_t CMeshManager::Create(uint64_t id)
{
	auto iter = m_mapMesh.find(id);
	if (iter == m_mapMesh.end()) {
		m_mapMesh.insert(make_pair(id, make_unique<CMesh>()));
	}
	return id;
}

CMesh* CMeshManager::CreateOrUpdateDynamicMesh(ID3D11DeviceContext* pContext, uint64_t meshKey
	, const void* pVertices, uint32_t vertexStride, uint32_t vertexCnt
	, const uint32_t* pIndices, uint32_t indexCnt)
{
	PROFILE_SCOPE();

	CMesh* pMesh = nullptr;
	auto it = m_mapMesh.find(meshKey);
	if (it == m_mapMesh.end()) {
		auto meshUPtr = make_unique<CMesh>();
		pMesh = meshUPtr.get();
		m_mapMesh.emplace(meshKey, std::move(meshUPtr));
	}
	else {
		pMesh = it->second.get();
	}

	if (!pMesh->UpdateDynamic(m_pDevice, pContext, pVertices, vertexStride, vertexCnt, pIndices, indexCnt))
		return nullptr;

	return pMesh;
}

CMesh* CMeshManager::Get(uint64_t id)
{
	auto iter = m_mapMesh.find(id);
	if (iter != m_mapMesh.end()) {
		return iter->second.get();
	}
	return nullptr;
}

const CMesh* CMeshManager::Get(uint64_t id) const
{
	auto iter = m_mapMesh.find(id);
	if (iter != m_mapMesh.end()) {
		return iter->second.get();
	}
	return nullptr;
}

uint64_t CMeshManager::CreateTriangle(uint64_t id)
{
	auto meshID = this->Create(id);
	this->Get(meshID)->CreateTriangle(m_pDevice);
	return id;
}

uint64_t CMeshManager::CreateQuad(uint64_t id)
{
	auto meshID = this->Create(id);
	this->Get(meshID)->CreateQuad(m_pDevice);
	return id;
}

uint64_t CMeshManager::CreateCube(uint64_t id)
{
	auto meshID = this->Create(id);
	this->Get(meshID)->CreateCube(m_pDevice);
	return id;
}

uint64_t CMeshManager::CreateMeshFromBakedModel(MODEL_ID modelID, const CRuntimeAtlas& atlas)
{
	/*const BakedModel* baked = BlockDB.GetBakedModel(modelID);
	if (!baked) return UINT64_ERROR;
	auto meshID = this->Create(modelID);
	this->Get(meshID)->CreateMeshFromBakedModel(m_pDevice, atlas, *baked);
	return meshID;*/
	return 0;
}

uint64_t CMeshManager::CreateAABBLine(uint64_t id)
{
	auto meshID = this->Create(id);
	this->Get(meshID)->CreateAABBLine(m_pDevice);
	return id;
}
