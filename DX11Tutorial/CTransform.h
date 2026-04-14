#pragma once
#include "CComponentBase.h"

class CTransform final : public CComponentBase<CTransform, COMPONENT_TYPE::TRANSFORM>
{
public:
	CTransform() = default;
	~CTransform() override = default;

	void Init() override;
	void Start() override;
	void Build() override;

	void BuildWorldMatrix();

	void AddChild(OBJECT_ID id);
	void SetParent(OBJECT_ID id);

	void SetLocalScale(const XMFLOAT3& fScale);
	void SetLocalRotateEulerDeg(const XMFLOAT3& fRotateDeg);
	void SetLocalRotateEulerRad(const XMFLOAT3& fRotateRad);
	void SetLocalRotateQuat(const XMVECTOR& vRotate);
	void SetLocalTrans(const XMFLOAT3& fTrans);

	inline const XMFLOAT3 GetLocalScale() const { return m_fScale; }
	inline const XMVECTOR GetLocalRotationQuat() const { return XMLoadFloat4(&m_qRotate); }
	inline const XMFLOAT3 GetLocalTrans() const { return m_fTrans; }

	void RotateLocalQuat(const XMVECTOR& delta);

	void SetWorldScale(const XMFLOAT3& fScale);
	void SetWorldRotationQuat(const XMVECTOR& vRotate);
	void WorldTrans(const XMFLOAT3& fTrans);

	const XMFLOAT3 GetWorldScale() const;
	const XMVECTOR GetWorldRotationQuat() const;
	const XMFLOAT3 GetWorldTrans() const;

	const XMFLOAT3 GetRight() const;
	const XMFLOAT3 GetUp() const;
	const XMFLOAT3 GetLook() const;

	const XMFLOAT3 GetRightNorm() const;
	const XMFLOAT3 GetUpNorm() const;
	const XMFLOAT3 GetLookNorm() const;

	CTransform* GetTransform(OBJECT_ID id);
	const CTransform* GetParent() const;

	inline bool HasParent() const { return m_uParentID != INVALID_OBJECT_ID; }
	inline const XMMATRIX& GetWorldMatrix() const { return m_matWorld; }

private:
	void _MarkLocalDirty();
	void _MarkWorldDirty();

private:
	OBJECT_ID m_uParentID = INVALID_OBJECT_ID;
	vector<OBJECT_ID> m_vecChildren;

	XMFLOAT3 m_fScale = {1.f, 1.f, 1.f};
	XMFLOAT4 m_qRotate = { 0.f, 0.f, 0.f, 0.f };
	//XMVECTOR m_qRotate = XMQuaternionIdentity(); // Quaternion
	XMFLOAT3 m_fTrans = {0.f, 0.f, 0.f};
	
	XMMATRIX m_matLocal = XMMatrixIdentity();
	XMMATRIX m_matWorld = XMMatrixIdentity();

	bool m_bLocalDirty = true;
	bool m_bWorldDirty = true;
};