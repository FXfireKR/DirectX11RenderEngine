#include "pch.h"
#include "Resource.h"
#include "CSystem.h"

CWindowSystem::CWindowSystem()
{
	m_hWnd = NULL;
	m_hInstance = NULL;
	m_pApplication = nullptr;
	m_lpcWstrApkName = L"";
}

CWindowSystem::~CWindowSystem()
{
}

bool CWindowSystem::Initialize()
{
	int iScreenWidth = 0;
	int iScreenHeight = 0;

	_InitializeWindow(iScreenWidth, iScreenHeight);

	if (nullptr == m_pApplication) 
		m_pApplication = new Application;

	if (false == m_pApplication->Initialize(m_hWnd, iScreenWidth, iScreenHeight)) 
		return false;

	m_bRunning = true;

	return true;
}

void CWindowSystem::Relase()
{
	if (nullptr != m_pApplication)
	{
		m_pApplication->Release(); 
		delete m_pApplication;
		m_pApplication = nullptr;
	}
}

void CWindowSystem::Run()
{
	MSG kMessage;
	ZeroMemory(&kMessage, sizeof(MSG));

	while (m_bRunning)
	{
		while (PeekMessage(&kMessage, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&kMessage);
			DispatchMessage(&kMessage);
		}

		_Tick();
	}
}

LRESULT CALLBACK CWindowSystem::WndHandler(HWND hWnd_, UINT uMessage_, WPARAM wParam_, LPARAM lParam_)
{
	return m_pApplication->WndProc(hWnd_, uMessage_, wParam_, lParam_);
	/*switch (uMessage_)
	{
		default: return m_pApplication->WndProc(hWnd_, uMessage_, wParam_, lParam_);
	}*/
}

void CWindowSystem::_InitializeWindow(int& iScreenWidth_, int& iScreenHeight_)
{
	DEVMODE kScreenSettings;
	int iPosX = 0, iPosY = 0;

	g_pSystem = this;
	m_hInstance = GetModuleHandle(NULL);
	m_lpcWstrApkName = L"Minecraft like voxel v1";

	WNDCLASSEXW wcex;
	wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = NULL;
	wcex.cbWndExtra     = NULL;
	wcex.hInstance      = m_hInstance;
	wcex.hIcon          = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_DX11TUTORIAL));
	wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = m_lpcWstrApkName;
	wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wcex.cbSize			= sizeof(WNDCLASSEX);
	RegisterClassExW(&wcex);

	iScreenWidth_ = GetSystemMetrics(SM_CXSCREEN);
	iScreenHeight_ = GetSystemMetrics(SM_CYSCREEN);

	// 현 시점에서는 의도된 상수
	if (true == FULL_SCREEN)
	{
		// 왜 Zeromem이 아니라 memset이지?
		memset(&kScreenSettings, 0, sizeof(DEVMODE));
		kScreenSettings.dmSize = sizeof(kScreenSettings);
		kScreenSettings.dmPelsWidth = (ULONG)iScreenWidth_;
		kScreenSettings.dmPelsHeight = (ULONG)iScreenHeight_;
		kScreenSettings.dmBitsPerPel = 32;
		kScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&kScreenSettings, CDS_FULLSCREEN);
	}
	else // (false == FULL_SCREEN)
	{
		iScreenWidth_ = INIT_SCREEN_SIZE_X;
		iScreenHeight_ = INIT_SCREEN_SIZE_Y;

		iPosX = (GetSystemMetrics(SM_CXSCREEN) - iScreenWidth_) / 2;
		iPosY = (GetSystemMetrics(SM_CYSCREEN) - iScreenHeight_) / 2;
	}

	g_ScreenSizeX = static_cast<ULONG>(iScreenWidth_);
	g_ScreenSizeY = static_cast<ULONG>(iScreenHeight_);

	m_hWnd = CreateWindowEx(WS_EX_APPWINDOW, m_lpcWstrApkName, m_lpcWstrApkName, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP
		, iPosX, iPosY, iScreenWidth_, iScreenHeight_, NULL, NULL, m_hInstance, NULL);

	ShowWindow(m_hWnd, SW_SHOW);
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);

	return;
}

void CWindowSystem::_Tick()
{
	m_pApplication->Tick();
}


LRESULT CALLBACK WndProc(HWND hWnd_, UINT uMessage_, WPARAM wParam_, LPARAM lParam_)
{
#ifdef IMGUI_ACTIVATE
	if (ImGui_ImplWin32_WndProcHandler(hWnd_, uMessage_, wParam_, lParam_))
		return true;
#endif // IMGUI_ACTIVATE

	switch (uMessage_)
	{
		case WM_CLOSE:
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		
		default:
		{
			return g_pSystem->WndHandler(hWnd_, uMessage_, wParam_, lParam_);
		}
	}
}