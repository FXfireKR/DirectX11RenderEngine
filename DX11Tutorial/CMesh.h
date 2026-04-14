#pragma once
#include "VertexTypes.h"

class CRuntimeAtlas;

struct SubMesh
{
	UINT uVertexStart;
	UINT uVertexCount;
};

class CMesh
{
public:
	CMesh() = default;
	~CMesh() = default;

	void CreateTriangle(ID3D11Device* pDevice);
	void CreateQuad(ID3D11Device* pDevice);
	void CreateCube(ID3D11Device* pDevice);
	void CreateMeshFromBakedModel(ID3D11Device* pDevice, const CRuntimeAtlas& atlas, const BakedModel& bakedModel);
	void CreateAABBLine(ID3D11Device* pDevice);

	
	bool UpdateDynamic(ID3D11Device* pDevice, ID3D11DeviceContext* pContext
		, const void* pVertices, uint32_t vertexStride, uint32_t vertexCnt
		, const uint32_t* pIndices, uint32_t indexCnt);

	void Bind(ID3D11DeviceContext* pContext) const;
	void Draw(ID3D11DeviceContext* pContext) const;

public:
	inline vector<SubMesh>& GetSubMesh() { return m_vecSubMesh; }
	inline ID3D11Buffer* GetVertexBuffer() const { return m_pVertexBuffer.Get(); }
	
private:
	vector<SubMesh> m_vecSubMesh;

	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ComPtr<ID3D11Buffer> m_pIndexBuffer;

	uint32_t m_uVertexStride = 0;
	uint32_t m_uVertexCnt = 0;
	uint32_t m_uIndexCnt = 0;

	uint32_t m_vbCapacityBytes = 0;
	uint32_t m_ibCapacityBytes = 0;
};