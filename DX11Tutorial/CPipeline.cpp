#include "pch.h"
#include "CPipeline.h"

void CPipeline::Initialize()
{

}

void CPipeline::CreateOpaqueState(ID3D11Device* pDevice)
{
	D3D11_RASTERIZER_DESC rs = {};
	rs.FillMode = D3D11_FILL_SOLID;
	rs.CullMode = D3D11_CULL_BACK;
	rs.FrontCounterClockwise = FALSE;
	pDevice->CreateRasterizerState(&rs, m_pRasterizerState.GetAddressOf());

	D3D11_DEPTH_STENCIL_DESC dsd{};
	dsd.DepthEnable = true;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = D3D11_COMPARISON_LESS; // 일반적인 Z Test
	dsd.StencilEnable = false; // 스텐실은 필요 없으면 끔
	pDevice->CreateDepthStencilState(&dsd, m_pDepthStencilState.GetAddressOf());

	D3D11_BLEND_DESC bd{};
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;

	auto& rt = bd.RenderTarget[0];
	rt.BlendEnable = false;
	rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	pDevice->CreateBlendState(&bd, m_pBlendState.GetAddressOf());
}

void CPipeline::CreateCutoutAlphaTestState(ID3D11Device* pDevice, bool bCullNone)
{
	D3D11_RASTERIZER_DESC rs{};
	rs.FillMode = D3D11_FILL_SOLID;
	rs.CullMode = bCullNone ? D3D11_CULL_NONE : D3D11_CULL_BACK;
	rs.FrontCounterClockwise = FALSE;
	pDevice->CreateRasterizerState(&rs, m_pRasterizerState.GetAddressOf());

	D3D11_DEPTH_STENCIL_DESC dsd{};
	dsd.DepthEnable = true;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = D3D11_COMPARISON_LESS;
	dsd.StencilEnable = false;
	pDevice->CreateDepthStencilState(&dsd, m_pDepthStencilState.GetAddressOf());

	D3D11_BLEND_DESC bd{};
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;

	auto& rt = bd.RenderTarget[0];
	rt.BlendEnable = false;
	rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	pDevice->CreateBlendState(&bd, m_pBlendState.GetAddressOf());
}

void CPipeline::CreateSkyAlphaState(ID3D11Device* pDevice, bool bCullNone)
{
	D3D11_RASTERIZER_DESC rs{};
	rs.FillMode = D3D11_FILL_SOLID;
	rs.CullMode = bCullNone ? D3D11_CULL_NONE : D3D11_CULL_BACK;
	rs.FrontCounterClockwise = FALSE;
	pDevice->CreateRasterizerState(&rs, m_pRasterizerState.GetAddressOf());

	D3D11_DEPTH_STENCIL_DESC dsd{};
	dsd.DepthEnable = false;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsd.DepthFunc = D3D11_COMPARISON_ALWAYS;
	dsd.StencilEnable = false;
	pDevice->CreateDepthStencilState(&dsd, m_pDepthStencilState.GetAddressOf());

	D3D11_BLEND_DESC bd{};
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;

	auto& rt = bd.RenderTarget[0];
	rt.BlendEnable = true;
	rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rt.BlendOp = D3D11_BLEND_OP_ADD;
	rt.SrcBlendAlpha = D3D11_BLEND_ONE;
	rt.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	pDevice->CreateBlendState(&bd, m_pBlendState.GetAddressOf());
}

void CPipeline::CreateTransparentAlphaState(ID3D11Device* pDevice, bool bCullNone)
{
	D3D11_RASTERIZER_DESC rs = {};
	rs.FillMode = D3D11_FILL_SOLID;
	rs.CullMode = bCullNone ? D3D11_CULL_NONE : D3D11_CULL_BACK;
	rs.FrontCounterClockwise = FALSE;
	pDevice->CreateRasterizerState(&rs, m_pRasterizerState.GetAddressOf());

	D3D11_DEPTH_STENCIL_DESC dsd{};
	dsd.DepthEnable = true;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsd.DepthFunc = D3D11_COMPARISON_LESS; // 일반적인 Z Test
	dsd.StencilEnable = false; // 스텐실은 필요 없으면 끔
	pDevice->CreateDepthStencilState(&dsd, m_pDepthStencilState.GetAddressOf());

	D3D11_BLEND_DESC bd{};
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;

	auto& rt = bd.RenderTarget[0];
	rt.BlendEnable = true;
	rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rt.BlendOp = D3D11_BLEND_OP_ADD;

	rt.SrcBlendAlpha = D3D11_BLEND_ONE;
	rt.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;

	rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	pDevice->CreateBlendState(&bd, m_pBlendState.GetAddressOf());
}

void CPipeline::CreateAdditiveState(ID3D11Device* pDevice, bool bCullNone)
{
	D3D11_RASTERIZER_DESC rs = {};
	rs.FillMode = D3D11_FILL_SOLID;
	rs.CullMode = bCullNone ? D3D11_CULL_NONE : D3D11_CULL_BACK;
	rs.FrontCounterClockwise = FALSE;
	pDevice->CreateRasterizerState(&rs, m_pRasterizerState.GetAddressOf());

	D3D11_DEPTH_STENCIL_DESC dsd{};
	dsd.DepthEnable = true;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsd.DepthFunc = D3D11_COMPARISON_LESS; // 일반적인 Z Test
	dsd.StencilEnable = false; // 스텐실은 필요 없으면 끔
	pDevice->CreateDepthStencilState(&dsd, m_pDepthStencilState.GetAddressOf());

	D3D11_BLEND_DESC bd{};
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;

	auto& rt = bd.RenderTarget[0];
	rt.BlendEnable = true;
	rt.SrcBlend = D3D11_BLEND_ONE;
	rt.DestBlend = D3D11_BLEND_ONE;
	rt.BlendOp = D3D11_BLEND_OP_ADD;

	rt.SrcBlendAlpha = D3D11_BLEND_ONE;
	rt.DestBlendAlpha = D3D11_BLEND_ONE;
	rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;

	rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	pDevice->CreateBlendState(&bd, m_pBlendState.GetAddressOf());
}

void CPipeline::CreateUIInvertState(ID3D11Device* pDevice, bool bCullNone)
{
	D3D11_RASTERIZER_DESC rs{};
	rs.FillMode = D3D11_FILL_SOLID;
	rs.CullMode = bCullNone ? D3D11_CULL_NONE : D3D11_CULL_BACK;
	rs.FrontCounterClockwise = FALSE;
	pDevice->CreateRasterizerState(&rs, m_pRasterizerState.GetAddressOf());

	// UI는 depth 영향 받지 않게
	D3D11_DEPTH_STENCIL_DESC dsd{};
	dsd.DepthEnable = false;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsd.DepthFunc = D3D11_COMPARISON_ALWAYS;
	dsd.StencilEnable = false;
	pDevice->CreateDepthStencilState(&dsd, m_pDepthStencilState.GetAddressOf());

	D3D11_BLEND_DESC bd{};
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;

	auto& rt = bd.RenderTarget[0];
	rt.BlendEnable = true;

	// out = src*(1-dest) + dest*(1-src)
	rt.SrcBlend = D3D11_BLEND_INV_DEST_COLOR;
	rt.DestBlend = D3D11_BLEND_INV_SRC_COLOR;
	rt.BlendOp = D3D11_BLEND_OP_ADD;

	// alpha는 크게 의미 없으니 단순 처리
	rt.SrcBlendAlpha = D3D11_BLEND_ONE;
	rt.DestBlendAlpha = D3D11_BLEND_ZERO;
	rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;

	rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	pDevice->CreateBlendState(&bd, m_pBlendState.GetAddressOf());
}

void CPipeline::SetShader(CShader* const pShader_)
{
	if (nullptr == pShader_) 
	{
		assert(false && "Shader nullptr");
		return;
	}

	// vertex shader 
	m_pVertexShader = pShader_->GetVertexShader();

	// pixel shader
	m_pPixelShader = pShader_->GetPixelShader();
}

void CPipeline::SetInputLayout(CInputLayer* const pInputLayout_)
{
	if (nullptr == pInputLayout_) 
	{
		assert(false && "InputLayout nullptr");
		return;
	}

	// ia layout
	m_pInputLayout = pInputLayout_->GetInputLayout();
}

void CPipeline::Bind(ID3D11DeviceContext* pDeviceContext_) const
{
	pDeviceContext_->IASetInputLayout(m_pInputLayout);
	pDeviceContext_->IASetPrimitiveTopology(m_ePrimitiveTopology);

	pDeviceContext_->VSSetShader(m_pVertexShader, nullptr, 0);
	pDeviceContext_->PSSetShader(m_pPixelShader, nullptr, 0);

	pDeviceContext_->RSSetState(m_pRasterizerState.Get());
	pDeviceContext_->OMSetDepthStencilState(m_pDepthStencilState.Get(), 0);

	float blendFactor[4] = { 1.f, 1.f, 1.f ,1.f };
	pDeviceContext_->OMSetBlendState(m_pBlendState.Get(), blendFactor, 0xffffffff);
}
