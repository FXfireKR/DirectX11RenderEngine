#pragma once

class CDirectX11Adapter
{
public:
	CDirectX11Adapter(const CDirectX11Adapter&) = delete;
	CDirectX11Adapter operator= (const CDirectX11Adapter&) = delete;

public:
	CDirectX11Adapter();
	~CDirectX11Adapter();

	HRESULT Initialize(HWND hWnd_);

private:
	HRESULT _CreateDeviceAndContext();
	HRESULT _BindDXGI();
	HRESULT _CreateSwapChain();
	HRESULT _FetchDistplayModeList();

public:
	inline ID3D11Device* GetDevice() const { return m_pDevice.Get(); }
	inline ID3D11Device& GetDeviceRef() const { return *(m_pDevice.Get()); }

	inline ID3D11DeviceContext* GetContext() const { return m_pContext.Get(); }
	inline ID3D11DeviceContext& GetContextRef() const { return *(m_pContext.Get()); }

	inline IDXGISwapChain* GetSwapChain() const { return m_pSwapChain.Get(); }

	inline void SetVerticalSync(bool bEnable) { m_bVerticalSync = bEnable; }
	inline const bool IsVerticalSync() const { return m_bVerticalSync; }

private:
	// DX
	ComPtr<ID3D11Device> m_pDevice = nullptr;
	ComPtr<ID3D11DeviceContext> m_pContext = nullptr;
	ComPtr<IDXGISwapChain> m_pSwapChain = nullptr;

	// DXGI
	ComPtr<IDXGIAdapter> m_pDXGIAdapter = nullptr;
	ComPtr<IDXGIFactory> m_pDXGIFactory = nullptr;
	ComPtr<IDXGIOutput> m_pDXGIOutput = nullptr;

	vector<DXGI_MODE_DESC> m_vecDXGImodes;
	
	HWND m_hWnd = nullptr;
	double m_dVideoCardMemory;
	bool m_bFullScreen = false;
	bool m_bVerticalSync = true;
	DXGI_RATIONAL m_kDXGIRational;
	D3D_FEATURE_LEVEL m_eFeatureLevel = D3D_FEATURE_LEVEL_11_0;
};
