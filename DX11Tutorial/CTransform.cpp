#include "pch.h"
#include "CTransform.h"
#include "CObject.h"
#include "CScene.h"

void CTransform::Init()
{
	m_uParentID = INVALID_OBJECT_ID;

	m_fScale = { 1.f, 1.f, 1.f };
	XMStoreFloat4(&m_qRotate, XMQuaternionIdentity());
	m_fTrans = { 0.f, 0.f, 0.f };

	m_matLocal = XMMatrixIdentity();
	m_matWorld = XMMatrixIdentity();

	m_bLocalDirty = true;
	m_bWorldDirty = true;
}

void CTransform::Start()
{
	if (m_pOwner->HasParent())
		m_uParentID = m_pOwner->GetParentID();
}

void CTransform::Build()
{
	BuildWorldMatrix();
}

void CTransform::BuildWorldMatrix()
{
	if (m_bLocalDirty) 
	{
		XMMATRIX matScale = XMMatrixScaling(m_fScale.x, m_fScale.y, m_fScale.z);
		XMMATRIX matRotate = XMMatrixRotationQuaternion(XMLoadFloat4(&m_qRotate));
		XMMATRIX matTranslation = XMMatrixTranslation(m_fTrans.x, m_fTrans.y, m_fTrans.z);

		m_matLocal = matScale * matRotate * matTranslation;

		m_bLocalDirty = false;
		m_bWorldDirty = true;
	}

	if (m_bWorldDirty) 
	{
		if (HasParent()) 
		{
			CTransform* parent = const_cast<CTransform*>(GetParent());
			parent->BuildWorldMatrix();

			m_matWorld = m_matLocal * GetParent()->GetWorldMatrix();
		}
		else
			m_matWorld = m_matLocal;

		m_bWorldDirty = false;
	}
}

void CTransform::AddChild(OBJECT_ID uChildID_)
{
	if (m_vecChildren.end() == std::find(m_vecChildren.begin(), m_vecChildren.end(), uChildID_)) {
		m_vecChildren.push_back(uChildID_);
	}
}

void CTransform::SetParent(OBJECT_ID id)
{
	if (m_uParentID == id) return;

	if (HasParent()) {
		CTransform* oldParent = const_cast<CTransform*>(GetParent());
		auto& children = oldParent->m_vecChildren;
		children.erase(std::remove(children.begin(), children.end(), m_pOwner->GetID()), children.end());
	}

	m_uParentID = id;

	if (HasParent())
	{
		CTransform* newParent = const_cast<CTransform*>(GetParent());
		newParent->AddChild(m_pOwner->GetID());
	}

	_MarkWorldDirty();
}

void CTransform::SetLocalScale(const XMFLOAT3& fScale)
{
	m_fScale = fScale;
	_MarkLocalDirty();
}

void CTransform::SetLocalRotateEulerDeg(const XMFLOAT3& fRotateDeg)
{
	XMFLOAT3 rad = {
		XMConvertToRadians(fRotateDeg.x),
		XMConvertToRadians(fRotateDeg.y),
		XMConvertToRadians(fRotateDeg.z)
	};

	XMVECTOR rotate = XMQuaternionNormalize(XMQuaternionRotationRollPitchYaw(rad.x, rad.y, rad.z));
	XMStoreFloat4(&m_qRotate, rotate);
	_MarkLocalDirty();
}

void CTransform::SetLocalRotateEulerRad(const XMFLOAT3& fRotateRad)
{
	XMVECTOR rotate = XMQuaternionNormalize(XMQuaternionRotationRollPitchYaw(fRotateRad.x, fRotateRad.y, fRotateRad.z));
	XMStoreFloat4(&m_qRotate, rotate);
	_MarkLocalDirty();
}

void CTransform::SetLocalRotateQuat(const XMVECTOR& vRotate)
{
	XMVECTOR rotate = XMQuaternionNormalize(vRotate);
	XMStoreFloat4(&m_qRotate, rotate);
	_MarkLocalDirty();
}

void CTransform::SetLocalTrans(const XMFLOAT3& fTrans)
{
	m_fTrans = fTrans;
	_MarkLocalDirty();
}

void CTransform::RotateLocalQuat(const XMVECTOR& delta)
{
	XMVECTOR rotate = XMLoadFloat4(&m_qRotate);
	rotate = XMQuaternionNormalize(XMQuaternionMultiply(rotate, delta));
	XMStoreFloat4(&m_qRotate, rotate);
	_MarkLocalDirty();
}

void CTransform::SetWorldScale(const XMFLOAT3& fScale)
{
	XMFLOAT3 newScale = fScale;
	if (HasParent()) {
		XMVECTOR s, r, t;
		XMMatrixDecompose(&s, &r, &t, GetParent()->GetWorldMatrix());

		XMFLOAT3 parentScale;
		XMStoreFloat3(&parentScale, s);

		newScale.x /= parentScale.x;
		newScale.y /= parentScale.y;
		newScale.z /= parentScale.z;
	}

	SetLocalScale(newScale);
}

void CTransform::SetWorldRotationQuat(const XMVECTOR& vRotate)
{
	XMVECTOR newRotate = vRotate;
	if (HasParent()) {
		XMVECTOR parentWorldRotate = GetParent()->GetWorldRotationQuat();
		XMVECTOR parentInv = XMQuaternionInverse(parentWorldRotate);
		newRotate = XMQuaternionMultiply(parentInv, vRotate);
	}
	SetLocalRotateQuat(newRotate);
}

void CTransform::WorldTrans(const XMFLOAT3& fTrans)
{
	XMFLOAT3 newTrans = fTrans;
	if (HasParent()) {
		XMMATRIX matParentWorldInverse = XMMatrixInverse(nullptr, GetParent()->GetWorldMatrix());
		XMStoreFloat3(&newTrans, XMVector3TransformCoord(XMLoadFloat3(&fTrans), matParentWorldInverse));
	}

	SetLocalTrans(newTrans);
}

const XMFLOAT3 CTransform::GetWorldScale() const
{
	XMVECTOR s, r, t;
	XMMatrixDecompose(&s, &r, &t, m_matWorld);

	XMFLOAT3 scale;
	XMStoreFloat3(&scale, s);
	return scale;
}

const XMVECTOR CTransform::GetWorldRotationQuat() const
{
	XMVECTOR s, r, t;
	XMMatrixDecompose(&s, &r, &t, m_matWorld);
	return r;
}

const XMFLOAT3 CTransform::GetWorldTrans() const
{
	return XMFLOAT3(
		m_matWorld.r[3].m128_f32[0],
		m_matWorld.r[3].m128_f32[1],
		m_matWorld.r[3].m128_f32[2]);
}

const XMFLOAT3 CTransform::GetRight() const
{
	return XMFLOAT3(
		m_matWorld.r[0].m128_f32[0],
		m_matWorld.r[0].m128_f32[1],
		m_matWorld.r[0].m128_f32[2]);
}

const XMFLOAT3 CTransform::GetUp() const
{
	return XMFLOAT3(
		m_matWorld.r[1].m128_f32[0],
		m_matWorld.r[1].m128_f32[1],
		m_matWorld.r[1].m128_f32[2]);
}

const XMFLOAT3 CTransform::GetLook() const
{
	return XMFLOAT3(
		m_matWorld.r[2].m128_f32[0],
		m_matWorld.r[2].m128_f32[1],
		m_matWorld.r[2].m128_f32[2]);
}

const XMFLOAT3 CTransform::GetRightNorm() const
{
	XMVECTOR right = m_matWorld.r[0];
	right = XMVector3Normalize(right);

	XMFLOAT3 fRight;
	XMStoreFloat3(&fRight, right);
	return fRight;
}

const XMFLOAT3 CTransform::GetUpNorm() const
{
	XMVECTOR up = m_matWorld.r[1];
	up = XMVector3Normalize(up);

	XMFLOAT3 fUp;
	XMStoreFloat3(&fUp, up);
	return fUp;
}

const XMFLOAT3 CTransform::GetLookNorm() const
{
	XMVECTOR look = m_matWorld.r[2];
	look = XMVector3Normalize(look);

	XMFLOAT3 fLook;
	XMStoreFloat3(&fLook, look);
	return fLook;
}

CTransform* CTransform::GetTransform(OBJECT_ID id)
{
	if (false == IsValidObjectID(id)) return nullptr;

	CObject* pObject = m_pOwner->GetOwnScene()->FindObject(id);
	if (nullptr == pObject) return nullptr;
	if (nullptr == pObject->GetComponent<CTransform>()) return nullptr;

	return pObject->GetComponent<CTransform>();
}

const CTransform* CTransform::GetParent() const
{
	if (false == IsValidObjectID(m_uParentID)) return nullptr;

	const CObject* pParentObject = m_pOwner->GetOwnScene()->FindObject(m_uParentID);
	if (nullptr == pParentObject) return nullptr;
	if (nullptr == pParentObject->GetComponent<CTransform>()) return nullptr;

	return pParentObject->GetComponent<CTransform>();
}

void CTransform::_MarkLocalDirty()
{
	m_bLocalDirty = true;
	_MarkWorldDirty();
}

void CTransform::_MarkWorldDirty()
{
	m_bWorldDirty = true;

	for (auto childID : m_vecChildren) {
		CTransform* child = GetTransform(childID);
		if (!child) continue;

		child->_MarkWorldDirty();
	}
}