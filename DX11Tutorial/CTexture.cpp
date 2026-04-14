#include "pch.h"
#include "CTexture.h"

TextureCreateInfo CTexture::_GetCreateInfo(TEXTURE_USAGE eUsage_)
{
	switch (eUsage_)
	{
		case TEXTURE_USAGE::StaticColor:
			return { 
				D3D11_USAGE_DEFAULT, 
				D3D11_BIND_SHADER_RESOURCE, 
				0,
				false,
				true
			};

		case TEXTURE_USAGE::StaticData:
			return { 
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_SHADER_RESOURCE, 
				0, 
				false,
				false
			};

		case TEXTURE_USAGE::StaticColorMip:
			return { 
				D3D11_USAGE_DEFAULT, 
				D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 
				D3D11_RESOURCE_MISC_GENERATE_MIPS, 
				true,
				true 
			};

		case TEXTURE_USAGE::RenderTarget:
			return {
				D3D11_USAGE_DEFAULT, 
				D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
				0, 
				false,
				false
			};

		case TEXTURE_USAGE::Depth:
			return { 
				D3D11_USAGE_DEFAULT, 
				D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, 
				0, 
				false,
				false
			};

		case TEXTURE_USAGE::CubeMapColor:
		case TEXTURE_USAGE::CubeMapData:
		case TEXTURE_USAGE::CubeMapRender:
			return { 
				D3D11_USAGE_DEFAULT, 
				D3D11_BIND_SHADER_RESOURCE, 
				D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS, 
				true,
				true
			};
	}
	return {};
}

bool CTexture2D::LoadFromFile(ID3D11Device* const pDevice_, ID3D11DeviceContext* const pContext_, const char* path_, TEXTURE_USAGE eUsage_)
{
	ComPtr<ID3D11Resource> pResource = nullptr;
	TextureCreateInfo info = _GetCreateInfo(eUsage_);
	wstring path = UTF8ToWstring(path_);
	UINT uLoaderFlags = 0;

	// 강제로 RGBA 32 로 만들어줘요...
	uLoaderFlags |= WIC_LOADER_FORCE_RGBA32;

	m_kDesc.bSRGB = info.bSRGB;
	m_kDesc.bMipmap = info.bMipmap;

	_CheckTextureSource(path_);

	HRESULT hr = S_OK;
	if (TEXTURE_SOURCE::DDS == m_eTextureSource) {

		if (m_kDesc.bSRGB) {
			uLoaderFlags |= DDS_LOADER_FORCE_SRGB;
		}

		hr = CreateDDSTextureFromFileEx(
			pDevice_,
			pContext_,
			path.c_str(),
			0,
			info.usage,
			info.bindFlags,
			0,
			info.miscFlags,
			static_cast<DDS_LOADER_FLAGS>(uLoaderFlags),
			pResource.GetAddressOf(),
			m_pShaderResourceView.GetAddressOf()
		);
	}
	else {

		if (m_kDesc.bSRGB) {
			uLoaderFlags |= WIC_LOADER_FORCE_SRGB;
		}

		hr = CreateWICTextureFromFileEx(
			pDevice_,
			pContext_,
			path.c_str(),
			0,
			info.usage,
			info.bindFlags,
			0,
			info.miscFlags,
			static_cast<WIC_LOADER_FLAGS>(uLoaderFlags),
			pResource.GetAddressOf(),
			m_pShaderResourceView.GetAddressOf()
		);
	}	

	if (FAILED(hr)) return false;
	if (nullptr == m_pShaderResourceView) return false;

	if (m_kDesc.bMipmap) {
		pContext_->GenerateMips(m_pShaderResourceView.Get());
	}

	hr = pResource.As(&m_pTexture);
	if (FAILED(hr)) return false;
	if (nullptr == m_pTexture) return false;

	D3D11_TEXTURE2D_DESC desc;
	m_pTexture->GetDesc(&desc);
	
	m_kDesc.format = desc.Format;
	m_kDesc.width = desc.Width;
	m_kDesc.height = desc.Height;

    return true;
}

void CTexture2D::_CheckTextureSource(const char* path_)
{
	filesystem::path texturePath(path_);
	if (true == texturePath.has_extension()) {
		string ext(texturePath.extension().string());
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		
		if (ext == ".dds") {
			m_eTextureSource = TEXTURE_SOURCE::DDS;
		}
		else {
			m_eTextureSource = TEXTURE_SOURCE::WIC;
		}
	}
}

bool CRenderTexture::Create(ID3D11Device* pDevice, uint32_t width, uint32_t height, DXGI_FORMAT eFormat, TEXTURE_USAGE eUsage)
{
	m_eUsage = eUsage;

	TextureCreateInfo info = _GetCreateInfo(eUsage);

	m_kDesc.width = width;
	m_kDesc.height = height;
	m_kDesc.format = eFormat;
	m_kDesc.bMipmap = info.bMipmap;
	m_kDesc.bSRGB = info.bSRGB;

	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = info.bMipmap ? 0 : 1;
	texDesc.ArraySize = 1;
	texDesc.Format = eFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = info.usage;
	texDesc.BindFlags = info.bindFlags;
	texDesc.MiscFlags = info.miscFlags;

	HRESULT hr = pDevice->CreateTexture2D(&texDesc, nullptr, m_pTexture.GetAddressOf());
	if (FAILED(hr)) return false;

	// Render Target View
	hr = pDevice->CreateRenderTargetView(m_pTexture.Get(), nullptr, m_pRenderTargetView.GetAddressOf());
	if (FAILED(hr)) return false;

	// Shader Resource View
	if (info.bindFlags & D3D11_BIND_SHADER_RESOURCE)
	{
		hr = pDevice->CreateShaderResourceView(m_pTexture.Get(), nullptr, m_pShaderResourceView.GetAddressOf());
		if (FAILED(hr)) return false;
	}

	return true;
}

bool CDepthTexture::Create(ID3D11Device* pDevice, uint32_t width, uint32_t height)
{
	m_eUsage = TEXTURE_USAGE::Depth;
	TextureCreateInfo info = _GetCreateInfo(m_eUsage);

	m_kDesc.width = width;
	m_kDesc.height = height;
	m_kDesc.format = DXGI_FORMAT_R32_TYPELESS;
	m_kDesc.bMipmap = info.bMipmap;
	m_kDesc.bSRGB = info.bSRGB;

	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = info.bMipmap ? 0 : 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = info.usage;
	texDesc.BindFlags = info.bindFlags;
	texDesc.MiscFlags = info.miscFlags;

	HRESULT hr = pDevice->CreateTexture2D(&texDesc, nullptr, m_pTexture.GetAddressOf());
	if (FAILED(hr)) return false;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	// Depth Stencil View
	hr = pDevice->CreateDepthStencilView(m_pTexture.Get(), &dsvDesc, m_pDepthStencilView.GetAddressOf());
	if (FAILED(hr)) return false;

	// Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(m_pTexture.Get(), &srvDesc, m_pShaderResourceView.GetAddressOf());
	if (FAILED(hr)) return false;

	return true;
}