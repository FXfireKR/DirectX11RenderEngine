#include "pch.h"
#include "CMesh.h"
#include "CRuntimeAtlas.h"

void CMesh::CreateTriangle(ID3D11Device* pDevice)
{
	VERTEX_POSITION_COLOR vert[3] =
	{
		{ {-0.5f, -0.5f, 0.0f}, {0,0,1,1} } ,
		{ {0.0f, 0.5f, 0.0f}, {1,0,1,1} },
		{ {0.5f, -0.5f, 0.0f}, {0,1,0,1} },
	};

	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = sizeof(vert);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = vert;

	pDevice->CreateBuffer(&desc, &data, m_pVertexBuffer.GetAddressOf());
}

void CMesh::CreateQuad(ID3D11Device* pDevice)
{
	VERTEX_POSITION_UV vert[4] =
	{
		{ {-0.5f, 0.5f, 0.0f}, {0.f, 0.f} },
		{ {0.5f, 0.5f, 0.0f}, {1.f, 0.f} },
		{ {-0.5f, -0.5f, 0.0f}, {0.f, 1.f} } ,
		{ {0.5f, -0.5f, 0.0f}, {1.f, 1.f} } ,
	};

	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = sizeof(vert);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = vert;

	pDevice->CreateBuffer(&desc, &data, m_pVertexBuffer.GetAddressOf());

	uint32_t indices[6] =
	{
		0, 1, 2,
		2, 1, 3
	};

	D3D11_BUFFER_DESC descIndex{};
	descIndex.ByteWidth = sizeof(indices);
	descIndex.Usage = D3D11_USAGE_DEFAULT;
	descIndex.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA dataIndex{};
	dataIndex.pSysMem = indices;

	pDevice->CreateBuffer(&descIndex, &dataIndex, m_pIndexBuffer.GetAddressOf());

	m_uVertexStride = sizeof(VERTEX_POSITION_UV);
	m_uVertexCnt = 4;
	m_uIndexCnt = 6;
}

void CMesh::CreateCube(ID3D11Device* pDevice)
{
	VERTEX_POSITION_UV_NORMAL vert[36] =
	{
		{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },

		{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f }, { 1.0f, 0.0f,  0.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f }, { 1.0f, 0.0f,  0.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, { 1.0f, 0.0f,  0.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, { 1.0f, 0.0f,  0.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f }, { 1.0f, 0.0f,  0.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f }, { 1.0f, 0.0f,  0.0f } },

		{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f }, { 0.0f, 0.0f,  1.0f } },
		{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f }, { 0.0f, 0.0f,  1.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f }, { 0.0f, 0.0f,  1.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f }, { 0.0f, 0.0f,  1.0f } },
		{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f }, { 0.0f, 0.0f,  1.0f } },
		{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f }, { 0.0f, 0.0f,  1.0f } },

		{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
		{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
		{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },

		{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f }, { 0.0f,  1.0f, 0.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f }, { 0.0f,  1.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f }, { 0.0f,  1.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f }, { 0.0f,  1.0f, 0.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f }, { 0.0f,  1.0f, 0.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f }, { 0.0f,  1.0f, 0.0f } },

		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
	};

	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = sizeof(vert);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = vert;

	pDevice->CreateBuffer(&desc, &data, m_pVertexBuffer.GetAddressOf());
}

void CMesh::CreateMeshFromBakedModel(ID3D11Device* pDevice, const CRuntimeAtlas& atlas,  const BakedModel& bakedModel)
{
	//vector<VERTEX_POSITION_UV_NORMAL> vert;
	//vector<uint32_t> indices;

	//size_t offset = 0;
	//for (auto& quad : bakedModel.quads)
	//{
	//	const uint16_t tileID = atlas.GetTileID(quad.textureHash);
	//	const UVRect r = (tileID == UINT16_MAX) ? atlas.GetUV(0) : atlas.GetUV(tileID);

	//	for (auto& vertex : quad.verts)
	//	{
	//		const XMFLOAT2 uvAtlas
	//		{
	//			r.u0 + (r.u1 - r.u0) * vertex.uv.x,
	//			r.v0 + (r.v1 - r.v0) * vertex.uv.y
	//		};

	//		vert.push_back({ {vertex.pos}, {uvAtlas}, {vertex.normal} });
	//	}

	//	indices.push_back(offset + 0);
	//	indices.push_back(offset + 1);
	//	indices.push_back(offset + 2);

	//	indices.push_back(offset + 0);
	//	indices.push_back(offset + 2);
	//	indices.push_back(offset + 3);

	//	offset = vert.size();
	//}

	//// Create VertexBuffer
	//D3D11_BUFFER_DESC desc{};
	//desc.ByteWidth = sizeof(VERTEX_POSITION_UV_NORMAL) * vert.size();
	//desc.Usage = D3D11_USAGE_DEFAULT;
	//desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//D3D11_SUBRESOURCE_DATA data{};
	//data.pSysMem = vert.data();
	//pDevice->CreateBuffer(&desc, &data, m_pVertexBuffer.GetAddressOf());

	//// Create IndexBuffer
	//D3D11_BUFFER_DESC descIndex{};
	//descIndex.ByteWidth = sizeof(uint32_t) * indices.size();
	//descIndex.Usage = D3D11_USAGE_DEFAULT;
	//descIndex.BindFlags = D3D11_BIND_INDEX_BUFFER;
	//D3D11_SUBRESOURCE_DATA dataIndex{};
	//dataIndex.pSysMem = indices.data();
	//pDevice->CreateBuffer(&descIndex, &dataIndex, m_pIndexBuffer.GetAddressOf());

	//m_uVertexStride = sizeof(VERTEX_POSITION_UV_NORMAL);
	//m_uVertexCnt = vert.size();
	//m_uIndexCnt = indices.size();
}

void CMesh::CreateAABBLine(ID3D11Device* pDevice)
{
	VERTEX_POSITION vert[8] =
	{
		{{0,0,0}},
		{{1,0,0}},
		{{1,1,0}},
		{{0,1,0}},
		{{0,0,1}},
		{{1,0,1}},
		{{1,1,1}},
		{{0,1,1}},
	};

	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = sizeof(vert);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = vert;

	pDevice->CreateBuffer(&desc, &data, m_pVertexBuffer.GetAddressOf());

	uint32_t indices[24] =
	{
		0,1, 1,2, 2,3, 3,0,
		4,5, 5,6, 6,7, 7,4,
		0,4, 1,5, 2,6, 3,7
	};

	D3D11_BUFFER_DESC descIndex{};
	descIndex.ByteWidth = sizeof(indices);
	descIndex.Usage = D3D11_USAGE_DEFAULT;
	descIndex.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA dataIndex{};
	dataIndex.pSysMem = indices;

	pDevice->CreateBuffer(&descIndex, &dataIndex, m_pIndexBuffer.GetAddressOf());

	m_uVertexStride = sizeof(VERTEX_POSITION);
	m_uVertexCnt = 8;
	m_uIndexCnt = 24;
}

bool CMesh::UpdateDynamic(ID3D11Device* pDevice, ID3D11DeviceContext* pContext
	, const void* pVertices, uint32_t vertexStride, uint32_t vertexCnt
	, const uint32_t* pIndices, uint32_t indexCnt
)
{
	if (vertexCnt == 0 || indexCnt == 0) 
	{
		m_uVertexStride = vertexStride;
		m_uVertexCnt = vertexCnt;
		m_uIndexCnt = indexCnt;
		m_vecSubMesh.clear();
		return true;
	}

	const uint32_t vbBytes = vertexStride * vertexCnt;
	const uint32_t ibBytes = sizeof(uint32_t) * indexCnt;

	// =========================
	// Vertex Buffer
	// =========================
	if (!m_pVertexBuffer || vbBytes > m_vbCapacityBytes)
	{
		// ComPtr 방식 해제
		m_pVertexBuffer.Reset();
		m_vbCapacityBytes = 0;

		D3D11_BUFFER_DESC bd{};
		bd.ByteWidth = vbBytes;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA sd{};
		sd.pSysMem = pVertices;

		HRESULT hr = pDevice->CreateBuffer(&bd, &sd, m_pVertexBuffer.GetAddressOf());
		if (FAILED(hr)) return false;

		m_vbCapacityBytes = vbBytes;
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE ms{};
		HRESULT hr = pContext->Map(m_pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		if (FAILED(hr)) return false;

		std::memcpy(ms.pData, pVertices, vbBytes);
		pContext->Unmap(m_pVertexBuffer.Get(), 0);
	}

	// =========================
	// Index Buffer
	// =========================
	if (!m_pIndexBuffer || ibBytes > m_ibCapacityBytes)
	{
		m_pIndexBuffer.Reset();
		m_ibCapacityBytes = 0;

		D3D11_BUFFER_DESC bd{};
		bd.ByteWidth = ibBytes;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA sd{};
		sd.pSysMem = pIndices;

		HRESULT hr = pDevice->CreateBuffer(&bd, &sd, m_pIndexBuffer.GetAddressOf());
		if (FAILED(hr)) return false;

		m_ibCapacityBytes = ibBytes;
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE ms{};
		HRESULT hr = pContext->Map(m_pIndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
		if (FAILED(hr)) return false;

		std::memcpy(ms.pData, pIndices, ibBytes);
		pContext->Unmap(m_pIndexBuffer.Get(), 0);
	}

	// counts
	m_uVertexStride = vertexStride;
	m_uVertexCnt = vertexCnt;
	m_uIndexCnt = indexCnt;

	// 청크는 submesh 1개로 고정해도 충분
	m_vecSubMesh.clear();
	m_vecSubMesh.push_back(SubMesh{ 0u, m_uVertexCnt });

	return true;
}

void CMesh::Bind(ID3D11DeviceContext* pContext) const
{
	UINT offset = 0;

	pContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &m_uVertexStride, &offset);

	if (m_pIndexBuffer) {
		pContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	}
}

void CMesh::Draw(ID3D11DeviceContext* pContext) const
{
	if (!m_pIndexBuffer) 
	{		
		pContext->Draw(m_uVertexCnt, 0);
	}
	else 
	{
		pContext->DrawIndexed(m_uIndexCnt, 0, 0);
	}
}