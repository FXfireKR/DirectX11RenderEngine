#pragma once
#include "VertexLayoutTypes.h"

class CInputLayer
{
public:
	CInputLayer(const CInputLayer&) = delete;
	CInputLayer& operator=(const CInputLayer&) = delete;

	CInputLayer() = default;
	~CInputLayer() = default;

	HRESULT Create(ID3D11Device* const pDevice_, const VertexLayoutDesc& layoutDesc_, ID3DBlob* pVertexBlob_);
	HRESULT Create(ID3D11Device* const pDevice_, const VertexLayoutDesc& layoutDesc_, const void* pShaderByteCode_, size_t pShaderBufferSize_);

public:
	inline ID3D11InputLayout* GetInputLayout() const { return m_pInputLayout.Get(); }

private:
	ComPtr<ID3D11InputLayout> m_pInputLayout = nullptr;
};