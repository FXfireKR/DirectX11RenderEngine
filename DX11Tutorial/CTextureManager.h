#pragma once
#include "CTexture.h"

class CTextureManager
{
public:
	CTextureManager() = default;
	~CTextureManager() = default;

	void Initialize(ID3D11Device& refDevice, ID3D11DeviceContext& refContext);

	uint64_t LoadTexture2D(uint64_t id, const char* path, TEXTURE_USAGE eUsage);
	uint64_t CreateRenderTexture(uint32_t width, uint32_t height, DXGI_FORMAT eFormat, TEXTURE_USAGE eUsage);
	uint64_t CreateDepthTexture(uint32_t width, uint32_t height);

	CTexture* GetTexture(uint64_t id);
	const CTexture* GetTexture(uint64_t id) const;

private:
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pContext = nullptr;

	vector<unique_ptr<CTexture>> m_vecTextures;
	unordered_map<uint64_t, uint64_t> m_mapping;
};

