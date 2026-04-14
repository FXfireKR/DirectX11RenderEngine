#include "pch.h"
#include "CRawInputDispatcher.h"

void CRawInputDispatcher::DispatchRawQueue()
{
	while (!m_queueRawInput.empty()) 
	{
		_OnRawInput(m_queueRawInput.front());
		m_queueRawInput.pop();
	}
}

void CRawInputDispatcher::_OnRawInput(const RAWINPUT& raw)
{
	switch (raw.header.dwType)
	{
	case RIM_TYPEMOUSE:
		m_mouse.OnRawInput(raw);
		break;

	case RIM_TYPEKEYBOARD:
		m_keyBoard.OnRawInput(raw);
		break;

	case RIM_TYPEHID:
		m_gamePad.OnRawInput(raw);
		break;
	}
}

void CRawInputDispatcher::Push(const RAWINPUT& raw)
{
	m_queueRawInput.push(raw);
}