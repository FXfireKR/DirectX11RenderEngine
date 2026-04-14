#include "pch.h"
#include "CApplication.h"
#include "CDeltaTimeManager.h"

bool Application::Initialize(HWND hWnd_, int iScreenWidth_, int iScreenHeight_)
{
	m_renderWorld.Initialize(hWnd_, iScreenWidth_, iScreenHeight_);
	m_audio.Initialize();

	m_gameWorld.Initialize(m_renderWorld, m_audio);

	m_window.Initialize(hWnd_);

	_ImGuiInitialize(hWnd_);
	_RegisterRawInput(hWnd_);

	m_rawInputDispatcher.GetMouse().SetWindowTarget(m_window);

	CInputManager& inputManager = CInputManager::Get();
	inputManager.Initialize();
	inputManager.SetMouseDevice(m_rawInputDispatcher.GetMouse());
	inputManager.SetKeyboardDevice(m_rawInputDispatcher.GetKeyboard());
	inputManager.SetGamePadDevice(m_rawInputDispatcher.GetGamePad());

	return true;
}

void Application::Release()
{
#ifdef IMGUI_ACTIVATE
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // IMGUI_ACTIVATE
}

void Application::Tick()
{
	PROFILE_FRAME();
	PROFILE_SCOPE();

	_BeginFrame();
	_RunGameFrame();
	_CommitFrameFence();
	_BuildRenderFrame();
	_ExecuteRenderFrame();
	_EndFrame();
}

LRESULT Application::WndProc(HWND hWnd_, UINT uMessage_, WPARAM wParam_, LPARAM lParam_)
{
#ifdef IMGUI_ACTIVATE
	if (ImGui_ImplWin32_WndProcHandler(hWnd_, uMessage_, wParam_, lParam_))
		return true;
#endif // IMGUI_ACTIVATE

	switch (uMessage_)
	{
		case WM_MOVE:
		case WM_SIZE:
			break;

		case WM_INPUT :
		{
			UINT size = 0;
			GetRawInputData((HRAWINPUT)lParam_, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));

			static BYTE buffer[1024];
			if (size > sizeof(buffer)) 
				break;

			if (GetRawInputData((HRAWINPUT)lParam_, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER)) != size) 
				break;

			RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(buffer);
			m_rawInputDispatcher.Push(*raw);
			return 0;
		} break;

		case WM_KEYUP: {
			if (wParam_ == VK_ESCAPE) 
				DestroyWindow(hWnd_);
		} break;
	}

	return DefWindowProc(hWnd_, uMessage_, wParam_, lParam_);
}

void Application::_BeginFrame()
{
	m_window.CalcWindowSize();

	dbg.BeginFrame();
	CInputManager::Get().BeginFrame();

	m_rawInputDispatcher.DispatchRawQueue();

#ifdef IMGUI_ACTIVATE
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif // IMGUI_ACTIVATE
}

void Application::_RunGameFrame()
{
	m_gameWorld.Tick();
}

void Application::_CommitFrameFence()
{
	m_gameWorld.CommitFrameFence();
	m_audio.Tick();
}

void Application::_BuildRenderFrame()
{
	m_gameWorld.BuildRenderFrame();
	m_gameWorld.RenderDebugOverlay();
}

void Application::_ExecuteRenderFrame()
{
	float renderExecuteMs = 0.f;
	{
		CScopedCpuTimer timer(renderExecuteMs);

		m_renderWorld.BeginFrame();
		m_renderWorld.DrawFrame();

#ifdef IMGUI_ACTIVATE
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif // IMGUI_ACTIVATE
	}
	dbg.SetRenderExecuteMs(renderExecuteMs);

	float presentMs = 0.f;
	{
		CScopedCpuTimer timer(presentMs);
		m_renderWorld.EndFrame();
	}
	dbg.SetPresentMs(presentMs);
}

void Application::_EndFrame()
{
	CInputManager::Get().EndFrame();
	dbg.EndFrame();
}

bool Application::_ImGuiInitialize(HWND hWnd_)
{
#ifdef IMGUI_ACTIVATE
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();

	if (false == ImGui_ImplWin32_Init(hWnd_))
		return false;

	if (false == ImGui_ImplDX11_Init(m_renderWorld.GetDevice(), m_renderWorld.GetContext()))
		return false;
#endif // IMGUI_ACTIVATE
	return true;
}

void Application::_RegisterRawInput(HWND hWnd_)
{
	const UINT SIZE_OF_DEVICE = 3;
	RAWINPUTDEVICE devices[SIZE_OF_DEVICE] = {};

	// keyboard
	devices[0].usUsagePage = 0x01;
	devices[0].usUsage = 0x06; // keyboard
	devices[0].dwFlags = 0;
	devices[0].hwndTarget = hWnd_;

	// mouse
	devices[1].usUsagePage = 0x01;
	devices[1].usUsage = 0x02; // mouse
	devices[1].dwFlags = 0;
	devices[1].hwndTarget = hWnd_;

	// game pad
	devices[2].usUsagePage = 0x01;
	devices[2].usUsage = 0x05; // game pad
	devices[2].dwFlags = 0;
	devices[2].hwndTarget = hWnd_;

	if (FALSE == RegisterRawInputDevices(devices, SIZE_OF_DEVICE, sizeof(RAWINPUTDEVICE))) {
		assert(false && "RegisterRawInputDevice failed!");
	}
}
