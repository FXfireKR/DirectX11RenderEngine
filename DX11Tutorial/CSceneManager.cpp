#include "pch.h"
#include "CSceneManager.h"

void CSceneManager::Initialize(CGameWorld& gameWorld)
{
	m_pGameWorld = &gameWorld;

	Create(SCENE_TYPE::BOOT_SCENE);
	m_eCurrentSceneType = SCENE_TYPE::BOOT_SCENE;

	CScene* pCurrentScene = _GetCurrent();
	pCurrentScene->Awake();
	pCurrentScene->Start();
}

void CSceneManager::Register(SCENE_TYPE eType_, SceneFactory factory_)
{
	m_mapFactories[eType_] = move(factory_);
}

void CSceneManager::Create(SCENE_TYPE eType_)
{
	auto iter = m_mapFactories.find(eType_);
	if (iter != m_mapFactories.end()) {
		uint32_t uIndex = static_cast<uint32_t>(eType_);
		m_arrayScene[uIndex] = iter->second();

		m_arrayScene[uIndex]->OnCreate(*m_pGameWorld);
	}
}

void CSceneManager::Remove(SCENE_TYPE eType_)
{
	UNREFERENCED_PARAMETER(eType_);
}

void CSceneManager::ChangeScene(SCENE_TYPE eNext_)
{
	if (m_eSceneChangeState != SCENE_CHANGE_STATE::NONE) return;
	if (eNext_ == SCENE_TYPE::END_SCENE) return;
	if (eNext_ == m_eCurrentSceneType) return;

	m_eNextSceneType = eNext_;
	m_eSceneChangeState = SCENE_CHANGE_STATE::UNLOADING;
}

void CSceneManager::FixedUpdate(float fDelta)
{
	if (m_eCurrentSceneType == SCENE_TYPE::END_SCENE)
		return;

	CScene* pCurrent = _GetCurrent();
	if (pCurrent)
	{
		pCurrent->FixedUpdate(fDelta);
	}
}

void CSceneManager::Update(float fDelta)
{
	if (m_eCurrentSceneType == SCENE_TYPE::END_SCENE)
		return;

	CScene* pCurrent = _GetCurrent();
	if (pCurrent)
	{
		pCurrent->Update(fDelta);
	}
}

void CSceneManager::LateUpdate(float fDelta)
{
	if (m_eCurrentSceneType == SCENE_TYPE::END_SCENE)
		return;

	CScene* pCurrent = _GetCurrent();
	if (pCurrent)
	{
		pCurrent->LateUpdate(fDelta);
	}
}

void CSceneManager::Build()
{
	if (m_eCurrentSceneType == SCENE_TYPE::END_SCENE)
		return;

	CScene* pCurrent = _GetCurrent();
	if (pCurrent)
	{
		pCurrent->Build();
	}
}

void CSceneManager::CommitFrameFence()
{
	if (m_eCurrentSceneType == SCENE_TYPE::END_SCENE)
		return;

	CScene* pCurrent = _GetCurrent();
	if (nullptr == pCurrent)
		return;

	if (pCurrent->IsRequestForChange())
	{
		const SCENE_TYPE nextScene = pCurrent->GetNextScene();
		pCurrent->ResetChangeSceneRequest();
		ChangeScene(nextScene);
	}

	switch (m_eSceneChangeState)
	{
		case SCENE_CHANGE_STATE::NONE:
		{
			pCurrent->CommitFrameFence();
		} break;

		case SCENE_CHANGE_STATE::UNLOADING:
		{
			_UnloadCurrentScene();
			m_eSceneChangeState = SCENE_CHANGE_STATE::LOADING;
		} break;

		case SCENE_CHANGE_STATE::LOADING:
		{
			_LoadNextScene();
			m_eSceneChangeState = SCENE_CHANGE_STATE::ACTIVATING;
		} break;

		case SCENE_CHANGE_STATE::ACTIVATING:
		{
			_ActivatingCurrentScene();
			m_eSceneChangeState = SCENE_CHANGE_STATE::NONE;

			CScene* pActivated = _GetCurrent();
			if (pActivated)
			{
				pActivated->CommitFrameFence();
			}
		} break;
	}
}

void CSceneManager::BuildRenderFrame()
{
	if (m_eCurrentSceneType == SCENE_TYPE::END_SCENE)
		return;

	CScene* pCurrent = _GetCurrent();
	if (pCurrent)
	{
		pCurrent->BuildRenderFrame();
	}
}

void CSceneManager::_UnloadCurrentScene()
{
	if (m_eCurrentSceneType == SCENE_TYPE::END_SCENE) 
		return;
}

void CSceneManager::_LoadNextScene()
{
	if (m_eNextSceneType == SCENE_TYPE::END_SCENE)
		return;

	CScene* pNext = _GetNext();
	if (pNext)
	{
		pNext->Awake();
	}
}

void CSceneManager::_ActivatingCurrentScene()
{
	if (m_eNextSceneType == SCENE_TYPE::END_SCENE)
		return;

	CScene* pNext = _GetNext();
	if (pNext)
	{
		pNext->Start();

		m_eCurrentSceneType = m_eNextSceneType;
		m_eNextSceneType = SCENE_TYPE::END_SCENE;
	}
}