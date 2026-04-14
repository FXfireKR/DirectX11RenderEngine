#pragma once
#include "CScene.h"
#include "CWorld.h"
#include "CBlockCrackRenderer.h"
#include "CFrustumCuller.h"
#include "CCloudLayerRenderer.h"
#include "CBgmController.h"

class CGameScene : public CScene
{
private: // enum, struct
	enum class ESectionDebugMode : uint8_t
	{
		OFF = 0,
		EXIST_ONLY,
		ALL,

		COUNT,
	};

public:
	CGameScene() = default;
	virtual ~CGameScene() = default;

	void Awake() override;
	void Start() override;

	void FixedUpdate(float fDelta) override;
	void Update(float fDelta) override;
	void LateUpdate(float fDelta) override;
	void Build() override;

	void CommitFrameFence() override;
	void BuildRenderFrame() override;

private:
	void _CreateHighlight();
	void _CreateWorldRender();
	void _CreateUICamera();
	void _CreateCrosshairUI();
	void _CreateSkyBillboardResources();

	void _SubmitSunMoonBillboards(CRenderWorld& rw);
	void _SubmitChunkBoundsDebug(CRenderWorld& rw) const;
	void _SubmitSectionBoundsDebug(CRenderWorld& rw) const;

	void _TrySpawnStreaming(CTransform* pPlayerTransform);

	XMFLOAT3 _LerpColor(const XMFLOAT3& a, const XMFLOAT3& b, float t);
	void _ApplySkyClearColor();

	XMMATRIX _BuildSkyLockedQuadWorld(const XMFLOAT3& center, const XMFLOAT3& dirFromCam
		, float width, float height);
	XMMATRIX _BuildScreenAlignedBillboardWorld(const XMFLOAT3& center, const XMFLOAT3& camRight
		, const XMFLOAT3& camUp, float width, float height);
	void _CalcSunMoonDirection(XMFLOAT3& outSunDir, XMFLOAT3& outMoonDir) const;

	XMFLOAT3 _MakeDirFromAngles(float yawDeg, float pitchDeg) const;

#ifdef IMGUI_ACTIVATE
	void _RenderHotbarOverlay();
	bool _TryGetBlockIconDrawInfo(const BlockCell& block, ImVec2& outUV0, ImVec2& outUV1, ImU32& outTint) const;
#endif // IMGUI_ACTIVATE

	void _UpdateAudioListener(float fDelta);

private: // Scene Listener
	CTransform* m_pListenerTransform = nullptr;
	XMFLOAT3 m_prevListenerPos{};
	bool m_bHasPrevListenerPos = false;

private: // Chunk
	CPipeline* m_pChunkPipeline = nullptr;
	CMaterial* m_pChunkMaterial = nullptr;

	CPipeline* m_pChunkCutoutPipeline = nullptr;
	CMaterial* m_pChunkCutoutMaterial = nullptr;

	CPipeline* m_pChunkTransparentPipeline = nullptr;
	CMaterial* m_pChunkTransparentMaterial = nullptr;

	CPipeline* m_pChunkShadowPipeline = nullptr;

private: // ChunkWorld & Player
	CWorld m_VoxelWorld;
	CBlockCrackRenderer m_blockCrackRenderer;
	CCloudLayerRenderer m_cloudLayer;
	CBgmController m_bgmController;

	WorldTimeParams timeParams{};

	CBlockBreakParticleSystem m_blockBreakParticleSystem;

	CObject* m_pPlayer = nullptr;
	CObject* m_pHighlightObject = nullptr;

private: // Debug Bounds
	ESectionDebugMode m_eSectionDebugMode = ESectionDebugMode::OFF;

	bool m_bShowChunkBounds = false;
	CMesh* m_pChunkBoundsDebugMesh = nullptr;
	CPipeline* m_pChunkBoundsDebugPipeline = nullptr;
	CMaterial* m_pChunkBoundsDebugMaterial = nullptr;

private: // UI & cross-hair
	CCamera* m_pUICamera = nullptr;
	CMesh* m_pCrosshairMesh = nullptr;
	CPipeline* m_pCrosshairPipeline = nullptr;
	CMaterial* m_pCrosshairMaterial = nullptr;

private: // sky billboard
	CMesh* m_pSkyBillboardMesh = nullptr;
	CPipeline* m_pSkyBillboardPipeline = nullptr;
	CMaterial* m_pSunBillboardMaterial = nullptr;
	CMaterial* m_pMoonBillboardMaterial = nullptr;

private: // optional
	float m_fSkyBillboardRadius = 400.f;
	float m_fSunBillboardSize = 450.f;
	float m_fMoonBillboardSize = 430.f;
	bool m_bShowSunMoon = true;
	bool m_bSpawnStreamingReady = false;

	float m_debugBias = 0.0005f;

	CFrustumCuller m_frustumCuller;
	bool m_bFrustumculling = true;
	uint32_t m_dbgFrustumTestCount = 0;
	uint32_t m_dbgFrustumCulledCount = 0;

	bool m_bSkyCruiseTest = false;
	float m_fSkyCruiseY = 75.f;
	float m_fSkyCruiseMoveSpeedScale = 10.0f;
};

