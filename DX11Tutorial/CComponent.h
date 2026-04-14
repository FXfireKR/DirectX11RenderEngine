#pragma once
#include "ObjectTypes.h"

class CObject;

enum class COMPONENT_TYPE : BYTE
{
	TRANSFORM = 0,
	CAMERA,
	MESHRENDER,

	PLAYERCONTROLLER,

	CHARACTERMOTER,
	BLOCKINTERACTOR,

	INVENTORY,

	CUSTOM_0,
	// ADD

	END,
};
const int COMPONENT_TYPE_MAX = static_cast<int>(COMPONENT_TYPE::END);

class CComponent
{
protected:
	friend class CObject;

public:
	CComponent();
	virtual ~CComponent();

	virtual void Init() {}
	virtual void Start() {}
	virtual void FixedUpdate(float fDelta) { UNREFERENCED_PARAMETER(fDelta); }
	virtual void Update(float fDelta) { UNREFERENCED_PARAMETER(fDelta); }
	virtual void LateUpdate(float fDelta) { UNREFERENCED_PARAMETER(fDelta); }
	virtual void Build() {}
	virtual void Render() {}

	virtual COMPONENT_TYPE GetType() const PURE;

public:
	inline void SetAlive(bool bAlive_) { m_bAlive = bAlive_; }
	inline bool GetAlive() const { return m_bAlive; }

	inline void SetEnable(bool bEnable_) { m_bEnable = bEnable_; }
	inline bool GetEnable() const { return m_bEnable; }

	inline void SetStarted(bool bStarted_) { m_bStarted = bStarted_; }
	inline bool GetStarted() const { return m_bStarted; }

	inline void SetOwner(CObject* pOwner_) { m_pOwner = pOwner_; }
	inline CObject* GetOwner() const { return m_pOwner; }

protected:
	CObject* m_pOwner = nullptr;

	bool m_bAlive = false;
	bool m_bEnable = false;
	bool m_bStarted = false;
};