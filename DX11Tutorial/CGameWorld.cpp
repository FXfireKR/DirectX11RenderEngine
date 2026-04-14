#include "pch.h"
#include "CGameWorld.h"
#include "CRenderWorld.h"
#include "CBlockBreakParticleSystem.h"

void CGameWorld::Initialize(CRenderWorld& renderWorld_, CAudioSystem& audioSystem)
{
	m_pRenderWorld = &renderWorld_;
	m_pAudioSystem = &audioSystem;

	m_gameTimeManager.Init();

	_RegisterScenes();
	m_sceneManager.Initialize(*this);
	
	uint8_t it = static_cast<uint8_t>(SCENE_TYPE::BOOT_SCENE) + 1;
	for (; it < static_cast<uint8_t>(SCENE_TYPE::END_SCENE); ++it) {
		m_sceneManager.Create(static_cast<SCENE_TYPE>(it));
	}
	
	m_sceneManager.ChangeScene(SCENE_TYPE::INIT_SCENE);
}

void CGameWorld::Tick()
{
	m_gameTimeManager.Tick();

	float fDelta = m_gameTimeManager.GetDeltaTime();

	dbg.SetFPS(static_cast<float>(m_gameTimeManager.GetFps()));
	dbg.SetFrameMs(fDelta * 1000.0f);

	m_fAccumulatedTime += fDelta;
	m_iFixedUpdateProcCnt = 0;

	// fixed-update logic
	while (m_fAccumulatedTime >= FIXED_DELTA && m_iFixedUpdateProcCnt < MAX_FIXED_STEP)
	{
		m_sceneManager.FixedUpdate(FIXED_DELTA);
		m_fAccumulatedTime -= FIXED_DELTA;
		++m_iFixedUpdateProcCnt;
	}

	float updateMs = 0.f;
	{
		CScopedCpuTimer timer(updateMs);
		m_sceneManager.Update(fDelta);
	}
	dbg.SetUpdateMs(updateMs);

	float lasUpdateMs = 0.f;
	{
		CScopedCpuTimer timer(lasUpdateMs);
		m_sceneManager.LateUpdate(fDelta);
	}
	dbg.SetLateUpdateMs(lasUpdateMs);
}

void CGameWorld::CommitFrameFence()
{
	m_sceneManager.CommitFrameFence();
}

void CGameWorld::BuildRenderFrame()
{
	float renderBuildMs = 0.f;
	{
		CScopedCpuTimer timer(renderBuildMs);

		m_sceneManager.Build();

		m_pRenderWorld->BeginBuildFrame();
		{
			m_sceneManager.BuildRenderFrame();
		}
		m_pRenderWorld->EndBuildFrame();
	}
	dbg.SetRenderBuildMs(renderBuildMs);
}

void CGameWorld::RenderDebugOverlay()
{
	m_debugOverlay.Render();
}

void CGameWorld::_RegisterScenes()
{
	m_sceneManager.Register(SCENE_TYPE::BOOT_SCENE, []() { return make_unique<CBootScene>(); });
	m_sceneManager.Register(SCENE_TYPE::INIT_SCENE, []() { return make_unique<CInitializeScene>(); });
	m_sceneManager.Register(SCENE_TYPE::GAME_SCENE, []() { return make_unique<CGameScene>(); });
	m_sceneManager.Register(SCENE_TYPE::TEST_SCENE, []() { return make_unique<CTestScene>(); });
}