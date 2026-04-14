#pragma once
#include "CScene.h"

class CBootScene : public CScene
{
public:
	CBootScene();
	virtual ~CBootScene();

	void Awake() override;
	void FixedUpdate(float fDelta) override;
	void Update(float fDelta) override;
	void LateUpdate(float fDelta) override;
};