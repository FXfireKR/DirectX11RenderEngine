#pragma once
#include "pch.h"

enum class KEY_STATE : uint8_t
{
	NONE,
	DOWN,
	PRESSED,
	UP
};

struct ButtonState
{
	bool up = false;
	bool down = false;
	bool isHeld = false;
};

enum class EActiveInputDevice : uint8_t
{
	KEYBOARD_MOUSE = 0,
	GAMEPAD,
};

class IInputDevice
{
public:
	virtual ~IInputDevice() = default;
	virtual void BeginFrame() {}
	virtual void EndFrame() {}
	virtual void OnRawInput(const RAWINPUT& raw) { UNREFERENCED_PARAMETER(raw); }
};