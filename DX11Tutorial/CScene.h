#pragma once
#include "CObjectManager.h"
#include "GameWorldHeader.h"

class CGameWorld;

enum class SCENE_TYPE : unsigned char
{
	BOOT_SCENE = 0,
	INIT_SCENE,
	GAME_SCENE,
	TEST_SCENE,


	END_SCENE,
};
constexpr uint32_t SCENE_MAX_SIZE = static_cast<uint32_t>(SCENE_TYPE::END_SCENE);

class CScene
{
public:
	CScene() = default;
	virtual ~CScene() = default;

	virtual void OnCreate(CGameWorld& gameWorld);

public: // Object 라이프 사이클
	virtual void Awake() {} // next Load 단계
	virtual void Start(); // current 활성화 단계

	// Physics
	virtual void FixedUpdate(float fDelta);
	virtual void Update(float fDelta);
	virtual void LateUpdate(float fDelta);
	virtual void Build();

	// Building Frame
	virtual void CommitFrameFence();
	virtual void BuildRenderFrame() {};

public: // Object API
	OBJECT_ID AddObject(const string& strName_);
	CObject* AddAndGetObject(const string& strName_);

	void DestroyObject(const string& strName_);
	void DestroyObject(OBJECT_ID uObjectID_);

	CObject* FindObject(const string& strName_);
	CObject* FindObject(OBJECT_ID uObjectID_);

	const CObject* FindObject(const string& strName_) const;
	const CObject* FindObject(OBJECT_ID uObjectID_) const;

public:
	CObjectManager& GetObjectManager() { return m_objectManager; }
	CCamera* GetCurrentCamera() { return m_pCurrentCamera; }
	const CCamera* GetCurrentCamera() const { return m_pCurrentCamera; }

	inline CRenderWorld& GetRenderWorld() { return *m_pRenderWorld; }
	inline CAudioSystem& GetAudioSystem() { return *m_pAudioSystem; }

	inline const bool& IsRequestForChange() { return m_bChangeSceneReq; }
	inline const SCENE_TYPE& GetNextScene() { return m_eNextSceneReq; }
	inline void ResetChangeSceneRequest() { m_bChangeSceneReq = false; m_eNextSceneReq = SCENE_TYPE::END_SCENE; }

protected:
	CObjectManager m_objectManager;
	CCamera* m_pCurrentCamera = nullptr;

	CRenderWorld* m_pRenderWorld = nullptr; 
	CAudioSystem* m_pAudioSystem = nullptr;

	bool m_bChangeSceneReq = false;
	SCENE_TYPE m_eNextSceneReq = SCENE_TYPE::END_SCENE;
};