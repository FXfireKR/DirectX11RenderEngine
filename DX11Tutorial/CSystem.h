#pragma once
#include "CApplication.h"

class CWindowSystem
{
public:
	CWindowSystem();
	~CWindowSystem();

	bool Initialize();
	void Relase();
	void Run();
	LRESULT CALLBACK WndHandler(HWND hWnd_, UINT uMessage_, WPARAM wParam_, LPARAM lParam_);

private:
	void _InitializeWindow(int& iScreenWidth_, int& iScreenHeight_);
	void _Tick();

private:
	HWND m_hWnd;
	HINSTANCE m_hInstance;
	LPCWSTR m_lpcWstrApkName;

	Application* m_pApplication;

	bool m_bRunning = false;
};

static LRESULT CALLBACK WndProc(HWND hWnd_, UINT uMessage_, WPARAM wParam_, LPARAM lParam_);
static CWindowSystem* g_pSystem = 0;