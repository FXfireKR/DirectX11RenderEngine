#pragma once

enum class TEXTURE_USAGE
{
	StaticColor,		// 알베도 / 컬러 텍스쳐
	StaticData,			// 노멀 / 러프 / 메탈
	StaticColorMip,		// 컬러 + mip
	RenderTarget,		// 후처리 / GBuffer
	Depth,				// 그림자 / 깊이 pre-pass

	CubeMapColor,			// SkyBox
	CubeMapData,
	CubeMapRender,
};

enum class TEXTURE_SOURCE
{
	WIC,
	DDS
};

struct TextureDesc
{
	uint32_t width;
	uint32_t height;
	DXGI_FORMAT format;
	bool bMipmap;
	bool bSRGB;
};

struct TextureCreateInfo
{
	D3D11_USAGE usage;
	UINT bindFlags;
	UINT miscFlags;
	bool bMipmap;
	bool bSRGB;
};

class CTexture
{
protected:
	// block create instance
	CTexture() = default;

public:
	virtual ~CTexture() = default;

protected:
	TextureCreateInfo _GetCreateInfo(TEXTURE_USAGE eUsage_);

public:
	inline ID3D11Texture2D* GetTexture2D() { return m_pTexture.Get(); }
	inline const ID3D11Texture2D* GetTexture2D() const { return m_pTexture.Get(); }

	inline ID3D11ShaderResourceView* GetShaderResourceView() { return m_pShaderResourceView.Get(); }
	inline const ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pShaderResourceView.Get(); }

	inline const TextureDesc& GetDesc() const { return m_kDesc; }
	inline const TEXTURE_USAGE& GetUsage() const { return m_eUsage; }

protected:
	ComPtr<ID3D11Texture2D> m_pTexture;
	ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView;

	TextureDesc m_kDesc;
	TEXTURE_USAGE m_eUsage;
};

//class CTextureCube

class CTexture2D : public CTexture 
{
public:
	bool LoadFromFile(ID3D11Device* const pDevice_, ID3D11DeviceContext* const pContext_
		, const char* path_, TEXTURE_USAGE eUsage_);

private:
	void _CheckTextureSource(const char* path_);

private:
	TEXTURE_SOURCE m_eTextureSource;
};

class CRenderTexture : public CTexture
{
public:
	bool Create(ID3D11Device* pDevice, uint32_t width, uint32_t height, DXGI_FORMAT eFormat, TEXTURE_USAGE eUsage);

public:
	inline ID3D11RenderTargetView* GetRenderTargetView() { return m_pRenderTargetView.Get(); }
	inline const ID3D11RenderTargetView* GetRenderTargetView() const { return m_pRenderTargetView.Get(); }

private:
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
};

class CDepthTexture : public CTexture
{
public:
	bool Create(ID3D11Device* pDevice, uint32_t width, uint32_t height);

public:
	inline ID3D11DepthStencilView* GetDepthStencilView() { return m_pDepthStencilView.Get(); }
	inline const ID3D11DepthStencilView* GetDepthStencilView() const { return m_pDepthStencilView.Get(); }

private:
	ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;
};