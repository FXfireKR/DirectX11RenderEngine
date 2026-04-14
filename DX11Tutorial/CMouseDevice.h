#pragma once
#include "IInputDevice.h"

class CWindow;

enum class MOUSE_BUTTON
{
	LEFT,
	RIGHT,
	MIDDLE,

	COUNT
};

class CMouseDevice : public IInputDevice
{
public:

	void BeginFrame() override;
	void OnRawInput(const RAWINPUT& raw) override;
	void EndFrame() override;

	const bool GetKey(uint16_t vk) const;
	const bool GetKeyDown(uint16_t vk) const;
	const bool GetKeyUp(uint16_t vk) const;

	inline const short& GetWheelCnt() const { return m_sWheel; }
	inline const short GetWheelDir() const { return m_sWheel > 0 ? 1 : -1; }

	inline const POINT& GetDelta() const { return m_delta; }
	inline const POINT& GetPosition() const { return m_pos; }

	inline const bool& GetMoveLock() const { return m_bMouseMoveLock; }
	inline const void EnalbleMove() { 
		if (m_bMouseMoveLock != false) {
			m_bMouseCursorDirty = true;
		}
		m_bMouseMoveLock = false;
	}
	inline const void DisalbleMove() { 
		if (m_bMouseMoveLock != true) {
			m_bMouseCursorDirty = true;
		}
		m_bMouseMoveLock = true; 
	}

	inline void SetWindowTarget(CWindow& window) { m_pWindow = &window; }

private:
	void _Handle(MOUSE_BUTTON eMouseButton, bool bDown);

private:
	array<ButtonState, (size_t)MOUSE_BUTTON::COUNT> m_buttons;

	POINT m_delta;
	POINT m_pos;
	short m_sWheel = 0;
	bool m_bMouseMoveLock = false;
	bool m_bMouseCursorDirty = false;

	CWindow* m_pWindow = nullptr;
};