#include "pch.h"
#include "CDeltaTimeManager.h"

void CDeltaTimeManager::Init()
{
	// cpu-time
	::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&m_uFrequency));
	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&m_uPrevCount));
}

void CDeltaTimeManager::Tick()
{
	// 기본 타입들 대부분은 힙 할당 없이 최적화 가능(생성, 소멸자가 있으면 이야기가 달라짐)
	// 따라서 생성/소멸자가 있는 지역 객체는 계속 만들지 말고 캐싱하는 것을 고려할 것.
	unsigned __int64 uCurrentTime = 0; 

	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&uCurrentTime));

	m_fDeltaTime = static_cast<float>(uCurrentTime - m_uPrevCount) / static_cast<float>(m_uFrequency);
	m_uPrevCount = uCurrentTime;

	++m_uFrameCount;
	m_fFrameTime += m_fDeltaTime;

	if (m_fFrameTime > 1.f) 
	{
		m_fFramePerSecond = static_cast<float>(m_uFrameCount) / m_fFrameTime;
		m_fFrameTime = 0.f;
		m_uFrameCount = 0;
	}
} 