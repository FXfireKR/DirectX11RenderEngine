#pragma once
#define _HAS_STD_BYTE 0

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

// Windows 헤더 파일
#define NOMINMAX
#include <windows.h>


#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>

//#define DEBUG_LOG
#ifdef DEBUG_LOG
#include <iostream>
#endif // DEBUG_LOG

#include <fstream>
#include <functional>
#include <thread>
#include <mutex>
#include <ctime>
#include <atomic>
#include <condition_variable>
#include <sstream>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <cstdint>
#include <algorithm>
#include <math.h>
using namespace std;

// STL
#include <vector>
#include <map>
#include <list>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>

#include "sparse_set.hpp"

// DIRECTX 11 x64
#include <D3D11.h>
#include <DXGI.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <wincodec.h>
#include <wrl.h>

#include <DirectXMath.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")
using namespace DirectX;
using namespace Microsoft::WRL;


// DirectX Tex
#include <DirectXTex/DirectXTex.h>
#ifdef _DEBUG
#pragma comment(lib, "DirectXTex\\DirectXTex_debug.lib")
#else // _DEBUG
#pragma comment(lib, "DirectXTex\\DirectXTex.lib")
#endif // _DEBUG

// DirectXTK
#include <DirectXTK/WICTextureLoader.h>
#include <DirectXTK/DDSTextureLoader.h>
#include <DirectXTK/SimpleMath.h>
#ifdef _DEBUG
#pragma comment(lib, "DirectXTK\\x64\\debug\\DirectXTK.lib")
#else // _DEBUG
#pragma comment(lib, "DirectXTK\\x64\\release\\DirectXTK.lib")
#endif // _DEBUG

// FMOD
#include <FMOD/fmod.hpp>
#ifdef _DEBUG
#pragma comment(lib, "FMOD\\debug\\fmodL_vc.lib")
#else // _DEBUG
#pragma comment(lib, "FMOD\\release\\fmod_vc.lib")
#endif // _DEBUG

// Rapid-JSON
#include <rapidjson\rapidjson.h>
#include <rapidjson\document.h>
#include <rapidjson\stringbuffer.h>
using namespace rapidjson;


// OPTICK 활성
//#define OPTICK_PROFILING

// Optick Profiler
#ifdef OPTICK_PROFILING
#include <Optick\optick.h>
#include <Optick\optick.config.h>
#ifdef _DEBUG
#pragma comment(lib, "Optick\\debug\\OptickCore.lib")
#else // _DEBUG
#pragma comment(lib, "Optick\\release\\OptickCore.lib")
#endif // _DEBUG
#endif // OPTICK_PROFILING

#if defined(OPTICK_PROFILING)
#define PROFILE_FRAME() OPTICK_FRAME("Main")
#define PROFILE_SCOPE() OPTICK_EVENT()
#else
#define PROFILE_FRAME() ((void)0)
#define PROFILE_SCOPE() ((void)0)
#endif


// ImGuI
#define IMGUI_ACTIVATE 1

#ifdef IMGUI_ACTIVATE
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#else // IMGUI_ACTIVATE
#define IMGUI_DISABLE
#endif // IMGUI_ACTIVATE

#define RELEASE(p) if (nullptr != p) { p->Release(); p = nullptr; }

//constexpr unsigned long INIT_SCREEN_SIZE_X = 1440;
//constexpr unsigned long INIT_SCREEN_SIZE_Y = 1024;

constexpr unsigned long INIT_SCREEN_SIZE_X = 2560;
constexpr unsigned long INIT_SCREEN_SIZE_Y = 1440;

extern unsigned long g_ScreenSizeX;
extern unsigned long g_ScreenSizeY;

#include "util.h"

struct UVRect
{
	float u0, v0, u1, v1;
};

// singleton
#include "CDebugCollector.h"
#include "CInputManager.h"
#include "CBlockDB.h"
#include "CBlockResourceDB.h"

#define dbg CDebugCollector::Get()
#define BlockDB CBlockDB::Get()
#define BlockResDB CBlockResourceDB::Get()


// 


////////////////////////////////////////////////////////////////////////////
// 
// AppliedModel vector 복사가 아닌 주소 받아오도록 변경.
// BakeQuad 복사는 필요하지 않으면 하지 않고 source 그대로 쓰도록 변경.
// 
#define OPTIMIZATION_1
// UpdateStream load/unload 단계를 상세히 나눠서 cpu 사용율 분산.
// 
#define OPTIMIZATION_2
////////////////////////////////////////////////////////////////////////////