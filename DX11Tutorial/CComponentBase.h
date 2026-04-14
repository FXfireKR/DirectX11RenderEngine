#pragma once
#include "CComponent.h"

template<typename t, COMPONENT_TYPE type>
class CComponentBase : public CComponent
{
public:
	CComponentBase() = default;
	virtual ~CComponentBase() = default;

	void Init() override {}
	void Start() override {}
	void FixedUpdate(float fDelta) override { UNREFERENCED_PARAMETER(fDelta); }
	void Update(float fDelta) override { UNREFERENCED_PARAMETER(fDelta); }
	void LateUpdate(float fDelta) override { UNREFERENCED_PARAMETER(fDelta); }
	void Build() override {}
	void Render() override {}

	inline static constexpr COMPONENT_TYPE GetStaticType() { return type; }
	inline COMPONENT_TYPE GetType() const override { return GetStaticType(); }
};