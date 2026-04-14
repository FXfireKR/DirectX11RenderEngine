#include "pch.h"
#include "CTextureManager.h"

void CTextureManager::Initialize(ID3D11Device& refDevice, ID3D11DeviceContext& refContext)
{
    m_pDevice = &refDevice;
    m_pContext = &refContext;
}

uint64_t CTextureManager::LoadTexture2D(uint64_t id, const char* path, TEXTURE_USAGE eUsage)
{
    auto iter = m_mapping.find(id);
    if (iter != m_mapping.end()) return iter->second;

    auto tex = make_unique<CTexture2D>();
    if (false == tex->LoadFromFile(m_pDevice, m_pContext, path, eUsage)) return UINT64_MAX;

    uint64_t newID = static_cast<uint64_t>(m_vecTextures.size());
    m_vecTextures.push_back(std::move(tex));
    m_mapping[id] = newID;

    return newID;
}

uint64_t CTextureManager::CreateRenderTexture(uint32_t width, uint32_t height, DXGI_FORMAT eFormat, TEXTURE_USAGE eUsage)
{
    auto tex = make_unique<CRenderTexture>();
    if (false == tex->Create(m_pDevice, width, height, eFormat, eUsage)) return UINT64_MAX;

    uint64_t newID = static_cast<uint64_t>(m_vecTextures.size());
    m_vecTextures.push_back(std::move(tex));
    return newID;
}

uint64_t CTextureManager::CreateDepthTexture(uint32_t width, uint32_t height)
{
    auto tex = make_unique<CDepthTexture>();
    if (false == tex->Create(m_pDevice, width, height)) return UINT64_MAX;

    uint64_t newID = static_cast<uint64_t>(m_vecTextures.size());
    m_vecTextures.push_back(std::move(tex));
    return newID;
}

CTexture* CTextureManager::GetTexture(uint64_t id)
{
    if (id < m_vecTextures.size()) {
        return m_vecTextures[id].get();
    }
    return nullptr;
}

const CTexture* CTextureManager::GetTexture(uint64_t id) const
{
    if (id < m_vecTextures.size()) {
        return m_vecTextures[id].get();
    }
    return nullptr;
}
