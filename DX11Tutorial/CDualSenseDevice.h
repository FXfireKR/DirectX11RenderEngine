#pragma once
#include "IGamePadDevice.h"

/*
* https://github.com/nondebug/dualsense
* DualSense 참고
*/

enum class DUALSENSE_BUTTON
{
	CROSS,
	CIRCLE,
	SQUARE,
	TRIANGLE,

	DPAD_UP,
	DPAD_DOWN,
	DPAD_LEFT,
	DPAD_RIGHT,

	L1,
	R1,
	L2,
	R2,

	OPTIONS,
	SHARE,

	LSTICK,
	RSTICK,

	COUNT
};

struct AxisState
{
	float value = 0.0f;
};

struct TriggerState
{
	float value = 0.0f;
};

class CDualSenseDevice : public IGamePadDevice
{
public:
	CDualSenseDevice(GAMEPAD_CONNECT_TYPE eConnectType, GAMEPAD_TYPE eType);

	void BeginFrame() override;
	void OnRawInput(const RAWINPUT& raw) override;
	void EndFrame() override;

public:
	bool GetButton(DUALSENSE_BUTTON btn) const;
	bool GetButtonDown(DUALSENSE_BUTTON btn) const;
	bool GetButtonUp(DUALSENSE_BUTTON btn) const;

	inline float GetLX() const { return m_lx.value; }
	inline float GetLY() const { return m_ly.value; }
	inline float GetRX() const { return m_rx.value; }
	inline float GetRY() const { return m_ry.value; }

	inline float GetL2Axis() const { return m_l2.value; }
	inline float GetR2Axis() const { return m_r2.value; }

	inline uint64_t GetLastInputTime() const { return m_uLastInputTime; }

private:
	void _ParsingDualSenseUSB(const uint8_t* data, UINT size);
	void _ParsingDualSenseBT(const uint8_t* data, UINT size);

	void _SetButton(DUALSENSE_BUTTON button, bool pressed);
	void _SetDPadByHat(uint8_t hat);
	float _NormalizeAxis(uint8_t raw) const;
	float _NormalizeTrigger(uint8_t raw) const;

private:
	array<ButtonState, (size_t)DUALSENSE_BUTTON::COUNT> m_buttons;

	AxisState m_lx, m_ly, m_rx, m_ry;
	TriggerState m_l2, m_r2;

	uint64_t m_uLastInputTime = 0;
};