#pragma once
#include "IInputDevice.h"

enum class GAMEPAD_CONNECT_TYPE
{
	USB,
	BLUETOOTH,
};

enum class GAMEPAD_TYPE
{
	DUALSENSE,
};

class IGamePadDevice : public IInputDevice
{
public:
	virtual ~IGamePadDevice() = default;

	inline const GAMEPAD_CONNECT_TYPE& GetConnectType() const { return m_eConnectType; }
	inline const GAMEPAD_TYPE& GetGamePadType() const { return m_eGamePadType; }

protected:
	GAMEPAD_CONNECT_TYPE m_eConnectType;
	GAMEPAD_TYPE m_eGamePadType;
};