#pragma once
#include "CComponentBase.h"
#include "CBlockInteractor.h"
#include "CInventoryComponent.h"

class CCharacterMotor;
class CBlockInteractor;
class CInventoryComponent;
class CTransform;
class CWorld;
class CAudioSystem;
class CCamera;

class CPlayerController : public CComponentBase<CPlayerController, COMPONENT_TYPE::PLAYERCONTROLLER>
{
private:
	struct PlayerInputCommand
	{
		float moveX = 0.f;
		float moveY = 0.f;
		float lookX = 0.f;
		float lookY = 0.f;

		bool jumpPressed = false;
		bool breakHeld = false;
		bool placePressed = false;
		bool hotbarPrev = false;
		bool hotbarNext = false;

		bool sprintHeld = false;
		bool switchControl = false;
	};

public:
	CPlayerController() = default;
	~CPlayerController() override = default;

	void Init() override;
	void Start() override;
	void Update(float fDelta) override;

public:
	inline void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }
	inline void SetCameraTransform(CTransform* pTransform) { m_pCamTransform = pTransform; }
	inline void SetWorld(CWorld* pWorld) { m_pWorld = pWorld; }
	inline void SetAudioSystem(CAudioSystem* pAudio) { m_pAudio = pAudio; }

private:
	void _UpdateMouseLockToggle();
	void _BuildInputCommand(float fDelta, PlayerInputCommand& outCommand) const;
	void _ApplyInputCommand(const PlayerInputCommand& command);

	void _UpdateHeadBobAndStep(float fDelta);
	bool _ResolveFootstepBlock(const XMFLOAT3& footPos, BlockCell& outCell) const;
	void _PlayFootstep(const XMFLOAT3& footPos, const BlockCell& cell);
	float _Approach(float cur, float target, float delta);

	void _UpdateMoveFov(float fDelta);

private:
	CCamera* m_pCamera = nullptr;
	CTransform* m_pOwnTransform = nullptr;
	CTransform* m_pCamTransform = nullptr;

	CCharacterMotor* m_pMotor = nullptr;
	CBlockInteractor* m_pBlockInteractor = nullptr;
	CInventoryComponent* m_pInventory = nullptr;
	CWorld* m_pWorld = nullptr;
	CAudioSystem* m_pAudio = nullptr;

	float m_fYaw = 0.f;
	float m_fPitch = 0.f;

	float m_fMouseSensitivity = 0.01f;
	float m_fPitchLimitRad = XM_PIDIV2 - 0.05f;
	float m_fPadLookSpeed = 2.4f;

	// presentation
	float m_fCameraBaseHeight = 1.5f;

	float m_fHeadBobBlend = 0.f;
	float m_fHeadBobPhase = 0.f;
	float m_fHeadBobAmplitude = 0.045f;
	float m_fHeadBobSideAmplitude = 0.012f;
	float m_fHeadBobBlendInSpeed = 10.f;
	float m_fHeadBobBlendOutSpeed = 14.f;
	float m_fHeadBobIntensity = 1.f;

	float m_fStepStrideMeters = 1.75f;
	float m_fStepDistanceAccum = 0.f;

	XMFLOAT3 m_prevFootPos{};
	bool m_bHasPrevFootPos = false;

	bool m_bPrevGrounded = false;
	float m_fPrevVelocityY = 0.f;

	float m_fLandingMinFallSpeed = 3.0f;
	float m_fLandingVolumeScale = 0.45f;

	bool m_bUIMode = false;

	// Fov maker
	float m_fBaseFov = XM_PI / 2.0f;
	float m_fCurrentFov = XM_PI / 2.0f;

	float m_fSprintFovAddRad = XMConvertToRadians(6.0f);
	float m_fCruiseFovAddRad = XMConvertToRadians(10.0f);
	float m_fFovApproachSpeedRad = XMConvertToRadians(90.0f);
};