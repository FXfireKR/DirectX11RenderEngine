#pragma once
#include "CScene.h"

class CInitializeScene : public CScene
{
private:
	enum class INITIALIZE_STEP : uint8_t
	{
		BLOCK_DB_LOAD = 0,
		BLOCK_RESOURCE_DB_LOAD,
		SOUND_PRE_LOAD,
		RENDER_WARM,

		END,
	};

public:
	CInitializeScene() = default;
	virtual ~CInitializeScene() = default;

	void Awake() override;
	void Update(float fDelta) override;

private:
	bool _WarmupGameSceneRenderResources();

private:
	uint8_t m_iCurrentLoadStep = 0;
};