#include "pch.h"
#include "CSamplerState.h"

bool CSamplerState::Create(ID3D11Device* pDevice, SAMPLER_TYPE eSamplerType)
{
	D3D11_SAMPLER_DESC desc{};
	_GetSamplerDesc(eSamplerType, desc);
	
	if (FAILED(pDevice->CreateSamplerState(&desc, m_pSamplerState.GetAddressOf())))
		return false;
	return true;
}

void CSamplerState::_GetSamplerDesc(SAMPLER_TYPE eSamplerType, D3D11_SAMPLER_DESC& desc)
{
	ZeroMemory(&desc, sizeof(D3D11_SAMPLER_DESC));

	desc.MipLODBias = 0.f;
	desc.MinLOD = 0.f;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	switch (eSamplerType)
	{
		case SAMPLER_TYPE::LINEAR_WARP:
		{
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		} break;

		case SAMPLER_TYPE::LINEAR_CLAMP:
		{
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		} break;

		case SAMPLER_TYPE::POINT_WRAP:
		{
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		} break;

		case SAMPLER_TYPE::POINT_CLAMP:
		{
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		} break;

		case SAMPLER_TYPE::ANISOTROPIC_WARP:
		{
			desc.Filter = D3D11_FILTER_ANISOTROPIC;
			desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.MaxAnisotropy = 16;
		} break;

		case SAMPLER_TYPE::SHADOWCOMPARISON:
		{
			desc.Filter = D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
			desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
			desc.BorderColor[0] = 1.f;
			desc.BorderColor[1] = 1.f;
			desc.BorderColor[2] = 1.f;
		} break;
	}
}