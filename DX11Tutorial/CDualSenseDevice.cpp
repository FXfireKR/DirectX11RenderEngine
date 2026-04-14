#include "pch.h"
#include "CDualSenseDevice.h"

CDualSenseDevice::CDualSenseDevice(GAMEPAD_CONNECT_TYPE eConnectType, GAMEPAD_TYPE eType)
{
	m_eConnectType = eConnectType;
	m_eGamePadType = eType;
}

void CDualSenseDevice::BeginFrame()
{
	for (auto& iter : m_buttons) 
	{
		iter.down = false;
		iter.up = false;
	}
}

void CDualSenseDevice::OnRawInput(const RAWINPUT& raw)
{
	const RAWHID& hid = raw.data.hid;
	if (hid.dwSizeHid < 64) 
		return;

	m_uLastInputTime = GetTickCount64();

	// Input Debuging
	//for (int i = 0; i < hid.dwSizeHid; ++i)
	//{
	//	printf("%02X ", hid.bRawData[i]);
	//}
	//printf("\n");

	if (GAMEPAD_CONNECT_TYPE::BLUETOOTH == m_eConnectType)
	{
		_ParsingDualSenseBT(reinterpret_cast<const uint8_t*>(hid.bRawData), hid.dwSizeHid);
	}
	else
	{	
		_ParsingDualSenseUSB(reinterpret_cast<const uint8_t*>(hid.bRawData), hid.dwSizeHid);
	}
}

void CDualSenseDevice::EndFrame()
{

}

bool CDualSenseDevice::GetButton(DUALSENSE_BUTTON btn) const
{
	return m_buttons[(size_t)btn].isHeld;
}

bool CDualSenseDevice::GetButtonDown(DUALSENSE_BUTTON btn) const
{
	return m_buttons[(size_t)btn].down;
}

bool CDualSenseDevice::GetButtonUp(DUALSENSE_BUTTON btn) const
{
	return m_buttons[(size_t)btn].up;
}

void CDualSenseDevice::_ParsingDualSenseUSB(const uint8_t* data, UINT size)
{
	// Raw Input에서 USB는 보통 0x01 입력 리포트 64바이트
	if (size < 11 || data[0] != 0x01)
		return;

	m_lx.value = _NormalizeAxis(data[1]);
	m_ly.value = _NormalizeAxis(data[2]);
	m_rx.value = _NormalizeAxis(data[3]);
	m_ry.value = _NormalizeAxis(data[4]);

	m_l2.value = _NormalizeTrigger(data[5]);
	m_r2.value = _NormalizeTrigger(data[6]);

	const uint8_t b0 = data[8];
	const uint8_t b1 = data[9];
	const uint8_t b2 = data[10];

	_SetDPadByHat(b0 & 0x0F);

	_SetButton(DUALSENSE_BUTTON::SQUARE, (b0 & 0x10) != 0);
	_SetButton(DUALSENSE_BUTTON::CROSS, (b0 & 0x20) != 0);
	_SetButton(DUALSENSE_BUTTON::CIRCLE, (b0 & 0x40) != 0);
	_SetButton(DUALSENSE_BUTTON::TRIANGLE, (b0 & 0x80) != 0);

	_SetButton(DUALSENSE_BUTTON::L1, (b1 & 0x01) != 0);
	_SetButton(DUALSENSE_BUTTON::R1, (b1 & 0x02) != 0);
	_SetButton(DUALSENSE_BUTTON::L2, (b1 & 0x04) != 0);
	_SetButton(DUALSENSE_BUTTON::R2, (b1 & 0x08) != 0);
	_SetButton(DUALSENSE_BUTTON::SHARE, (b1 & 0x10) != 0);
	_SetButton(DUALSENSE_BUTTON::OPTIONS, (b1 & 0x20) != 0);
	_SetButton(DUALSENSE_BUTTON::LSTICK, (b1 & 0x40) != 0);
	_SetButton(DUALSENSE_BUTTON::RSTICK, (b1 & 0x80) != 0);

	UNREFERENCED_PARAMETER(b2);
}


void CDualSenseDevice::_ParsingDualSenseBT(const uint8_t* data, UINT size)
{
	// 1) BT minimal input report 0x01 (10 bytes)
	if (size >= 10 && data[0] == 0x01)
	{
		m_lx.value = _NormalizeAxis(data[1]);
		m_ly.value = _NormalizeAxis(data[2]);
		m_rx.value = _NormalizeAxis(data[3]);
		m_ry.value = _NormalizeAxis(data[4]);

		const uint8_t b0 = data[5];
		const uint8_t b1 = data[6];
		const uint8_t b2 = data[7];

		m_l2.value = _NormalizeTrigger(data[8]);
		m_r2.value = _NormalizeTrigger(data[9]);

		_SetDPadByHat(b0 & 0x0F);

		_SetButton(DUALSENSE_BUTTON::SQUARE, (b0 & 0x10) != 0);
		_SetButton(DUALSENSE_BUTTON::CROSS, (b0 & 0x20) != 0);
		_SetButton(DUALSENSE_BUTTON::CIRCLE, (b0 & 0x40) != 0);
		_SetButton(DUALSENSE_BUTTON::TRIANGLE, (b0 & 0x80) != 0);

		_SetButton(DUALSENSE_BUTTON::L1, (b1 & 0x01) != 0);
		_SetButton(DUALSENSE_BUTTON::R1, (b1 & 0x02) != 0);
		_SetButton(DUALSENSE_BUTTON::L2, (b1 & 0x04) != 0);
		_SetButton(DUALSENSE_BUTTON::R2, (b1 & 0x08) != 0);
		_SetButton(DUALSENSE_BUTTON::SHARE, (b1 & 0x10) != 0);
		_SetButton(DUALSENSE_BUTTON::OPTIONS, (b1 & 0x20) != 0);
		_SetButton(DUALSENSE_BUTTON::LSTICK, (b1 & 0x40) != 0);
		_SetButton(DUALSENSE_BUTTON::RSTICK, (b1 & 0x80) != 0);

		UNREFERENCED_PARAMETER(b2);
		return;
	}

	// 2) BT full report 0x31도 최소 대응
	if (size >= 12 && data[0] == 0x31)
	{
		// 0x31 BT full report는 헤더 뒤 공통 입력 영역이 이어짐
		const uint8_t* p = data + 2;

		m_lx.value = _NormalizeAxis(p[0]);
		m_ly.value = _NormalizeAxis(p[1]);
		m_rx.value = _NormalizeAxis(p[2]);
		m_ry.value = _NormalizeAxis(p[3]);

		m_l2.value = _NormalizeTrigger(p[4]);
		m_r2.value = _NormalizeTrigger(p[5]);

		const uint8_t b0 = p[7];
		const uint8_t b1 = p[8];
		const uint8_t b2 = p[9];

		_SetDPadByHat(b0 & 0x0F);

		_SetButton(DUALSENSE_BUTTON::SQUARE, (b0 & 0x10) != 0);
		_SetButton(DUALSENSE_BUTTON::CROSS, (b0 & 0x20) != 0);
		_SetButton(DUALSENSE_BUTTON::CIRCLE, (b0 & 0x40) != 0);
		_SetButton(DUALSENSE_BUTTON::TRIANGLE, (b0 & 0x80) != 0);

		_SetButton(DUALSENSE_BUTTON::L1, (b1 & 0x01) != 0);
		_SetButton(DUALSENSE_BUTTON::R1, (b1 & 0x02) != 0);
		_SetButton(DUALSENSE_BUTTON::L2, (b1 & 0x04) != 0);
		_SetButton(DUALSENSE_BUTTON::R2, (b1 & 0x08) != 0);
		_SetButton(DUALSENSE_BUTTON::SHARE, (b1 & 0x10) != 0);
		_SetButton(DUALSENSE_BUTTON::OPTIONS, (b1 & 0x20) != 0);
		_SetButton(DUALSENSE_BUTTON::LSTICK, (b1 & 0x40) != 0);
		_SetButton(DUALSENSE_BUTTON::RSTICK, (b1 & 0x80) != 0);

		UNREFERENCED_PARAMETER(b2);
	}
}

void CDualSenseDevice::_SetButton(DUALSENSE_BUTTON button, bool pressed)
{
	ButtonState& s = m_buttons[(size_t)button];
	if (pressed != s.isHeld)
	{
		if (pressed) s.down = true;
		else         s.up = true;
	}
	s.isHeld = pressed;
}

void CDualSenseDevice::_SetDPadByHat(uint8_t hat)
{
	bool up = false, down = false, left = false, right = false;

	switch (hat & 0x0F)
	{
	case 0: up = true; break;
	case 1: up = true; right = true; break;
	case 2: right = true; break;
	case 3: right = true; down = true; break;
	case 4: down = true; break;
	case 5: down = true; left = true; break;
	case 6: left = true; break;
	case 7: left = true; up = true; break;
	default: break; // 8 = neutral
	}

	_SetButton(DUALSENSE_BUTTON::DPAD_UP, up);
	_SetButton(DUALSENSE_BUTTON::DPAD_DOWN, down);
	_SetButton(DUALSENSE_BUTTON::DPAD_LEFT, left);
	_SetButton(DUALSENSE_BUTTON::DPAD_RIGHT, right);
}

float CDualSenseDevice::_NormalizeAxis(uint8_t raw) const
{
	float v = (static_cast<int>(raw) - 128) / 127.0f;
	v = std::clamp(v, -1.0f, 1.0f);

	const float dead = 0.15f;
	if (fabsf(v) < dead)
		return 0.0f;

	const float sign = (v >= 0.0f) ? 1.0f : -1.0f;
	const float mag = (fabsf(v) - dead) / (1.0f - dead);
	return sign * std::clamp(mag, 0.0f, 1.0f);
}

float CDualSenseDevice::_NormalizeTrigger(uint8_t raw) const
{
	return std::clamp(raw / 255.0f, 0.0f, 1.0f);
}
