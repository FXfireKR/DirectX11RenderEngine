#include "pch.h"
#include "DirectX11Com.h"

DirectX11Com::DirectX11Com()
{
	m_bVerticalSync = true;
	m_dVideoCardMemory = 0.0;
	m_uDXGIDisplayWidth = m_uDXGIDisplayHeight = 0;
	ZeroMemory(&m_uDXGIDisplayRational, sizeof(DXGI_RATIONAL));

	m_pDepthStencilState = nullptr;
	m_pRasterState = nullptr;

	ZeroMemory(&m_kViewPort, sizeof(m_kViewPort));

	m_matProjection = XMMatrixIdentity();
	m_matWorld = XMMatrixIdentity();
	m_matOrtho = XMMatrixIdentity();
}

DirectX11Com::~DirectX11Com()
{
	_ImGuiRelease();

	if (nullptr != m_pSwapChain)
	{
		// 창모드 설정 전 종료나 해제되는 경우 예외 발생 방지
		m_pSwapChain->SetFullscreenState(false, nullptr); 
	}
}

HRESULT DirectX11Com::Initialize(HWND hWnd_, int iScreenWidth_, int iScreenHeight_, bool bFullScreen_, float fScreenDepth_, float fScreenNear_)
{	
	HRESULT hResult;

	//// DXGI Load
	//{
	//	IDXGIFactory1* pDXGIFactory = nullptr;
	//	IDXGIAdapter* pDXGIAdapter = nullptr;
	//	IDXGIOutput* pDXGIOutput = nullptr;
	//	DXGI_MODE_DESC* pDXGIDisplayModeList = nullptr;

	//	hResult = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pDXGIFactory);
	//	if (FAILED(hResult)) return hResult;

	//	hResult = pDXGIFactory->EnumAdapters(0, &pDXGIAdapter);
	//	if (FAILED(hResult)) return hResult;

	//	hResult = pDXGIAdapter->EnumOutputs(0, &pDXGIOutput);
	//	if (FAILED(hResult)) return hResult;

	//	// Get DisplayModelList
	//	UINT uNumModes = 0;
	//	hResult = pDXGIOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &uNumModes, nullptr);
	//	if (FAILED(hResult)) return hResult;

	//	pDXGIDisplayModeList = new DXGI_MODE_DESC[uNumModes];
	//	if (nullptr == pDXGIDisplayModeList) return E_FAIL;

	//	hResult = pDXGIOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &uNumModes, pDXGIDisplayModeList);
	//	if (FAILED(hResult)) return hResult;

	//	// Enumorate Display model list
	//	for (UINT i = 0; i < uNumModes; ++i)
	//	{
	//		if (pDXGIDisplayModeList[i].Width == (UINT)iScreenWidth_)
	//		{
	//			if (pDXGIDisplayModeList[i].Height == (UINT)iScreenHeight_)
	//			{
	//				m_uDXGIDisplayRational = pDXGIDisplayModeList[i].RefreshRate;
	//				break;
	//			}
	//		}
	//	}

	//	DXGI_ADAPTER_DESC dxgiAdapterDesc;
	//	hResult = pDXGIAdapter->GetDesc(&dxgiAdapterDesc);
	//	if (FAILED(hResult)) return hResult;

	//	// Store dedicated video card memory in megabytes
	//	m_dVideoCardMemory = (double)(dxgiAdapterDesc.DedicatedVideoMemory / 1024.0 / 1024.0);

	//	// release memory
	//	if (nullptr != pDXGIDisplayModeList)
	//	{
	//		delete[] pDXGIDisplayModeList;
	//		pDXGIDisplayModeList = nullptr;
	//	}
	//	RELEASE(pDXGIOutput);
	//	RELEASE(pDXGIAdapter);
	//	RELEASE(pDXGIFactory);
	//}
	//
	//// D3DX Setting
	//{
	//	DXGI_SWAP_CHAIN_DESC kDXGISwapChainDesc;
	//	D3D_FEATURE_LEVEL kFeatureLevel;
	//	ID3D11Texture2D* pBackBuffer;
	//	D3D11_TEXTURE2D_DESC kDepthBufferDesc;
	//	D3D11_DEPTH_STENCIL_DESC kDepthStencilDesc;
	//	D3D11_DEPTH_STENCIL_VIEW_DESC kDepthStencilViewDesc;
	//	D3D11_RASTERIZER_DESC kRasterDesc;
	//	float fFieldOfView, fScreenAspect;

	//	// SwapChain Setting
	//	ZeroMemory(&kDXGISwapChainDesc, sizeof(kDXGISwapChainDesc));
	//	kDXGISwapChainDesc.BufferCount = 1;	// 단일 백버퍼 설정한다는 뜻
	//	kDXGISwapChainDesc.BufferDesc.Width = iScreenWidth_;
	//	kDXGISwapChainDesc.BufferDesc.Height = iScreenHeight_;
	//	kDXGISwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32비트 표면 설정
	//	
	//	// 수직 동기화 설정 (hz)
	//	if (true == m_bVerticalSync)
	//	{
	//		kDXGISwapChainDesc.BufferDesc.RefreshRate = m_uDXGIDisplayRational;
	//	}
	//	else // (false == m_bVerticalSync)
	//	{
	//		kDXGISwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	//		kDXGISwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	//	}

	//	// 백버퍼 사용 설정
	//	kDXGISwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	//	// 랜더링할 창의 핸들
	//	kDXGISwapChainDesc.OutputWindow = hWnd_;

	//	// 멀티 샘플링
	//	kDXGISwapChainDesc.SampleDesc.Count = 1;
	//	kDXGISwapChainDesc.SampleDesc.Quality = 0;

	//	// 창모드
	//	kDXGISwapChainDesc.Windowed = !bFullScreen_;

	//	// 스캔 라인 순서 및 크기 조절 지정 설정
	//	kDXGISwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	//	kDXGISwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//	// 표시 후 백 버퍼 내용 설정
	//	kDXGISwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	//	// 고급 플래그 설정
	//	kDXGISwapChainDesc.Flags = 0;

	//	// 기능 수준 설정
	//	kFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	//	
	//	// try to create device and swapchain
	//	hResult = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &kFeatureLevel, 1, D3D11_SDK_VERSION
	//		, &kDXGISwapChainDesc, m_pSwapChain.GetAddressOf(), m_pDevice.GetAddressOf(), NULL, m_pDeviceContext.GetAddressOf());

	//	// Back buffer connect with Swap chain
	//	hResult = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	//	if (FAILED(hResult)) return hResult;

	//	hResult = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, m_pRenderTargetView.GetAddressOf());
	//	if (FAILED(hResult)) return hResult;

	//	RELEASE(pBackBuffer);

	//	// Depth buffer setting
	//	{
	//		ZeroMemory(&kDepthBufferDesc, sizeof(kDepthBufferDesc));
	//		kDepthBufferDesc.Width = iScreenWidth_;
	//		kDepthBufferDesc.Height = iScreenHeight_;
	//		kDepthBufferDesc.MipLevels = 1;
	//		kDepthBufferDesc.ArraySize = 1;
	//		kDepthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//		kDepthBufferDesc.SampleDesc.Count = 1;
	//		kDepthBufferDesc.SampleDesc.Quality = 0;
	//		kDepthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	//		kDepthBufferDesc.CPUAccessFlags = 0;
	//		kDepthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	//		kDepthBufferDesc.MiscFlags = 0;

	//		hResult = m_pDevice->CreateTexture2D(&kDepthBufferDesc, nullptr, &m_pDepthStencilBuffer);
	//		if (FAILED(hResult)) return hResult;
	//	}

	//	// Depth stencil buffer setting
	//	{
	//		ZeroMemory(&kDepthStencilDesc, sizeof(kDepthStencilDesc));
	//		kDepthStencilDesc.DepthEnable = true;
	//		kDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	//		kDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	//		kDepthStencilDesc.StencilEnable = true;
	//		kDepthStencilDesc.StencilReadMask = 0xFF;
	//		kDepthStencilDesc.StencilWriteMask = 0xFF;

	//		// 픽셀이 정면을 향하는 경우 스텐실 설정
	//		kDepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	//		kDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	//		kDepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	//		kDepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//		// 픽셀이 후면을 향하는 경우 스텐실 설정
	//		kDepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	//		kDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	//		kDepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	//		kDepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//		hResult = m_pDevice->CreateDepthStencilState(&kDepthStencilDesc, &m_pDepthStencilState);
	//		if (FAILED(hResult)) return hResult;

	//		// 깊이 스텐실 버퍼 세팅
	//		m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilState, 1);
	//	}

	//	// Depth stencil View setting
	//	{
	//		ZeroMemory(&kDepthStencilViewDesc, sizeof(kDepthStencilViewDesc));

	//		kDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//		kDepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	//		kDepthStencilViewDesc.Texture2D.MipSlice = 0;

	//		hResult = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &kDepthStencilViewDesc, &m_pDepthStencilView);
	//		if (FAILED(hResult)) return hResult;

	//		// Render target view 와 Depth stencil buffer 가 렌더 파이프라인에 바인딩 된다.
	//		m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView);
	//	}

	//	// Rasterizer setting
	//	{
	//		kRasterDesc.AntialiasedLineEnable = false;
	//		kRasterDesc.CullMode = D3D11_CULL_BACK;
	//		kRasterDesc.DepthBias = 0;
	//		kRasterDesc.DepthBiasClamp = 0.0f;
	//		kRasterDesc.DepthClipEnable = true;
	//		kRasterDesc.FillMode = D3D11_FILL_SOLID;
	//		//kRasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	//		kRasterDesc.FrontCounterClockwise = false;
	//		kRasterDesc.MultisampleEnable = false;
	//		kRasterDesc.ScissorEnable = false;
	//		kRasterDesc.SlopeScaledDepthBias = 0.0f;

	//		hResult = m_pDevice->CreateRasterizerState(&kRasterDesc, &m_pRasterState);
	//		if (FAILED(hResult)) return hResult;

	//		m_pDeviceContext->RSSetState(m_pRasterState);
	//	}
	//	
	//	// View port setting
	//	{
	//		m_kViewPort.Width = (float)iScreenWidth_;
	//		m_kViewPort.Height = (float)iScreenHeight_;
	//		m_kViewPort.MinDepth = 0.0f;
	//		m_kViewPort.MaxDepth = 1.f;
	//		m_kViewPort.TopLeftX = 0.0f;
	//		m_kViewPort.TopLeftY = 0.0f;

	//		m_pDeviceContext->RSSetViewports(1, &m_kViewPort);
	//	}
	//	
	//	// projection matrix
	//	fFieldOfView = XM_PI * 0.25f;
	//	fScreenAspect = m_kViewPort.Width / m_kViewPort.Height;
	//	
	//	m_matProjection = XMMatrixPerspectiveFovLH(fFieldOfView, fScreenAspect, fScreenNear_, fScreenDepth_);
	//	m_matWorld = XMMatrixIdentity();
	//}

	//// ImGuI Init
	//_ImGuiInit(hWnd_);

	return hResult;
}

void DirectX11Com::BeginRender()
{
	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	//SetBackBufferRenderTarget();

	// 백버퍼를 지웁니다
	//m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), color);

	// 깊이 버퍼를 지웁니다
	//m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void DirectX11Com::EndRender()
{
	HRESULT hr;

	if (true == m_bVerticalSync)
	{
		hr = m_pSwapChain->Present(1, 0);
	}
	else // (false == m_bVerticalSync)
	{
		hr = m_pSwapChain->Present(0, 0);
	}
}

void DirectX11Com::SetBackBufferRenderTarget()
{
	// 후면 버퍼에 그리라고 지정해주는 것.
	//m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView);
}

void DirectX11Com::ResetViewPort()
{
	m_pDeviceContext->RSSetViewports(1, &m_kViewPort);
}


void DirectX11Com::ImGuiTick()
{
#ifdef IMGUI_ACTIVATE
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif // IMGUI_ACTIVATE
}

void DirectX11Com::ImGuiRender()
{
#ifdef IMGUI_ACTIVATE
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif // IMGUI_ACTIVATE
}

HRESULT DirectX11Com::_ImGuiInit(HWND hWnd_)
{
#ifdef IMGUI_ACTIVATE
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();

	if (false == ImGui_ImplWin32_Init(hWnd_)) 
		return E_FAIL;

	if (false == ImGui_ImplDX11_Init(m_pDevice.Get(), m_pDeviceContext.Get()))
		return E_FAIL;
#endif // IMGUI_ACTIVATE

	return S_OK;
}

void DirectX11Com::_ImGuiRelease()
{
#ifdef IMGUI_ACTIVATE
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // IMGUI_ACTIVATE
}
