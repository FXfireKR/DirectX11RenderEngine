#pragma once
#include "CDeltaTimeManager.h"
#include "CSceneManager.h"
#include "CDebugOverlay.h"

class CRenderWorld;

class CGameWorld
{
public:
	CGameWorld() = default;
	~CGameWorld() = default;

	void Initialize(CRenderWorld& renderWorld_, CAudioSystem& audioSystem);
	void Tick();
	void CommitFrameFence();
	void BuildRenderFrame();
	void RenderDebugOverlay();

public:
	inline CRenderWorld* GetRenderWorld() const { return m_pRenderWorld; }
	inline CAudioSystem* GetAudioSystem() const { return m_pAudioSystem; }

private:
	void _RegisterScenes();

private:
	CDebugOverlay m_debugOverlay;
	CDeltaTimeManager m_gameTimeManager;
	CSceneManager m_sceneManager;

	CRenderWorld* m_pRenderWorld = nullptr;
	CAudioSystem* m_pAudioSystem = nullptr;

private: // fixed-update logic elements
	const float FIXED_DELTA = 1.0f / 60.0f; // 120.f는 추후
	const int MAX_FIXED_STEP = 5; // 5 * (FIXED_DELTA) = 약 83ms
	float m_fAccumulatedTime = 0.0f;
	int m_iFixedUpdateProcCnt = 0;
};