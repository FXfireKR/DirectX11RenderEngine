#pragma once

enum class SAMPLER_TYPE
{
	LINEAR_WARP,
	LINEAR_CLAMP,
	POINT_WRAP,
	POINT_CLAMP,
	ANISOTROPIC_WARP,
	SHADOWCOMPARISON,
	// add here
};

class CSamplerState
{
public:
	CSamplerState() = default;
	~CSamplerState() = default;

	bool Create(ID3D11Device* pDevice, SAMPLER_TYPE eSamplerType);

	inline ID3D11SamplerState* Get() { return m_pSamplerState.Get(); }
	inline const ID3D11SamplerState* Get() const { return m_pSamplerState.Get(); }

private:
	void _GetSamplerDesc(SAMPLER_TYPE eSamplerType, D3D11_SAMPLER_DESC& desc);

private:
	ComPtr<ID3D11SamplerState> m_pSamplerState;
};