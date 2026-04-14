#pragma once
#include <Windows.h>

class CScopedCpuTimer
{
public:
	explicit CScopedCpuTimer(float& outMs)
		: m_outMs(outMs)
	{
		LARGE_INTEGER freq{};
		QueryPerformanceFrequency(&freq);

		m_dFreqInv = 1000.0 / static_cast<double>(freq.QuadPart);

		QueryPerformanceCounter(&m_begin);
	}

	~CScopedCpuTimer()
	{
		LARGE_INTEGER end{};
		QueryPerformanceCounter(&end);

		const LONGLONG delta = end.QuadPart - m_begin.QuadPart;
		m_outMs = static_cast<float>(static_cast<double>(delta) * m_dFreqInv);
	}

private:
	float& m_outMs;
	LARGE_INTEGER m_begin{};
	double m_dFreqInv = 0.0;
};