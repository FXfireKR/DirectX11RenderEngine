#pragma once

struct WorldTimeParams
{
	float day01 = 0.f; // [0, 1)
	float tickOfDay = 0.f; // [0, 24000)

	float sunAngleRad = 0.f;
	float moonAngleRad = 0.f;

	float sunHeight = 0.f; // -1 ~ 1
	float daylight = 0.f;	// 0 ~ 1
	float night = 0.f;	// 0 ~ 1

	float sunriseFactor = 0.f;
	float sunsetFactor = 0.f;
};

class CWorldTime
{
public:
	CWorldTime() = default;
	~CWorldTime() = default;

	void Initialize(float startDay01 = 0.25f);
	void Update(float fDelta);

	float GetDay01() const;
	float GetTickOfDay() const;
	WorldTimeParams Evaluate() const;

public:
	inline void SetPaused(bool bPaused) { m_bPaused = bPaused; }
	inline bool IsPaused() const { return m_bPaused; }

	inline void SetDayLengthSec(float sec) { m_fDayLengthSec = sec; }
	inline float GetDayLengthSec() const { return m_fDayLengthSec; }

	inline void SetTimeScale(float scale) { m_fTimeScale = scale; }
	inline float GetTimeScale() const { return m_fTimeScale; }


private:
	double m_dAccumWorldSec = 0.0;
	float m_fDayLengthSec = 120.0f; // ex ) 현실 20분 = 게임의 하루
	float m_fTimeScale = 1.0f;
	bool m_bPaused = false;

};