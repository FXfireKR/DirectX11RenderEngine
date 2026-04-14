#pragma once
#include "CMouseDevice.h"
#include "CKeyboardDevice.h"
#include "CGamePadHub.h"

#include "singleton.h"

class CInputManager : public singleton<CInputManager>
{
public:
	CInputManager() = default;
	~CInputManager() = default;

	void Initialize();
	void BeginFrame();
	void EndFrame();

public:
	inline void SetMouseDevice(CMouseDevice& mouse) { m_pMouse = &mouse; }
	inline void SetKeyboardDevice(CKeyboardDevice& keyboard) { m_pKeyboard = &keyboard; }
	inline void SetGamePadDevice(CGamePadHub& gamepad) { m_pGamePad = &gamepad; }

	inline CMouseDevice& Mouse() { return *m_pMouse; }
	inline CKeyboardDevice& Keyboard() { return *m_pKeyboard; }
	inline CGamePadHub& GamePad() { return *m_pGamePad; }

	inline EActiveInputDevice GetActiveInputDevice() const { return m_eActiveInputDevice; }
	inline void SetActiveInputDevice(EActiveInputDevice eDevice) { m_eActiveInputDevice = eDevice; }
	inline bool IsGamePadMode() const { return m_eActiveInputDevice == EActiveInputDevice::GAMEPAD; }

private:
	CMouseDevice* m_pMouse = nullptr;
	CKeyboardDevice* m_pKeyboard = nullptr;
	CGamePadHub* m_pGamePad = nullptr;

	EActiveInputDevice m_eActiveInputDevice = EActiveInputDevice::KEYBOARD_MOUSE;
};

// TODO: 차후 Action Bind가 필요한경우 활용한 클래스를 만들어내야한다.