#pragma once
#include "CGameWorld.h"
#include "CRenderWorld.h"
#include "CRawInputDispatcher.h"
#include "CWindow.h"
#include "CInputManager.h"
#include "CAudioSystem.h"

const bool FULL_SCREEN = false;
const int SCREENX = 1440;
const int SCREENY = 1024;

class Application
{
public:
	Application() = default;
	~Application() = default;

	bool Initialize(HWND hWnd_, int iScreenWidth_, int iScreenHeight_);
	void Release();
	void Tick();
	LRESULT CALLBACK WndProc(HWND hWnd_, UINT uMessage_, WPARAM wParam_, LPARAM lParam_);

public:
	inline CAudioSystem& GetAudioSystem() { return m_audio; }

private:
	void _BeginFrame();
	void _RunGameFrame();
	void _CommitFrameFence();
	void _BuildRenderFrame();
	void _ExecuteRenderFrame();
	void _EndFrame();

private:
	bool _ImGuiInitialize(HWND hWnd_);
	void _RegisterRawInput(HWND hWnd_);

private:
	CWindow m_window;
	CGameWorld m_gameWorld;
	CRenderWorld m_renderWorld;
	CAudioSystem m_audio;

	CRawInputDispatcher m_rawInputDispatcher;
};