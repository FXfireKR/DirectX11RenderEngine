#pragma once
#include "DirectX11Com.h"
#include "CDirectX11Adapter.h"

#include "CRenderManager.h"
#include "CShaderManager.h"
#include "CInputLayerManager.h"
#include "CPipelineManager.h"
#include "CMeshManager.h"
#include "CMaterialManager.h"
#include "CTextureManager.h"
#include "CSamplerManager.h"
#include "CRuntimeAtlas.h"

// RenderWorld는 렌더링 결과 하나를 책임지는 부분이다.
// 즉 다수로 구성될 수 있다...

struct CB_FrameData
{
	XMFLOAT4X4 view;
	XMFLOAT4X4 proj;

	XMFLOAT4 lightDirWs; // wyz = world-space direction to light
	XMFLOAT4 lightColorIntensity; // rgb = light color, a = intensity
	XMFLOAT4 ambientColor; // rgb = abient, a = unused

	XMFLOAT4 skyColor;

	XMFLOAT4X4 lightViewProj;
	XMFLOAT4 shadowParams; // x = bias, y = shadowMinLight, z/w unused
};

class CRenderWorld
{
public:
	CRenderWorld() = default;
	~CRenderWorld();

	void Initialize(HWND hWnd_, int iScreenWidth_, int iScreenHeight_);

	// Render
	void BeginFrame();
	void DrawFrame();
	void EndFrame();

	// Build render item
	void BeginBuildFrame();
	void Submit(const RenderItem& renderItem);
	void EndBuildFrame();

public:
	inline void SetBackColor(float r, float g, float b, float a) {
		m_fBackColor[0] = r;
		m_fBackColor[1] = g;
		m_fBackColor[2] = b;
		m_fBackColor[3] = a;
	}

	inline void SetViewMatrix(XMMATRIX view) { m_matView = view; }
	inline void SetProjectionMatrix(XMMATRIX proj) { m_matProjection = proj; }

	inline void SetUIViewMatrix(XMMATRIX view) { m_matUIView = view; }
	inline void SetUIProjectionMatrix(XMMATRIX proj) { m_matUIProjection = proj; }

	inline void SetDirectionalLight(const XMFLOAT3& dirWs, const XMFLOAT3& color, float intensity) { m_vLightDirWs = { dirWs.x, dirWs.y, dirWs.z, 0.0f }; m_vLightColorIntensity = { color.x, color.y, color.z, intensity }; }
	inline void SetAmbientLight(const XMFLOAT3& ambient) { m_vAmbientColor = { ambient.x, ambient.y, ambient.z, 0.0f }; }
	inline void SetSkyColor(const XMFLOAT3& skyColor) { m_vSkyColor = { skyColor.x, skyColor.y, skyColor.z, 1.0f }; }
	inline void SetLightViewProj(XMMATRIX lightViewProj) { m_matLightViewProj = lightViewProj; }
	inline void SetShadowParams(float bias, float shadowMinLight) { m_vShadowParams = { bias, shadowMinLight, 0.0f, 0.0f }; }

	inline bool GetVerticalSync() const { return m_dxAdapter.IsVerticalSync(); }
	inline void SetVerticalSync(bool bEnable) { m_dxAdapter.SetVerticalSync(bEnable); }

	inline ID3D11ShaderResourceView* GetShadowMapSRV() const { return m_pShadowSRV.Get(); }

public:
	inline ID3D11Device* GetDevice() const { return m_dxAdapter.GetDevice(); }
	inline ID3D11DeviceContext* GetContext() const { return m_dxAdapter.GetContext(); }

	inline CShaderManager& GetShaderManager() { return m_shaderManager; }
	inline CInputLayerManager& GetIALayoutManager() { return m_inputLayerManager; }
	inline CPipelineManager& GetPipelineManager() { return m_pipelineManager; }
	inline CMeshManager& GetMeshManager() { return m_meshManager; }
	inline CMaterialManager& GetMaterialManager() { return m_materialManager; }
	inline CTextureManager& GetTextureManager() { return m_textureManager; }
	inline CSamplerManager& GetSamplerManager() { return m_samplerManager; }
	inline CRuntimeAtlas& GetRuntimeAtlas() { return m_runtimeAtlas; }

private:
	void _CreateConstFrameBuffer();
	void _CreateConstObjectBuffer();
	void _CreateRenderTargetView();
	void _CreateDepthStencilView();
	void _CreateShadowResources();
	void _UnloadFrameConstants(XMMATRIX view, XMMATRIX proj);

private:
	CDirectX11Adapter m_dxAdapter;

	CRenderManager m_renderManager;

	CShaderManager		m_shaderManager;
	CInputLayerManager	m_inputLayerManager;
	CPipelineManager	m_pipelineManager;
	CMeshManager		m_meshManager;
	CMaterialManager	m_materialManager;
	CTextureManager		m_textureManager;
	CSamplerManager		m_samplerManager;

	CRuntimeAtlas		m_runtimeAtlas;

private:
	ComPtr<ID3D11Texture2D> m_pBackBuffer = nullptr;
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView = nullptr;

	ComPtr<ID3D11Texture2D> m_pDepthBuffer = nullptr;
	ComPtr<ID3D11DepthStencilView> m_pDepthStencilView = nullptr;

	ComPtr<ID3D11Texture2D> m_pShadowTexture = nullptr;
	ComPtr<ID3D11DepthStencilView> m_pShadowDSV = nullptr;
	ComPtr<ID3D11ShaderResourceView> m_pShadowSRV = nullptr;

	XMMATRIX m_matView = XMMatrixIdentity();
	XMMATRIX m_matProjection = XMMatrixIdentity();

	XMMATRIX m_matUIView = XMMatrixIdentity();
	XMMATRIX m_matUIProjection = XMMatrixIdentity();

	ComPtr<ID3D11Buffer> m_pCBFrame; // b0
	ComPtr<ID3D11Buffer> m_pCBObject; // b1

	XMFLOAT4 m_vLightDirWs = { 0.0f, 1.0f, 0.0f, 0.0f };
	XMFLOAT4 m_vLightColorIntensity = { 1.0f, 0.98f, 0.92f, 1.0f };
	XMFLOAT4 m_vAmbientColor = { 0.25f, 0.27f, 0.30f, 0.0f };
	XMFLOAT4 m_vSkyColor = { 0.0f, 0.0f, 0.0f, 0.0f };

	D3D11_VIEWPORT m_kShadowViewport{};
	XMMATRIX m_matLightViewProj = XMMatrixIdentity();
	XMFLOAT4 m_vShadowParams = { 0.0008f, 0.35f, 0.0f, 0.0f };

	UINT m_uShadowMapSize = 2048;

	ID3D11Device* m_pDevice = nullptr; // not-own
	ID3D11DeviceContext* m_pContext = nullptr; // not-own
	IDXGISwapChain* m_pSwapChain = nullptr; // not-own

	D3D11_VIEWPORT m_kViewPort;
	UINT m_uWidth = 0;
	UINT m_uHeight = 0;

	float m_fBackColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

private:
	const size_t MAX_FRAME_WAIT_QUEUE = 3;
};