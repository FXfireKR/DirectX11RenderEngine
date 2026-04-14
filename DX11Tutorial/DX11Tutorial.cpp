#include "pch.h"
#include "CSystem.h"

#ifdef DEBUG_LOG
// CMD form
#ifdef UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#else // UNICODE
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif // UNICODE

// Memory Debug
#include <crtdbg.h>
static int nFlag = _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//static int nBreak_1 = _CrtSetBreakAlloc(2595);

#endif // DEBUG_LOG

unsigned long g_ScreenSizeX = 0;
unsigned long g_ScreenSizeY = 0;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	CWindowSystem* pSystem = new CWindowSystem;
	if (true == pSystem->Initialize())
	{
		pSystem->Run();
	}

	pSystem->Relase();
	delete pSystem;
	pSystem = nullptr;

#ifdef _DEBUG
	// memory leak check
	if (_CrtDumpMemoryLeaks()) {
		int iLeakCheckPlz = INT_MIN;
	}
#endif // _DEBUG

	return 0;
}