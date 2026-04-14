#include "pch.h"
#include "CInputLayerManager.h"

void CInputLayerManager::Initialize(ID3D11Device& refDevice_)
{
    m_pDevice = &refDevice_;
}

uint32_t CInputLayerManager::Create(const VertexLayoutDesc& layoutDesc_, const ShaderKey& shaderKey_, ID3DBlob* pVertexBlob)
{
    // duplicate check
    for (size_t i = 0; i < m_vecInputLayoutData.size(); ++i) {
        const auto& elem = m_vecInputLayoutData[i];
        if (elem.layoutDesc == layoutDesc_ && elem.shaderKey == shaderKey_) {
            return static_cast<uint32_t>(i);
        }
    }

    InputLayoutData newData;
    newData.layoutDesc = layoutDesc_;
    newData.shaderKey = shaderKey_;
    newData.data = make_unique<CInputLayer>();

    newData.data->Create(m_pDevice, layoutDesc_, pVertexBlob->GetBufferPointer(), pVertexBlob->GetBufferSize());

    m_vecInputLayoutData.push_back(std::move(newData));
    return static_cast<uint32_t>(m_vecInputLayoutData.size() - 1);
}

CInputLayer* CInputLayerManager::Get(uint32_t id_)
{
    if (id_ < m_vecInputLayoutData.size()) {
        return m_vecInputLayoutData[id_].data.get();
    }
    return nullptr;
}

const CInputLayer* CInputLayerManager::Get(uint32_t id_) const
{
    if (id_ < m_vecInputLayoutData.size()) {
        return m_vecInputLayoutData[id_].data.get();
    }
    return nullptr;
}