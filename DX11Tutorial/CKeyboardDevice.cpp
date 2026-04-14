#include "pch.h"
#include "CKeyboardDevice.h"

void CKeyboardDevice::BeginFrame()
{
	for (auto& iter : m_buttons) {
		iter.down = false;
		iter.up = false;
	}
}

void CKeyboardDevice::OnRawInput(const RAWINPUT& raw)
{
	const RAWKEYBOARD& keyboard = raw.data.keyboard;
	uint16_t vk = (uint16_t)keyboard.VKey;

	bool down = !(keyboard.Flags & RI_KEY_BREAK);

	if (down && !m_buttons[vk].isHeld) m_buttons[vk].down = true;
	if (!down && m_buttons[vk].isHeld) m_buttons[vk].up = true;

	m_buttons[vk].isHeld = down;
}

void CKeyboardDevice::EndFrame()
{
}