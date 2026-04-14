#pragma once
#include "singleton.h"

class CDeltaTimeManager : public singleton<CDeltaTimeManager>
{
public:
	CDeltaTimeManager() {}
	~CDeltaTimeManager() {}

	void Init();
	void Tick();

	inline unsigned __int64 GetFps() { return static_cast<unsigned __int64>(m_fFramePerSecond); }
	inline const float& GetDeltaTime() { return m_fDeltaTime; }

private:
	unsigned __int64 m_uFrequency = 0;
	unsigned __int64 m_uPrevCount = 0;
	unsigned __int64 m_uFrameCount = 0;

	float m_fFramePerSecond = 0.f;
	float m_fFrameTime = 0.f;
	float m_fDeltaTime = 0.f;
};