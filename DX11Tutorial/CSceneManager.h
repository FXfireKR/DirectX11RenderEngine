#pragma once
#include "SceneHeader.h"

enum class SCENE_CHANGE_STATE : uint32_t
{
	NONE = 0,
	UNLOADING,	// prev scene unload
	LOADING,	// next scene load
	ACTIVATING,	// current scene activate
};

using SceneFactory = std::function<unique_ptr<CScene>()>;

class CSceneManager
{
public:
	CSceneManager() = default;
	~CSceneManager() = default;

	void Initialize(CGameWorld& gameWorld);
	void Register(SCENE_TYPE eType_, SceneFactory factory_);

	void Create(SCENE_TYPE eType_);
	void Remove(SCENE_TYPE eType_);

	void ChangeScene(SCENE_TYPE eNext_);

	void FixedUpdate(float fDelta);
	void Update(float fDelta);
	void LateUpdate(float fDelta);
	void Build();
	void CommitFrameFence();
	void BuildRenderFrame();

private:
	void _UnloadCurrentScene();
	void _LoadNextScene();
	void _ActivatingCurrentScene();

public:
	inline SCENE_TYPE GetCurrentSceneType() const { return m_eCurrentSceneType; }

private:
	inline CScene* const _GetCurrent() { return m_arrayScene[static_cast<uint32_t>(m_eCurrentSceneType)].get(); }
	inline CScene* const _GetNext() { return m_arrayScene[static_cast<uint32_t>(m_eNextSceneType)].get(); }

private:
	unordered_map<SCENE_TYPE, SceneFactory> m_mapFactories;
	array<unique_ptr<CScene>, SCENE_MAX_SIZE> m_arrayScene; // CScene의 own은 여기

	SCENE_CHANGE_STATE m_eSceneChangeState = SCENE_CHANGE_STATE::NONE;
	SCENE_TYPE m_eCurrentSceneType = SCENE_TYPE::END_SCENE;
	SCENE_TYPE m_eNextSceneType = SCENE_TYPE::END_SCENE;

	CGameWorld* m_pGameWorld = nullptr;
};