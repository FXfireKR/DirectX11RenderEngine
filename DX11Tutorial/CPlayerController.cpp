#include "pch.h"
#include "CPlayerController.h"
#include "CCharacterMotor.h"
#include "CBlockInteractor.h"
#include "CInventoryComponent.h"
#include "CObject.h"
#include "CTransform.h"
#include "CWorld.h"
#include "CAudioSystem.h"

#include "ChunkMath.h"

void CPlayerController::Init()
{
	CInputManager::Get().Mouse().DisalbleMove();
	
	m_fYaw = 0.f;
	m_fPitch = 0.f;
	m_fMouseSensitivity = 0.001f;
	m_fPitchLimitRad = XM_PIDIV2 - 0.05f;

	m_fCameraBaseHeight = 1.5f;

	m_fHeadBobBlend = 0.f;
	m_fHeadBobPhase = 0.f;
	m_fHeadBobAmplitude = 0.045f;
	m_fHeadBobSideAmplitude = 0.012f;
	m_fHeadBobBlendInSpeed = 10.f;
	m_fHeadBobBlendOutSpeed = 14.f;

	m_fStepStrideMeters = 1.75f;
	m_fStepDistanceAccum = 0.f;

	m_prevFootPos = {};
	m_bHasPrevFootPos = false;

	m_bPrevGrounded = false;
	m_fPrevVelocityY = 0.f;

	m_fLandingMinFallSpeed = 3.0f;
	m_fLandingVolumeScale = 0.45f;
}

void CPlayerController::Start()
{
	m_pOwnTransform = m_pOwner->GetComponent<CTransform>();
	m_pMotor = m_pOwner->GetComponent<CCharacterMotor>();
	m_pBlockInteractor = m_pOwner->GetComponent<CBlockInteractor>();
	m_pInventory = m_pOwner->GetComponent<CInventoryComponent>();

	if (m_pCamera)
	{
		m_fBaseFov = m_pCamera->GetPerspectiveParams().fFieldOfView;
		m_fCurrentFov = m_fBaseFov;
	}
}

void CPlayerController::Update(float fDelta)
{
	if (nullptr == m_pOwnTransform || nullptr == m_pCamTransform) 
		return;

	_UpdateMouseLockToggle();

#ifdef IMGUI_ACTIVATE
	ImGuiIO& io = ImGui::GetIO();
	const bool blockMouse = m_bUIMode || io.WantCaptureMouse;
	const bool blockKeyboard = m_bUIMode || io.WantCaptureKeyboard;
#else // IMGUI_ACTIVATE
	const bool blockMouse = m_bUIMode;
	const bool blockKeyboard = m_bUIMode;
#endif // IMGUI_ACTIVATE

	if (!(blockMouse || blockKeyboard))
	{
		PlayerInputCommand command{};
		_BuildInputCommand(fDelta, command);
		_ApplyInputCommand(command);
		_UpdateHeadBobAndStep(fDelta);
	}
	else
	{
		m_pMotor->SetMoveInput({ 0.f, 0.f });
		m_pMotor->SetInputMoveSpeedScale(1.0f);

		if (m_pBlockInteractor)
			m_pBlockInteractor->SetBreakHeld(false);
	}

	_UpdateMoveFov(fDelta);

	XMFLOAT3 pos = m_pOwnTransform->GetWorldTrans();
	dbg.SetPlayerPosition(pos);

	XMINT3 chunkCoord{};
	chunkCoord.x = ChunkMath::FloorDiv16((int)std::floor(pos.x));
	chunkCoord.y = ChunkMath::FloorDiv16((int)std::floor(pos.y));
	chunkCoord.z = ChunkMath::FloorDiv16((int)std::floor(pos.z));
	dbg.SetCurrentChunkCoord(chunkCoord);
}

void CPlayerController::_UpdateMouseLockToggle()
{
	if (!CInputManager::Get().Keyboard().GetKeyUp(VK_TAB))
		return;

	m_bUIMode = !m_bUIMode;

	CMouseDevice& mouse = CInputManager::Get().Mouse();
	if (m_bUIMode)
		mouse.EnalbleMove();
	else
		mouse.DisalbleMove();
}

void CPlayerController::_BuildInputCommand(float fDelta, PlayerInputCommand& outCmd) const
{
	CInputManager& input = CInputManager::Get();
	const bool bUseGamePad = input.IsGamePadMode();

	if (bUseGamePad)
	{
		const CDualSenseDevice* pPad = input.GamePad().GetActivateDualSense();
		if (pPad)
		{
			outCmd.moveX = pPad->GetLX();
			outCmd.moveY = -pPad->GetLY();

			outCmd.lookX = pPad->GetRX() * m_fPadLookSpeed * fDelta;
			outCmd.lookY = pPad->GetRY() * m_fPadLookSpeed * fDelta;

			outCmd.jumpPressed = pPad->GetButtonDown(DUALSENSE_BUTTON::CROSS);
			outCmd.breakHeld = pPad->GetButton(DUALSENSE_BUTTON::L2) || (pPad->GetL2Axis() > 0.35f);
			outCmd.placePressed = pPad->GetButtonDown(DUALSENSE_BUTTON::R2) || (pPad->GetR2Axis() > 0.5f);

			outCmd.hotbarPrev = pPad->GetButtonDown(DUALSENSE_BUTTON::L1);
			outCmd.hotbarNext = pPad->GetButtonDown(DUALSENSE_BUTTON::R1);
			outCmd.sprintHeld = pPad->GetButton(DUALSENSE_BUTTON::LSTICK);
			outCmd.switchControl = pPad->GetButtonDown(DUALSENSE_BUTTON::SHARE);
		}
	}
	else // (!bUseGamePad)
	{
		const POINT& delta = input.Mouse().GetDelta();
		outCmd.lookX = static_cast<float>(delta.x) * m_fMouseSensitivity;
		outCmd.lookY = static_cast<float>(delta.y) * m_fMouseSensitivity;

		if (input.Keyboard().GetKey('W')) outCmd.moveY += 1.f;
		if (input.Keyboard().GetKey('S')) outCmd.moveY -= 1.f;
		if (input.Keyboard().GetKey('D')) outCmd.moveX += 1.f;
		if (input.Keyboard().GetKey('A')) outCmd.moveX -= 1.f;

		outCmd.jumpPressed = input.Keyboard().GetKey(VK_SPACE);
		outCmd.breakHeld = input.Mouse().GetKey(VK_LBUTTON);
		outCmd.placePressed = input.Mouse().GetKey(VK_RBUTTON);
		outCmd.sprintHeld = input.Keyboard().GetKey(VK_SHIFT);
		outCmd.switchControl = input.Keyboard().GetKeyUp(VK_HOME);

		const short wheel = input.Mouse().GetWheelCnt();
		const short dir = input.Mouse().GetWheelDir();
		if (wheel != 0)
		{
			if (dir > 0)
				outCmd.hotbarNext = true;
			else
				outCmd.hotbarPrev = true;
		}
	}
}

void CPlayerController::_ApplyInputCommand(const PlayerInputCommand& cmd)
{
	m_fYaw += cmd.lookX;
	m_fPitch += cmd.lookY;
	m_fPitch = std::clamp(m_fPitch, -m_fPitchLimitRad, m_fPitchLimitRad);

	m_pOwnTransform->SetLocalRotateEulerRad({ 0.f, m_fYaw, 0.f });
	m_pCamTransform->SetLocalRotateEulerRad({ m_fPitch, 0.f, 0.f });
	m_pMotor->SetYaw(m_fYaw);

	const float sprintScale = cmd.sprintHeld ? 1.75f : 1.0f;
	m_pMotor->SetInputMoveSpeedScale(sprintScale);

	XMFLOAT2 moveAxis{ cmd.moveX, cmd.moveY };
	moveAxis.x = std::clamp(moveAxis.x, -1.f, 1.f);
	moveAxis.y = std::clamp(moveAxis.y, -1.f, 1.f);
	m_pMotor->SetMoveInput(moveAxis);

	if (cmd.jumpPressed)
		m_pMotor->RequestJump();

	m_pBlockInteractor->SetBreakHeld(cmd.breakHeld);
	if (cmd.placePressed)
		m_pBlockInteractor->RequestPlace();

	int step = 0;
	if (cmd.hotbarPrev) --step;
	if (cmd.hotbarNext) ++step;

	if (step != 0)
	{
		int idx = m_pInventory->GetSelectedSlotIndex();
		idx += step;
		idx %= 9;
		if (idx < 0) 
			idx += 9;
		m_pInventory->SetSelectedSlotIndex(idx);
	}

	if (cmd.switchControl)
	{
		if (EActiveInputDevice::KEYBOARD_MOUSE == CInputManager::Get().GetActiveInputDevice())
		{
			if (nullptr != CInputManager::Get().GamePad().GetActivateDualSense())
				CInputManager::Get().SetActiveInputDevice(EActiveInputDevice::GAMEPAD);
		}
		else if (EActiveInputDevice::GAMEPAD == CInputManager::Get().GetActiveInputDevice())
			CInputManager::Get().SetActiveInputDevice(EActiveInputDevice::KEYBOARD_MOUSE);
	}
}

void CPlayerController::_UpdateHeadBobAndStep(float fDelta)
{
	if (!m_pMotor || !m_pCamTransform)
		return;

	const XMFLOAT3 footPos = m_pOwnTransform->GetWorldTrans();

	if (!m_bHasPrevFootPos)
	{
		m_prevFootPos = footPos;
		m_bHasPrevFootPos = true;
	}

	float dx = footPos.x - m_prevFootPos.x;
	float dz = footPos.z - m_prevFootPos.z;
	float movedDistXZ = std::sqrt(dx * dx + dz * dz);

	// 순간이동/스폰 보정
	if (movedDistXZ > 2.0f)
	{
		m_prevFootPos = footPos;
		movedDistXZ = 0.0f;
		m_fStepDistanceAccum = 0.0f;
	}

	m_prevFootPos = footPos;

	const XMFLOAT3 vel = m_pMotor->GetVelocity();
	const float planarSpeed = std::sqrt(vel.x * vel.x + vel.z * vel.z);
	const bool bGrounded = m_pMotor->IsGrounded();
	const bool bMoving = (bGrounded && planarSpeed > 0.1f && movedDistXZ > 0.0001f);
	const bool bJustLanded = (!m_bPrevGrounded && bGrounded && m_fPrevVelocityY < -m_fLandingMinFallSpeed);

	if (bJustLanded)
	{
		BlockCell footCell{};
		if (_ResolveFootstepBlock(footPos, footCell))
		{
			if (!footCell.IsAir() && m_pAudio)
			{
				ResolvedSound resolved{};
				if (BlockResDB.ResolveBlock(footCell.blockID, EBlockSoundUsage::FALL, resolved))
				{
					const float fallSpeed = std::min(-m_fPrevVelocityY, 12.0f);
					const float t = (fallSpeed - m_fLandingMinFallSpeed) / (12.0f - m_fLandingMinFallSpeed);
					const float volumeMul = std::clamp(t, 0.0f, 1.0f);

					const XMFLOAT3 soundPos =
					{
						footPos.x,
						footPos.y - 0.8f,
						footPos.z
					};

					m_pAudio->Submit3D(
						resolved.soundID,
						soundPos,
						resolved.playDesc.bus,
						resolved.playDesc.volume * m_fLandingVolumeScale * (0.6f + 0.4f * volumeMul),
						resolved.playDesc.pitch,
						resolved.playDesc.minDistance,
						resolved.playDesc.maxDistance
					);
				}
			}
		}
	}

	const float blendTarget = bMoving ? 1.0f : 0.0f;
	const float blendSpeed = bMoving ? m_fHeadBobBlendInSpeed : m_fHeadBobBlendOutSpeed;
	m_fHeadBobBlend = _Approach(m_fHeadBobBlend, blendTarget, blendSpeed * fDelta);

	if (bJustLanded)
	{
		m_fHeadBobBlend = std::min(m_fHeadBobBlend, 0.35f);
	}

	if (bMoving)
	{
		const float phaseAdvance = movedDistXZ * (XM_2PI / m_fStepStrideMeters);
		m_fHeadBobPhase += phaseAdvance;
		m_fStepDistanceAccum += movedDistXZ;

		while (m_fStepDistanceAccum >= m_fStepStrideMeters)
		{
			m_fStepDistanceAccum -= m_fStepStrideMeters;

			BlockCell footCell{};
			if (_ResolveFootstepBlock(footPos, footCell))
			{
				_PlayFootstep(footPos, footCell);
			}
		}
	}

	const float bobPulse = 0.5f - std::cos(m_fHeadBobPhase);
	const float bobSide = std::sin(m_fHeadBobPhase);

	const float offsetY = -(bobPulse * m_fHeadBobAmplitude * m_fHeadBobBlend * m_fHeadBobIntensity);
	const float offsetX = (bobSide * m_fHeadBobSideAmplitude * m_fHeadBobBlend * m_fHeadBobIntensity);

	m_pCamTransform->SetLocalTrans({
		offsetX,
		m_fCameraBaseHeight + offsetY,
		0.0f
	});

	m_bPrevGrounded = bGrounded;
	m_fPrevVelocityY = vel.y;
}

bool CPlayerController::_ResolveFootstepBlock(const XMFLOAT3& footPos, BlockCell& outCell) const
{
	outCell = {};

	if (!m_pWorld)
		return false;

	const int wx = static_cast<int>(std::floor(footPos.x));
	const int wz = static_cast<int>(std::floor(footPos.z));

	// 발 바로 아래
	int wy = static_cast<int>(std::floor(footPos.y - 0.05f));
	outCell = m_pWorld->GetBlockCell(wx, wy, wz);
	if (!outCell.IsAir())
		return true;

	// 혹시 skin/보정 때문에 비면 한 칸 더 아래 fallback
	outCell = m_pWorld->GetBlockCell(wx, wy - 1, wz);
	if (!outCell.IsAir())
		return true;

	return false;
}

void CPlayerController::_PlayFootstep(const XMFLOAT3& footPos, const BlockCell& cell)
{
	if (!m_pAudio)
		return;

	if (cell.IsAir())
		return;

	ResolvedSound resolved{};
	if (!BlockResDB.ResolveBlock(cell.blockID, EBlockSoundUsage::STEP, resolved))
		return;

	const float t = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	const float pitchMul = 0.98f + (0.04f * t); // 0.98 ~ 1.02

	const XMFLOAT3 soundPos =
	{
		footPos.x,
		footPos.y - 0.8f,
		footPos.z
	};

	m_pAudio->Submit3D(
		resolved.soundID,
		soundPos,
		resolved.playDesc.bus,
		resolved.playDesc.volume * 0.35f,
		resolved.playDesc.pitch * pitchMul,
		resolved.playDesc.minDistance,
		resolved.playDesc.maxDistance
	);
}

float CPlayerController::_Approach(float cur, float target, float delta)
{
	if (cur < target) return std::min(cur + delta, target);
	return std::max(cur - delta, target);
}

void CPlayerController::_UpdateMoveFov(float fDelta)
{
	if (!m_pCamera || !m_pMotor)
		return;

	const XMFLOAT3 vel = m_pMotor->GetVelocity();
	const float planarSpeed = std::sqrt(vel.x * vel.x + vel.z * vel.z);

	float targetFov = m_fBaseFov;

	// base 4.5, sprint 약 7.8, cruise는 그보다 훨씬 큼
	if (!m_bUIMode && m_pMotor->IsGrounded())
	{
		if (planarSpeed > 12.0f)
		{
			targetFov += m_fCruiseFovAddRad;
		}
		else if (planarSpeed > 5.2f)
		{
			targetFov += m_fSprintFovAddRad;
		}
	}

	m_fCurrentFov = _Approach(m_fCurrentFov, targetFov, m_fFovApproachSpeedRad * fDelta);

	const float curCameraFov = m_pCamera->GetPerspectiveParams().fFieldOfView;
	if (std::fabs(curCameraFov - m_fCurrentFov) > 0.0001f)
	{
		m_pCamera->SetFov(m_fCurrentFov);
	}
}