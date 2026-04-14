#pragma once
#include "CMouseDevice.h"
#include "CKeyboardDevice.h"
#include "CGamePadHub.h"

class CRawInputDispatcher
{
public:
	CRawInputDispatcher() = default;
	~CRawInputDispatcher() = default;

	void DispatchRawQueue();
	void Push(const RAWINPUT& raw);

private:
	void _OnRawInput(const RAWINPUT& raw);

public:
	inline CMouseDevice& GetMouse() { return m_mouse; }
	inline CKeyboardDevice& GetKeyboard() { return m_keyBoard; }
	inline CGamePadHub& GetGamePad() { return m_gamePad; }

private:
	CMouseDevice m_mouse;
	CKeyboardDevice m_keyBoard;
	CGamePadHub m_gamePad;

	queue<RAWINPUT> m_queueRawInput;
};