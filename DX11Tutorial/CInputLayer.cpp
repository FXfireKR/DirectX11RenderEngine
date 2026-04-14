#include "pch.h"
#include "CInputLayer.h"

HRESULT CInputLayer::Create(ID3D11Device* const pDevice_, const VertexLayoutDesc& layoutDesc_, ID3DBlob* pVertexBlob_)
{
	const uint32_t uElemSize = static_cast<uint32_t>(layoutDesc_.vecElements.size());
	vector<D3D11_INPUT_ELEMENT_DESC> layout(uElemSize);

	for (size_t i = 0; i < layoutDesc_.vecElements.size(); ++i) {
		const auto& elem = layoutDesc_.vecElements[i];

		layout[i].SemanticName = GetVertexSemanticString(elem.eSemantic);
		layout[i].SemanticIndex = elem.uSemanticIndex;
		layout[i].Format = elem.dxgiFormat;
		layout[i].InputSlot = elem.uInputSlot;
		layout[i].AlignedByteOffset = elem.uOffset;
		layout[i].InputSlotClass = elem.eInputSlotClass;
		layout[i].InstanceDataStepRate = elem.uInstanceDataStepRate;
	}

	return pDevice_->CreateInputLayout(layout.data(), uElemSize, pVertexBlob_->GetBufferPointer()
		, pVertexBlob_->GetBufferSize(), m_pInputLayout.GetAddressOf());
}

HRESULT CInputLayer::Create(ID3D11Device* const pDevice_, const VertexLayoutDesc& layoutDesc_, const void* pShaderByteCode_, size_t pShaderBufferSize_)
{
	const uint32_t uElemSize = static_cast<uint32_t>(layoutDesc_.vecElements.size());
	vector<D3D11_INPUT_ELEMENT_DESC> layout(uElemSize);

	for (size_t i = 0; i < layoutDesc_.vecElements.size(); ++i) {
		const auto& elem = layoutDesc_.vecElements[i];

		layout[i].SemanticName = GetVertexSemanticString(elem.eSemantic);
		layout[i].SemanticIndex = elem.uSemanticIndex;
		layout[i].Format = elem.dxgiFormat;
		layout[i].InputSlot = elem.uInputSlot;
		layout[i].AlignedByteOffset = elem.uOffset;
		layout[i].InputSlotClass = elem.eInputSlotClass;
		layout[i].InstanceDataStepRate = elem.uInstanceDataStepRate;
	}

	return pDevice_->CreateInputLayout(layout.data(), uElemSize, pShaderByteCode_
		, pShaderBufferSize_, m_pInputLayout.GetAddressOf());
}