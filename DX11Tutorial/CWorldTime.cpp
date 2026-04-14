#include "pch.h"
#include "CWorldTime.h"

namespace
{
	constexpr float MINECRAFT_TICKS_PER_DAY = 24000.0f;

	static float Wrap01(float v)
	{
		v = std::fmod(v, 1.0f);
		if (v < 0.0f)
			v += 1.0f;
		return v;
	}

	static float Saturate(float v)
	{
		if (v < 0.0f) return 0.0f;
		if (v > 1.0f) return 1.0f;
		return v;
	}
}

void CWorldTime::Initialize(float startDay01)
{
	startDay01 = Wrap01(startDay01);
	m_dAccumWorldSec = static_cast<double>(startDay01 * m_fDayLengthSec);
}

void CWorldTime::Update(float fDelta)
{
	if (m_bPaused)
		return;

	if (m_fDayLengthSec <= 0.0f)
		return;

	m_dAccumWorldSec += static_cast<double>(fDelta * m_fTimeScale);

	const double dayLen = static_cast<double>(m_fDayLengthSec);
	m_dAccumWorldSec = std::fmod(m_dAccumWorldSec, dayLen);

	if (m_dAccumWorldSec < 0.0)
		m_dAccumWorldSec += dayLen;
}

float CWorldTime::GetDay01() const
{
	if (m_fDayLengthSec <= 0.0f)
		return 0.0f;

	const float day01 = static_cast<float>(m_dAccumWorldSec / static_cast<double>(m_fDayLengthSec));
	return Wrap01(day01);
}

float CWorldTime::GetTickOfDay() const
{
	return GetDay01() * MINECRAFT_TICKS_PER_DAY;
}

WorldTimeParams CWorldTime::Evaluate() const
{
    WorldTimeParams out{};

    out.day01 = GetDay01();
    out.tickOfDay = GetTickOfDay();

    const float phase = out.day01 * XM_2PI - XM_PIDIV2;

    out.sunAngleRad = phase;
    out.moonAngleRad = phase + XM_PI;

    out.sunHeight = sinf(out.sunAngleRad);

    out.daylight = Saturate(0.5f + 0.5f * out.sunHeight);
    out.night = 1.0f - out.daylight;

    return out;
}