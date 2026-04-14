#pragma once
#include "CDualSenseDevice.h"

class CGamePadHub
{
public:
	CGamePadHub() = default;
	~CGamePadHub() = default;

	void BeginFrame();
	void EndFrame();
	void OnRawInput(const RAWINPUT& raw);

	const CDualSenseDevice* GetActivateDualSense() const;

private:
	bool _CreatePadDevice(const RAWINPUT& raw);
	bool _GetDeviceName(HANDLE device, wstring& outName);
	bool _GetDeviceInfo(HANDLE device, RID_DEVICE_INFO& pDeviceInfo);
	bool _CheckBTDeviceName(HANDLE device);
	bool _CheckDualSenseBt(const RAWINPUT& raw);

private:
	unordered_map<HANDLE, unique_ptr<IGamePadDevice>> m_mapDevices;
};