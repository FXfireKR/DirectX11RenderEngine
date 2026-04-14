#include "pch.h"
#include "CCamera.h"
#include "CTransform.h"
#include "CObject.h"
#include "CScene.h"

void CCamera::Init()
{
	m_eProjectionType = CAMERA_PROJECTION_TYPE::PERSPECTIVE;

	m_matView = XMMatrixIdentity();
	m_matProjection = XMMatrixIdentity();

	m_bDirty = true;

	m_kPerspective = {};
	m_kOrthographic = {};
}

void CCamera::Start()
{
	m_pOwnTransform = m_pOwner->GetComponent<CTransform>();
}

void CCamera::Build()
{
	UpdateCameraMatrix();
}

void CCamera::UpdateCameraMatrix()
{
	_UpdateViewMatrix();
	_UpdateProjectionMatrix();
}

void CCamera::SetProjToPerspective()
{
	if (CAMERA_PROJECTION_TYPE::PERSPECTIVE == m_eProjectionType)
		return;

	m_eProjectionType = CAMERA_PROJECTION_TYPE::PERSPECTIVE;
	m_bDirty = true;
}


void CCamera::SetProjToOrthographic()
{
	if (CAMERA_PROJECTION_TYPE::ORTHOGRAPHIC == m_eProjectionType)
		return;

	m_eProjectionType = CAMERA_PROJECTION_TYPE::ORTHOGRAPHIC;
	m_bDirty = true;
}

void CCamera::SetOrthographicSize(float width, float height)
{
	m_kOrthographic.fWidth = width;
	m_kOrthographic.fHeight = height;

	if (CAMERA_PROJECTION_TYPE::ORTHOGRAPHIC == m_eProjectionType)
		m_bDirty = true;
}

void CCamera::SetOrthographicNearFar(float nearZ, float farZ)
{
	m_kOrthographic.fNearZ = nearZ;
	m_kOrthographic.fFarZ = farZ;

	if (CAMERA_PROJECTION_TYPE::ORTHOGRAPHIC == m_eProjectionType)
		m_bDirty = true;
}

void CCamera::SetFov(float newFov)
{
	m_kPerspective.fFieldOfView = newFov;
	m_bDirty = true;
}

void CCamera::SetAspectRatio(float newRatio)
{
	m_kPerspective.fAspectRatio = newRatio;
	m_bDirty = true;
}

void CCamera::SetProjectionType(const CAMERA_PROJECTION_TYPE& eProjectionType_)
{
	m_eProjectionType = eProjectionType_;
	m_bDirty = true;
}

const CTransform* CCamera::GetTransform() const
{
	return m_pOwnTransform;
}

void CCamera::_UpdateViewMatrix()
{
	if (nullptr == m_pOwnTransform) return;

	const auto& world = m_pOwnTransform->GetWorldMatrix();

	XMVECTOR eye = world.r[3];
	// look 이 normalize되면 한 좌표에 고정되어 보므로, eye를 더해줘야 움직이는 위치에 맞게 보인다.
	XMVECTOR look = eye + XMVector3Normalize(world.r[2]);
	XMVECTOR up = XMVector3Normalize(world.r[1]);

	// view-matrix
	m_matView = XMMatrixLookAtLH(eye, look, up);
}

void CCamera::_UpdateProjectionMatrix()
{
	if (m_bDirty) 
	{
		if (CAMERA_PROJECTION_TYPE::PERSPECTIVE == m_eProjectionType)
		{
			m_matProjection = XMMatrixPerspectiveFovLH
			(
				m_kPerspective.fFieldOfView,
				m_kPerspective.fAspectRatio,
				m_kPerspective.fNearZ,
				m_kPerspective.fFarZ
			);
		}
		else
		{
			m_matProjection = XMMatrixOrthographicLH
			(
				m_kOrthographic.fWidth,
				m_kOrthographic.fHeight,
				m_kOrthographic.fNearZ,
				m_kOrthographic.fFarZ
			);
		}

		m_bDirty = false;
	}
}