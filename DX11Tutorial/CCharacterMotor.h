#pragma once
#include "CComponentBase.h"

class CTransform;
class CWorld;

class CCharacterMotor : public CComponentBase<CCharacterMotor, COMPONENT_TYPE::CHARACTERMOTER>
{
public:
	void Init() override;
	void Start() override;
	void Update(float fDelta) override;

public:
	void SetWorld(CWorld* pWorld);
	void SetMoveInput(const XMFLOAT2& axis);
	void SetYaw(FLOAT fYawRad);
	void RequestJump();
	
	bool IsGrounded();
	const XMFLOAT3& GetVelocity() const;

	void GetCollisionAABB(XMFLOAT3& outCenter, XMFLOAT3& outHalfExtents) const;

	inline void SetFrozen(bool bFrozen) { m_bFrozen = bFrozen; }
	inline bool IsFrozen() const { return m_bFrozen; }

	inline void SetInputMoveSpeedScale(float scale) { m_fInputMoveSpeedScale = std::max(0.0f, scale); }
	inline void SetCruiseMoveSpeedScale(float scale) { m_fCruiseMoveSpeedScale = std::max(0.0f, scale); }

private:
	void _AppluHorizontalMove(float fDelta);
	void _ApplyJump();
	void _ApplyGravity(float fDelta);
	void _MoveWithCollision(float fDelta);
	void _RefreshGrounded();
	float _ResolveGroundSnapY(const XMFLOAT3& footPos, const XMFLOAT3& halfExtents) const;

	XMFLOAT3 _GetAABBCenterFromFootPos(const XMFLOAT3& footPos) const;

private:
	CTransform* m_pOwnTransform = nullptr;
	CWorld* m_pWorld = nullptr;

	XMFLOAT2 m_moveAxis{ 0.f, 0.f };
	bool m_bJumpRequested = false;

	XMFLOAT3 m_velocity{ 0.f, 0.f, 0.f };
	bool m_bGrounded = false;
	bool m_bFrozen = false;

	float m_fYaw = 0.f;

	float m_fMoveSpeed = 60.f;
	float m_fJumpSpeed = 7.f;
	float m_fGravity = 20.f;
	float m_fMaxFallSpeed = 40.f;

	float m_fHalfWidth = 0.3f;
	float m_fHalfHeight = 0.9f;
	
	float m_fInputMoveSpeedScale = 1.0f;
	float m_fCruiseMoveSpeedScale = 1.0f;
};