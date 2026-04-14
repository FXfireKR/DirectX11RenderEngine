#pragma once
#include "IInputDevice.h"

using KeyCode = uint16_t;

class CKeyboardDevice : public IInputDevice
{
public:
	void BeginFrame() override;
	void OnRawInput(const RAWINPUT& raw) override;
	void EndFrame() override;

	inline const bool& GetKey(uint16_t vk) const { return m_buttons[vk].isHeld; }
	inline const bool& GetKeyDown(uint16_t vk) const { return m_buttons[vk].down; }
	inline const bool& GetKeyUp(uint16_t vk) const { return m_buttons[vk].up; }

private:
	static constexpr int KEY_MAX = 256;

	array<ButtonState, KEY_MAX> m_buttons;
};