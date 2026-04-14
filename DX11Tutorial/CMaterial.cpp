#include "pch.h"
#include "CMaterial.h"

CMaterial::CMaterial()
{
	m_arrayTexture.fill(nullptr);
	m_arraySamplerState.fill(nullptr);
}

void CMaterial::SetTexture(uint32_t slot, ID3D11ShaderResourceView* shaderReousrceView)
{
	assert(slot < MAX_TEXTURE_SLOT);
	if (slot >= MAX_TEXTURE_SLOT) 
		return;

	m_arrayTexture[slot] = shaderReousrceView;
}

void CMaterial::SetSampler(uint32_t slot, ID3D11SamplerState* sampler)
{
	assert(slot < MAX_SAMPLER_SLOT);
	if (slot >= MAX_SAMPLER_SLOT)
		return;

	m_arraySamplerState[slot] = sampler;
}

void CMaterial::Bind(ID3D11DeviceContext* pContext) const
{
	assert(pContext != nullptr);
    if (pContext == nullptr) return;

    ID3D11ShaderResourceView* textures[MAX_TEXTURE_SLOT]{};
    ID3D11SamplerState* samplers[MAX_SAMPLER_SLOT]{};

    for (uint32_t i = 0; i < MAX_TEXTURE_SLOT; ++i)
        textures[i] = m_arrayTexture[i].Get();

    for (uint32_t i = 0; i < MAX_SAMPLER_SLOT; ++i)
        samplers[i] = m_arraySamplerState[i].Get();

    pContext->PSSetShaderResources(0, MAX_TEXTURE_SLOT, textures);
    pContext->PSSetSamplers(0, MAX_SAMPLER_SLOT, samplers);

	//pContext->PSSetShaderResources(0, MAX_TEXTURE_SLOT, m_arrayTexture.data()->GetAddressOf());
	//pContext->PSSetSamplers(0, MAX_SAMPLER_SLOT, m_arraySamplerState.data()->GetAddressOf());
}

