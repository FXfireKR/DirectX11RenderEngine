#include "pch.h"
#include "CCharacterMotor.h"
#include "CWorld.h"
#include "CBlockInteractor.h"
#include "CObject.h"

void CCharacterMotor::Init()
{
	m_moveAxis = { 0.f, 0.f };
	m_bJumpRequested = false;

	m_velocity = { 0.f, 0.f, 0.f };
	m_bGrounded = false;

	m_fYaw = 0.f;

	m_fMoveSpeed = 4.5f;
	m_fJumpSpeed = 7.f;
	m_fGravity = 20.f;
	m_fMaxFallSpeed = 40.f;

	m_fInputMoveSpeedScale = 1.0f;
	m_fCruiseMoveSpeedScale = 1.0f;

	m_fHalfWidth = 0.3f;
	m_fHalfHeight = 0.9f;
}

void CCharacterMotor::Start()
{
	m_pOwnTransform = m_pOwner->GetComponent<CTransform>();
}

void CCharacterMotor::Update(float fDelta)
{
	if (nullptr == m_pOwnTransform || nullptr == m_pWorld) 
		return;

	if (m_bFrozen)
	{
		m_velocity = { 0.f, 0.f, 0.f };
		m_moveAxis = { 0.f, 0.f };
		m_bJumpRequested = false;
		m_fInputMoveSpeedScale = 1.0f;
		return;
	}

	_AppluHorizontalMove(fDelta);
	_ApplyJump();
	_ApplyGravity(fDelta);
	_MoveWithCollision(fDelta);
	_RefreshGrounded();

	m_bJumpRequested = false;
}

void CCharacterMotor::SetWorld(CWorld* pWorld)
{
	m_pWorld = pWorld;
}

void CCharacterMotor::SetMoveInput(const XMFLOAT2& axis)
{
	m_moveAxis = axis;
}

void CCharacterMotor::SetYaw(FLOAT fYawRad)
{
	m_fYaw = fYawRad;
}

void CCharacterMotor::RequestJump()
{
	m_bJumpRequested = true;
}

bool CCharacterMotor::IsGrounded()
{
	return m_bGrounded;
}

const XMFLOAT3& CCharacterMotor::GetVelocity() const
{
	return m_velocity;
}

void CCharacterMotor::GetCollisionAABB(XMFLOAT3& outCenter, XMFLOAT3& outHalfExtents) const
{
	const XMFLOAT3 footPos = m_pOwnTransform ? m_pOwnTransform->GetWorldTrans() : XMFLOAT3{};
	outCenter = { footPos.x, footPos.y + m_fHalfHeight, footPos.z };
	outHalfExtents = { m_fHalfWidth, m_fHalfHeight, m_fHalfWidth };
}

void CCharacterMotor::_AppluHorizontalMove(float fDelta)
{
	XMVECTOR forward = XMVectorSet(0.f, 0.f, 1.f, 0.f);
	XMVECTOR right = XMVectorSet(1.f, 0.f, 0.f, 0.f);
	XMMATRIX yawRot = XMMatrixRotationY(m_fYaw);

	forward = XMVector3TransformNormal(forward, yawRot);
	right = XMVector3TransformNormal(right, yawRot);

	XMVECTOR move = right * m_moveAxis.x + forward * m_moveAxis.y;
	const float lenSq = XMVectorGetX(XMVector3LengthSq(move));
	if (lenSq > 0.0001f)
		move = XMVector3Normalize(move);
	else
		move = XMVectorZero();

	XMFLOAT3 moveDir{};
	XMStoreFloat3(&moveDir, move);

	const float finalMoveSpeed = m_fMoveSpeed * m_fInputMoveSpeedScale * m_fCruiseMoveSpeedScale;

	m_velocity.x = moveDir.x * finalMoveSpeed;
	m_velocity.z = moveDir.z * finalMoveSpeed;
}

void CCharacterMotor::_ApplyJump()
{
	if (m_bJumpRequested && m_bGrounded)
	{
		m_velocity.y = m_fJumpSpeed;
		m_bGrounded = false;
	}
}

void CCharacterMotor::_ApplyGravity(float fDelta)
{
	if (!m_bGrounded)
	{
		m_velocity.y -= m_fGravity * fDelta;

		if (m_velocity.y < -m_fMaxFallSpeed)
			m_velocity.y = -m_fMaxFallSpeed;
	}
	else
	{
		if (m_velocity.y < 0.f)
			m_velocity.y = 0.f;
	}
}

void CCharacterMotor::_MoveWithCollision(float fDelta)
{
	XMFLOAT3 footPos = m_pOwnTransform->GetWorldTrans();
	const XMFLOAT3 halfExtents = { m_fHalfWidth, m_fHalfHeight, m_fHalfWidth };

	XMFLOAT3 delta = 
	{
		m_velocity.x * fDelta,
		m_velocity.y * fDelta,
		m_velocity.z * fDelta,
	};

	// X
	{
		XMFLOAT3 testFoot = footPos;
		testFoot.x += delta.x;

		const XMFLOAT3 center = _GetAABBCenterFromFootPos(testFoot);
		if (!m_pWorld->CheckAABBBlocked(center, halfExtents))
		{
			footPos.x = testFoot.x;
		}
		else
		{
			m_velocity.x = 0.f;
		}
	}

	// Y
	{
		XMFLOAT3 testFoot = footPos;	
		testFoot.y += delta.y;

		const XMFLOAT3 center = _GetAABBCenterFromFootPos(testFoot);
		if (!m_pWorld->CheckAABBBlocked(center, halfExtents))
		{
			footPos.y = testFoot.y;
		}
		else
		{
			if (m_velocity.y < 0.f)
			{
				footPos.y = _ResolveGroundSnapY(footPos, halfExtents);
				m_bGrounded = true;
			}
			m_velocity.y = 0.f;
		}
	}

	// Z
	{
		XMFLOAT3 testFoot = footPos;
		testFoot.z += delta.z;

		const XMFLOAT3 center = _GetAABBCenterFromFootPos(testFoot);
		if (!m_pWorld->CheckAABBBlocked(center, halfExtents))
		{
			footPos.z = testFoot.z;
		}
		else
		{
			m_velocity.z = 0.f;
		}
	}

	m_pOwnTransform->SetLocalTrans(footPos);
}

void CCharacterMotor::_RefreshGrounded()
{
	/*if (m_velocity.y > 0.f)
	{
		m_bGrounded = false;
		return;
	}*/

	XMFLOAT3 footPos = m_pOwnTransform->GetWorldTrans();

	const XMFLOAT3 probeHalf = { m_fHalfWidth - 0.025f, 0.05f, m_fHalfWidth - 0.025f };
	//const XMFLOAT3 probeHalf = { m_fHalfWidth * 0.45f, 0.05f, m_fHalfWidth * 0.45f };
	const XMFLOAT3 probeCenter = { footPos.x, footPos.y - 0.05f, footPos.z };

	const bool wasGrounded = m_bGrounded;
	m_bGrounded = m_pWorld->CheckAABBBlocked(probeCenter, probeHalf);

	if (!wasGrounded && m_bGrounded && m_velocity.y <= 0.f)
	{
		const XMFLOAT3 halfExtents = { m_fHalfWidth, m_fHalfHeight, m_fHalfWidth };

		footPos.y = _ResolveGroundSnapY(footPos, halfExtents);
		m_pOwnTransform->SetLocalTrans(footPos);

		if (m_velocity.y < 0.f)
			m_velocity.y = 0.f;
	}
}

float CCharacterMotor::_ResolveGroundSnapY(const XMFLOAT3& footPos, const XMFLOAT3& halfExtents) const
{
	constexpr float fMaxSnapDown = 0.1f;
	constexpr int iMaxSteps = 16;
	constexpr float fSkin = 0.001f;

	float bestY = footPos.y;

	for (int i = 1; i <= iMaxSteps; ++i)
	{
		const float t = static_cast<float>(i) / static_cast<float>(iMaxSteps);
		const float y = footPos.y - fMaxSnapDown * t;

		XMFLOAT3 testFoot = footPos;
		testFoot.y = y;

		const XMFLOAT3 center = _GetAABBCenterFromFootPos(testFoot);
		if (m_pWorld->CheckAABBBlocked(center, halfExtents))
			break;

		bestY = y;
	}

	return bestY + fSkin;
}

XMFLOAT3 CCharacterMotor::_GetAABBCenterFromFootPos(const XMFLOAT3& footPos) const
{
	return { footPos.x, footPos.y + m_fHalfHeight, footPos.z };
}
