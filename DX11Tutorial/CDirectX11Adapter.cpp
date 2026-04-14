#include "pch.h"
#include "CDirectX11Adapter.h"

CDirectX11Adapter::CDirectX11Adapter()
{
}

CDirectX11Adapter::~CDirectX11Adapter()
{
	if (nullptr != m_pSwapChain)
	{
		// 창모드 설정 전 종료나 해제되는 경우 예외 발생 방지
		m_pSwapChain->SetFullscreenState(false, nullptr);
	}
}

HRESULT CDirectX11Adapter::Initialize(HWND hWnd_)
{
	m_hWnd = hWnd_;
	assert(m_hWnd);

	HRESULT hr = _CreateDeviceAndContext();
	assert(SUCCEEDED(hr));

	hr = _BindDXGI();
	assert(SUCCEEDED((hr)));

	hr = _CreateSwapChain();
	assert(SUCCEEDED((hr)));

	hr = _FetchDistplayModeList();
	assert(SUCCEEDED((hr)));

	return S_OK;
}

HRESULT CDirectX11Adapter::_CreateDeviceAndContext()
{
	HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &m_eFeatureLevel, 1, D3D11_SDK_VERSION,
		m_pDevice.GetAddressOf(), NULL, m_pContext.GetAddressOf());
	assert(SUCCEEDED(hr));
	return hr;
}

HRESULT CDirectX11Adapter::_BindDXGI()
{
	ComPtr<IDXGIDevice1> pDXGIDevice;

	// Device -> IDXGIDevice -> IDXGIAdapter -> IDXGIFactory
	HRESULT hr = m_pDevice.As(&pDXGIDevice);
	if (FAILED(hr)) return hr;

	hr = pDXGIDevice->GetAdapter(&m_pDXGIAdapter);
	if (FAILED(hr)) return hr;

	hr = m_pDXGIAdapter->GetParent(IID_PPV_ARGS(&m_pDXGIFactory));
	if (FAILED(hr)) return hr;

	hr = m_pDXGIAdapter->EnumOutputs(0, m_pDXGIOutput.GetAddressOf());
	if (FAILED(hr)) return hr;

	return S_OK;
}

HRESULT CDirectX11Adapter::_CreateSwapChain()
{
	DXGI_SWAP_CHAIN_DESC kDXGISwapChainDesc;
	ZeroMemory(&kDXGISwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	kDXGISwapChainDesc.BufferCount = 2;
	kDXGISwapChainDesc.BufferDesc.Width = 0;
	kDXGISwapChainDesc.BufferDesc.Height = 0;
	//kDXGISwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	kDXGISwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 버퍼의 refresh rate
	kDXGISwapChainDesc.BufferDesc.RefreshRate = { 0, 1 };

	// 스캔 라인 순서 및 크기 조절 지정 설정
	kDXGISwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	kDXGISwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// 백버퍼 사용 설정
	kDXGISwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// 랜더링할 창의 핸들
	kDXGISwapChainDesc.OutputWindow = m_hWnd;

	// 멀티 샘플링
	kDXGISwapChainDesc.SampleDesc.Count = 1;
	kDXGISwapChainDesc.SampleDesc.Quality = 0;

	// 창모드 (일단은 창모드로 만듬)
	kDXGISwapChainDesc.Windowed = TRUE;

	// 표시 후 백 버퍼 내용 설정
	kDXGISwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// 고급 플래그 설정
	kDXGISwapChainDesc.Flags = 0;

	HRESULT hr = m_pDXGIFactory->CreateSwapChain(m_pDevice.Get(), &kDXGISwapChainDesc, m_pSwapChain.GetAddressOf());

	if (FAILED(hr)) {
		assert(false && "Failed to create SwapCahin");
		return hr;
	}

	return S_OK;
}

HRESULT CDirectX11Adapter::_FetchDistplayModeList()
{
	// DXGI_ENUM_MODES_INTERLACED 인터레이스드는 채널 분리식으로 로드하므로 요즘 디스플레이에는 안어울림
	vector<DXGI_MODE_DESC> vecDXGIDisplayMode;

	UINT uNumModes = 0;
	HRESULT hr = m_pDXGIOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &uNumModes, nullptr);
	if (FAILED(hr)) return hr;

	vecDXGIDisplayMode.resize(uNumModes);
	if (vecDXGIDisplayMode.empty()) return E_FAIL;

	hr = m_pDXGIOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &uNumModes, vecDXGIDisplayMode.data());
	if (FAILED(hr)) return hr;

	// Enumorate Display model list
	m_vecDXGImodes.clear();
	m_vecDXGImodes.reserve(uNumModes);
	for (const DXGI_MODE_DESC& dxgiMode : vecDXGIDisplayMode)
	{
		if (dxgiMode.Scaling != DXGI_MODE_SCALING_UNSPECIFIED)
		{
			continue;
		}

		if (dxgiMode.ScanlineOrdering != DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE &&
			dxgiMode.ScanlineOrdering != DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED)
		{
			continue;
		}
		m_vecDXGImodes.push_back(dxgiMode);
	}

	return S_OK;
}



// 그언젠가 비디오 메모리에 접근할 일이 있다면...
//DXGI_ADAPTER_DESC dxgiAdapterDesc;
//hResult = pDXGIAdapter->GetDesc(&dxgiAdapterDesc);
//if (FAILED(hResult)) return hResult;
//// Store dedicated video card memory in megabytes
//m_dVideoCardMemory = (double)(dxgiAdapterDesc.DedicatedVideoMemory / 1024.0 / 1024.0);