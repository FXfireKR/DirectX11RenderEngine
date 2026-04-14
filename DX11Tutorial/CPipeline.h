#pragma once
#include "CShader.h"
#include "CInputLayer.h"

class CPipeline
{
public:
	CPipeline() = default;
	~CPipeline() = default;

	void Initialize();

	void CreateOpaqueState(ID3D11Device* pDevice);
	void CreateCutoutAlphaTestState(ID3D11Device* pDevice, bool bCullNone = true);
	void CreateSkyAlphaState(ID3D11Device* pDevice, bool bCullNone = true);
	void CreateTransparentAlphaState(ID3D11Device* pDevice, bool bCullNone = true);
	void CreateAdditiveState(ID3D11Device* pDevice, bool bCullNone = true);
	void CreateUIInvertState(ID3D11Device* pDevice, bool bCullNone = true);

	void SetShader(CShader* const pShader_);
	void SetInputLayout(CInputLayer* const pInputLayout_);

	void Bind(ID3D11DeviceContext* pDeviceContext_) const;

public:
	inline const ID3D11RasterizerState* GetRasterizer() const { return m_pRasterizerState.Get(); }
	inline const ID3D11DepthStencilState* GetDepthStencil() const { return m_pDepthStencilState.Get(); }
	inline const ID3D11BlendState* GetBlendState() const { return m_pBlendState.Get(); }

	inline void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY ePrimitiveTopology) { m_ePrimitiveTopology = ePrimitiveTopology; }
	inline const D3D11_PRIMITIVE_TOPOLOGY& GetPrimitiveTopology() const { return m_ePrimitiveTopology; }

private:
	ID3D11VertexShader* m_pVertexShader = nullptr;
	ID3D11PixelShader* m_pPixelShader = nullptr;
	ID3D11InputLayout* m_pInputLayout = nullptr;

	D3D11_PRIMITIVE_TOPOLOGY m_ePrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// Render states
	ComPtr<ID3D11RasterizerState> m_pRasterizerState;
	ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;
	ComPtr<ID3D11BlendState> m_pBlendState;
};