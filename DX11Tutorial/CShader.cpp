#include "pch.h"
#include "CShader.h"

CShader::CShader(SHADER_DESC shaderDesc_,  vector<D3D_SHADER_MACRO>&& vecMacros_)
	: m_shaderDesc(std::move(shaderDesc_))
	, m_vecShaderMacro(std::move(vecMacros_)) 
{
	// null-terminated check
	if (true == m_vecShaderMacro.empty() || nullptr != m_vecShaderMacro.back().Name) {
		m_vecShaderMacro.push_back({ nullptr, nullptr });
	}

	m_eCompileState = SHADER_COMPILE_STATE::NOT_READY;
}

CShader::~CShader() {}

void CShader::Apply(ID3D11DeviceContext* pContext_) const
{
	if (nullptr != m_pVertexShader) {
		pContext_->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
	}

	if (nullptr != m_pPixelShader) {
		pContext_->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
	}
}

HRESULT CShader::Compile(ID3D11Device* const pDevice_)
{
	if (nullptr == pDevice_) {
		assert(false && "Device is null");
		return E_FAIL;
	}

	// duplicate compile protection
	if (m_eCompileState != SHADER_COMPILE_STATE::NOT_READY) return S_OK;
	m_eCompileState = SHADER_COMPILE_STATE::COMPILING;

	HRESULT hr = S_OK;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	if (false == m_shaderDesc.strVertexShaderPath.empty()) 
	{
		wstring wstrPath(m_shaderDesc.strVertexShaderPath.begin(), m_shaderDesc.strVertexShaderPath.end());
		errorBlob.Reset();

		hr = ::D3DCompileFromFile(
			wstrPath.c_str(),
			m_vecShaderMacro.data(),
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			m_shaderDesc.strVertexShaderEntry.c_str(),
			m_shaderDesc.strVertexShaderVersion.c_str(),
#ifdef _DEBUG
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
#else // _DEBUG
			0,
#endif // _DEBUG
			0,
			m_pVertexBlob.GetAddressOf(),
			errorBlob.GetAddressOf()
		);

		if (FAILED(hr)) {
			if (nullptr != errorBlob) {
#ifdef DEBUG_LOG
				cout << (char*)errorBlob->GetBufferPointer() << endl;
#endif // DEBUG_LOG
			}
			m_eCompileState = SHADER_COMPILE_STATE::FAIL;
			return hr;
		}

		// Create Vertex Blob
		hr = pDevice_->CreateVertexShader(
			m_pVertexBlob->GetBufferPointer(),
			m_pVertexBlob->GetBufferSize(),
			nullptr,
			m_pVertexShader.GetAddressOf()
		);

		if (FAILED(hr)) {
			return hr;
		}
	}


	if (false == m_shaderDesc.strPixelShaderPath.empty())
	{
		wstring wstrPath(m_shaderDesc.strPixelShaderPath.begin(), m_shaderDesc.strPixelShaderPath.end());
		errorBlob.Reset();

		hr = ::D3DCompileFromFile(
			wstrPath.c_str(),
			m_vecShaderMacro.data(),
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			m_shaderDesc.strPixelShaderEntry.c_str(),
			m_shaderDesc.strPixelShaderVersion.c_str(),
#ifdef _DEBUG
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
#else // _DEBUG
			0,
#endif // _DEBUG
			0,
			m_pPixelBlob.GetAddressOf(),
			errorBlob.GetAddressOf()
		);

		if (FAILED(hr)) {
			if (nullptr != errorBlob) {
#ifdef DEBUG_LOG
				cout << (char*)errorBlob->GetBufferPointer() << endl;
#endif // DEBUG_LOG
			}
			m_eCompileState = SHADER_COMPILE_STATE::FAIL;
			return hr;
		}

		// Create Pixel Blob
		hr = pDevice_->CreatePixelShader(
			m_pPixelBlob->GetBufferPointer(),
			m_pPixelBlob->GetBufferSize(),
			nullptr,
			m_pPixelShader.GetAddressOf()
		);

		if (FAILED(hr)) {
			return hr;
		}
	}

	m_eCompileState = SHADER_COMPILE_STATE::READY;
	return hr;
}

void CShader::ReleaseBlobs()
{
	m_pVertexBlob.Reset();
	m_pPixelBlob.Reset();
}