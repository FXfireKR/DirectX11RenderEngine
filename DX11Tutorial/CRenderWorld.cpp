#include "pch.h"
#include "CRenderWorld.h"

CRenderWorld::~CRenderWorld()
{
}

void CRenderWorld::Initialize(HWND hWnd_, int iScreenWidth_, int iScreenHeight_)
{
	m_dxAdapter.Initialize(hWnd_);

	m_pDevice = m_dxAdapter.GetDevice();
	assert(m_pDevice);

	m_pContext = m_dxAdapter.GetContext();
	assert(m_pContext);

	m_pSwapChain = m_dxAdapter.GetSwapChain();
	assert(m_pSwapChain);

	_CreateConstFrameBuffer();
	_CreateConstObjectBuffer();

	m_renderManager.Initialize(MAX_FRAME_WAIT_QUEUE, *(m_pCBObject.Get()));

	m_shaderManager.Initialize(m_dxAdapter.GetDeviceRef());
	m_inputLayerManager.Initialize(m_dxAdapter.GetDeviceRef());
	m_meshManager.Initialize(m_dxAdapter.GetDeviceRef());
	m_textureManager.Initialize(m_dxAdapter.GetDeviceRef(), m_dxAdapter.GetContextRef());
	m_samplerManager.Initialize(m_dxAdapter.GetDeviceRef());

	RECT rc;
	GetClientRect(hWnd_, &rc);
	m_uWidth = rc.right - rc.left;
	m_uHeight = rc.bottom - rc.top;

	_CreateRenderTargetView();
	_CreateDepthStencilView();
	_CreateShadowResources();

	ZeroMemory(&m_kViewPort, sizeof(D3D11_VIEWPORT));
	m_kViewPort.TopLeftX = 0.f;
	m_kViewPort.TopLeftY = 0.f;
	m_kViewPort.Width = static_cast<float>(m_uWidth);
	m_kViewPort.Height = static_cast<float>(m_uHeight);
	m_kViewPort.MinDepth = 0.f;
	m_kViewPort.MaxDepth = 1.f;	
}

void CRenderWorld::BeginFrame()
{
	_UnloadFrameConstants(m_matView, m_matProjection);

	m_pContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

	m_pContext->ClearRenderTargetView(m_pRenderTargetView.Get(), m_fBackColor);
	m_pContext->ClearDepthStencilView(m_pDepthStencilView.Get()
		, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pContext->RSSetViewports(1, &m_kViewPort);
}

void CRenderWorld::DrawFrame()
{
    // 1) shadow map을 DSV로 쓰기 전에, 이전 프레임에 PS에 물려 있던 shadow SRV 해제
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    m_pContext->PSSetShaderResources(1, 1, nullSRV);

    // 2) shadow pass
    m_pContext->OMSetRenderTargets(0, nullptr, m_pShadowDSV.Get());
    m_pContext->ClearDepthStencilView(m_pShadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    m_pContext->RSSetViewports(1, &m_kShadowViewport);

    m_renderManager.DrawPass(m_pContext, ERenderPass::SHADOW_PASS);

    // 3) main pass들
    m_pContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());
    m_pContext->RSSetViewports(1, &m_kViewPort);

    m_renderManager.DrawPass(m_pContext, ERenderPass::SKY_PASS);
	m_renderManager.DrawPass(m_pContext, ERenderPass::CLOUD_PASS);
    m_renderManager.DrawPass(m_pContext, ERenderPass::OPAQUE_PASS);
	m_renderManager.DrawPass(m_pContext, ERenderPass::CUTOUT_PASS);
    m_renderManager.DrawPass(m_pContext, ERenderPass::TRANSPARENT_PASS);
    m_renderManager.DrawPass(m_pContext, ERenderPass::DEBUG_PASS);


	_UnloadFrameConstants(m_matUIView, m_matUIProjection);
    m_renderManager.DrawPass(m_pContext, ERenderPass::ORTH_PASS);
}

void CRenderWorld::EndFrame()
{
	m_pSwapChain->Present(m_dxAdapter.IsVerticalSync() ? 1 : 0, 0);
}

void CRenderWorld::BeginBuildFrame()
{
	m_renderManager.BeginFrame();
}

void CRenderWorld::Submit(const RenderItem& renderItem)
{
	dbg.AddSubmittedRenderItem();
	m_renderManager.Submit(renderItem);
}

void CRenderWorld::EndBuildFrame()
{
	m_renderManager.EndFrame();
}

void CRenderWorld::_CreateConstFrameBuffer()
{
	// Create Frame const buffer
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DYNAMIC; // CPU_Write + GPU_Read
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(CB_FrameData);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	m_pDevice->CreateBuffer(&desc, nullptr, m_pCBFrame.GetAddressOf());
}

void CRenderWorld::_CreateConstObjectBuffer()
{
	// Create Frame const buffer
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DYNAMIC; // CPU_Write + GPU_Read
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(CB_ObjectData);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	m_pDevice->CreateBuffer(&desc, nullptr, m_pCBObject.GetAddressOf());
}

void CRenderWorld::_CreateRenderTargetView()
{
	// Backbuffer -> RTV
	m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&m_pBackBuffer));
	m_pDevice->CreateRenderTargetView(m_pBackBuffer.Get(), nullptr, m_pRenderTargetView.GetAddressOf());
}

void CRenderWorld::_CreateDepthStencilView()
{
	// Create Depth stencil
	D3D11_TEXTURE2D_DESC depthDesc{};
	depthDesc.Width = m_uWidth;
	depthDesc.Height = m_uHeight;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.CPUAccessFlags = 0;

	m_pDevice->CreateTexture2D(&depthDesc, nullptr, m_pDepthBuffer.GetAddressOf());
	m_pDevice->CreateDepthStencilView(m_pDepthBuffer.Get(), nullptr, m_pDepthStencilView.GetAddressOf());
}

void CRenderWorld::_CreateShadowResources()
{
	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = m_uShadowMapSize;
	texDesc.Height = m_uShadowMapSize;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	m_pDevice->CreateTexture2D(&texDesc, nullptr, m_pShadowTexture.GetAddressOf());

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	m_pDevice->CreateDepthStencilView(m_pShadowTexture.Get(), &dsvDesc, m_pShadowDSV.GetAddressOf());

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	m_pDevice->CreateShaderResourceView(m_pShadowTexture.Get(), &srvDesc, m_pShadowSRV.GetAddressOf());

	ZeroMemory(&m_kShadowViewport, sizeof(D3D11_VIEWPORT));
	m_kShadowViewport.TopLeftX = 0.0f;
	m_kShadowViewport.TopLeftY = 0.0f;
	m_kShadowViewport.Width = static_cast<float>(m_uShadowMapSize);
	m_kShadowViewport.Height = static_cast<float>(m_uShadowMapSize);
	m_kShadowViewport.MinDepth = 0.0f;
	m_kShadowViewport.MaxDepth = 1.0f;
}

void CRenderWorld::_UnloadFrameConstants(XMMATRIX view, XMMATRIX proj)
{
	CB_FrameData cb{};
	XMStoreFloat4x4(&cb.view, XMMatrixTranspose(view));
	XMStoreFloat4x4(&cb.proj, XMMatrixTranspose(proj));
	cb.lightDirWs = m_vLightDirWs;
	cb.lightColorIntensity = m_vLightColorIntensity;
	cb.ambientColor = m_vAmbientColor;
	cb.skyColor = m_vSkyColor;
	XMStoreFloat4x4(&cb.lightViewProj, XMMatrixTranspose(m_matLightViewProj));
	cb.shadowParams = m_vShadowParams;

	D3D11_MAPPED_SUBRESOURCE mapped{};
	m_pContext->Map(m_pCBFrame.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &cb, sizeof(CB_FrameData));
	m_pContext->Unmap(m_pCBFrame.Get(), 0);

	m_pContext->VSSetConstantBuffers(0, 1, m_pCBFrame.GetAddressOf());
	m_pContext->PSSetConstantBuffers(0, 1, m_pCBFrame.GetAddressOf());
}
