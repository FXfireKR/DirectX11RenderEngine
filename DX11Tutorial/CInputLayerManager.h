#pragma once
#include "CInputLayer.h"
#include "ShaderTypes.h"

struct InputLayoutData
{
    VertexLayoutDesc layoutDesc;
    ShaderKey shaderKey;
    unique_ptr<CInputLayer> data = nullptr;

    bool operator==(const InputLayoutData& rhs) const {
        return layoutDesc == rhs.layoutDesc
            && shaderKey == rhs.shaderKey;
    }
};

class CInputLayerManager
{
public:
	CInputLayerManager() = default;
	~CInputLayerManager() = default;

    void Initialize(ID3D11Device& refDevice_);
	uint32_t Create(const VertexLayoutDesc& layoutDesc_, const ShaderKey& shaderKey_, ID3DBlob* pVertexBlob);
    CInputLayer* Get(uint32_t id_);
	const CInputLayer* Get(uint32_t id_) const;

private:
	vector<InputLayoutData> m_vecInputLayoutData;
    ID3D11Device* m_pDevice = nullptr;
};