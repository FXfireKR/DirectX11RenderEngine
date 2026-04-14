#pragma once

class DirectX11Com
{
public:
	DirectX11Com();
	~DirectX11Com();

	HRESULT Initialize(HWND hWnd_, int iScreenWidth_, int iScreenHeight_
        , bool bFullScreen_, float fScreenDepth_, float fScreenNear_);

	void BeginRender();
	void EndRender();
	void SetBackBufferRenderTarget();
	void ResetViewPort();

public:
	void ImGuiTick();
	void ImGuiRender();

private:
	HRESULT _ImGuiInit(HWND hWnd_);
	void _ImGuiRelease();

public:
	inline ID3D11Device* GetDevice() const { return m_pDevice.Get(); }
	inline ID3D11Device& GetDeviceRef() { return *(m_pDevice.Get()); }

	inline ID3D11DeviceContext* GetContext() const { return m_pDeviceContext.Get(); }
	inline ID3D11DeviceContext& GetContextRef() { return *(m_pDeviceContext.Get()); }

	inline IDXGISwapChain* GetSwapChain() const { return m_pSwapChain.Get(); }

	inline const bool GetVerticalSync() const { return m_bVerticalSync; }

	inline void GetProjectionMatrix(XMMATRIX& matWorld_) { matWorld_ = m_matProjection; }
	inline void GetWorldMatrix(XMMATRIX& matWorld_) { matWorld_ = m_matWorld; }
	inline void GetOrthoMatrix(XMMATRIX& matWorld_) { matWorld_ = m_matOrtho; }

private:
	bool m_bVerticalSync;
	double m_dVideoCardMemory;
	UINT m_uDXGIDisplayWidth;
	UINT m_uDXGIDisplayHeight;
	DXGI_RATIONAL m_uDXGIDisplayRational;
	
	ComPtr<ID3D11Device> m_pDevice = nullptr;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext = nullptr;
	ComPtr<IDXGISwapChain> m_pSwapChain = nullptr;

    ID3D11DepthStencilState* m_pDepthStencilState;
    ID3D11RasterizerState* m_pRasterState;

    D3D11_VIEWPORT m_kViewPort;
    XMMATRIX m_matProjection;
    XMMATRIX m_matWorld;
    XMMATRIX m_matOrtho;    // Orthographic - 직교적인/정렬된
};