#pragma once
class CMaterial
{
public:
	CMaterial();
	~CMaterial() = default;

	void SetTexture(uint32_t slot, ID3D11ShaderResourceView* shaderReousrceView);
	void SetSampler(uint32_t slot, ID3D11SamplerState* sampler);

	void Bind(ID3D11DeviceContext* pContext) const;

private:
	static constexpr uint32_t MAX_TEXTURE_SLOT = 8;
	static constexpr uint32_t MAX_SAMPLER_SLOT = 4;

	array<ComPtr<ID3D11ShaderResourceView>, MAX_TEXTURE_SLOT> m_arrayTexture;
	array<ComPtr<ID3D11SamplerState>, MAX_SAMPLER_SLOT> m_arraySamplerState;
};