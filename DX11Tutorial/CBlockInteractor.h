#pragma once
#include "CComponentBase.h"
#include "BlockRaycastTypes.h"
#include "CBlockBreakParticleSystem.h"

class CWorld;
class CInventoryComponent;
class CCharacterMotor;
class CTransform;
class CObject;
class CAudioSystem;

class CBlockInteractor : public CComponentBase<CBlockInteractor, COMPONENT_TYPE::BLOCKINTERACTOR>
{
public:
	void Init() override;
	void Start() override;
	void LateUpdate(float fDelta) override;

public:
	void SetBreakHeld(bool bHeld);

	void RequestPlace();
	void RequestBreak();

public:
	inline void SetWorld(CWorld* pWorld) { m_pWorld = pWorld; }
	inline void SetCameraTransform(CTransform* pCamTransform) { m_pCamTransform = pCamTransform; }
	inline void SetHighlightObject(CObject* pHighlightObject) { m_pHighlightObject = pHighlightObject; }
	inline void SetParticleSystem(CBlockBreakParticleSystem* pParticleSystem) { m_pParticle = pParticleSystem; }
	inline void SetAudioSystem(CAudioSystem* pAudioSystem) { m_pAudio = pAudioSystem; }

	inline const BlockHitResult& GetBlockHit() const { return m_hitResult; }

	inline bool IsMining() const { return m_bMining; }
	inline const XMINT3& GetMiningBlock() const { return m_miningBlock; }
	inline const BlockCell& GetMiningCell() const { return m_miningCell; }

	inline float GetBreakProgress01() const
	{
		if (!m_bMining || m_fBreakRequired <= 0.f)
			return 0.f;

		return std::clamp(m_fBreakAccum / m_fBreakRequired, 0.f, 1.f);
	}

private:
	void _UpdateRaycast();
	void _UpdateHighlight();
	void _ConsumeRequests();

	void _UpdateMining(float fDelta);
	void _ResetMining();
	bool _IsSameMiningTarget(const BlockHitResult& hit) const;
	float _CalcBreakRequired(const BlockCell& cell) const;

	bool _TryPlaceBlock();
	bool _TryBreakBlock();
	bool _OverlapAABBForPlacement(const XMFLOAT3& aCenter, const XMFLOAT3& aHalf
		, const XMFLOAT3& bCenter, const XMFLOAT3& bHalf);
	bool _OverlapAABB(const XMFLOAT3& aCenter, const XMFLOAT3& aHalf
		, const XMFLOAT3& bCenter, const XMFLOAT3& bHalf);

	void _MakeCenterRay(OUT XMFLOAT3& outOrigin, XMFLOAT3& outDir) const;

	void _PlayBlockSound(const BlockCell& cell, EBlockSoundUsage usage, const XMINT3& blockPos, float pitchJitter = 0.f);
	float _RandomPitch(float basePitch, float jitter) const;

private:
	CWorld* m_pWorld = nullptr;
	CInventoryComponent* m_pInventory = nullptr;
	CCharacterMotor* m_pMotor = nullptr;
	CTransform* m_pCamTransform = nullptr;
	CObject* m_pHighlightObject = nullptr;
	CBlockBreakParticleSystem* m_pParticle = nullptr;
	CAudioSystem* m_pAudio = nullptr;

private:
	BlockHitResult m_hitResult{};
	float m_fReach = 7.f;

	bool m_bBreakHeld = false;
	bool m_bMining = false;
	XMINT3 m_miningBlock{};
	XMINT3 m_miningNormal{};
	BlockCell m_miningCell{};
	float m_fBreakAccum = 0.f;
	float m_fBreakRequired = 0.25f;

	const float HIT_FX_COOL = 0.225f;
	float m_fHitFxCoolDown = 0.f;
	
	bool m_bPlaceRequested = false;
	float m_fPlaceCoolDown = 0.f;

	bool m_bBreakRequested = false;
};