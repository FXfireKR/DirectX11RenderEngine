#pragma once
#include "CMesh.h"

class CMeshManager
{
public:
	CMeshManager() = default;
	~CMeshManager() = default;

	void Initialize(ID3D11Device& refDevice_);
	uint64_t Create(uint64_t id);
	CMesh* CreateOrUpdateDynamicMesh(ID3D11DeviceContext* pContext, uint64_t meshKey, const void* pVertices, uint32_t vertexStride, uint32_t vertexCnt
		, const uint32_t* pIndices, uint32_t indexCnt);
	CMesh* Get(uint64_t id);
	const CMesh* Get(uint64_t id) const;

public:
	uint64_t CreateTriangle(uint64_t id);
	uint64_t CreateQuad(uint64_t id);
	uint64_t CreateCube(uint64_t id);
	uint64_t CreateMeshFromBakedModel(MODEL_ID modelID, const CRuntimeAtlas& atlas);
	uint64_t CreateAABBLine(uint64_t id);


private:
	std::unordered_map<uint64_t, unique_ptr<CMesh>> m_mapMesh;

	ID3D11Device* m_pDevice = nullptr;
};