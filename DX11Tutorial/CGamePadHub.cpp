#include "pch.h"
#include "CGamePadHub.h"

void CGamePadHub::BeginFrame()
{
	for (auto& kv : m_mapDevices)
	{
		if (kv.second)
			kv.second->BeginFrame();
	}
}

void CGamePadHub::EndFrame()
{
	for (auto& kv : m_mapDevices)
	{
		if (kv.second)
			kv.second->BeginFrame();
	}
}

void CGamePadHub::OnRawInput(const RAWINPUT& raw)
{
	HANDLE hDevice = raw.header.hDevice;
	if (false == m_mapDevices.contains(hDevice)) 
	{
		if (false == _CreatePadDevice(raw)) 
			return;
	}
	m_mapDevices[hDevice]->OnRawInput(raw);
}

const CDualSenseDevice* CGamePadHub::GetActivateDualSense() const
{
	const CDualSenseDevice* pBest = nullptr;
	uint64_t bestTime = 0;

	for (const auto& kv : m_mapDevices)
	{
		if (!kv.second)
			continue;

		if (kv.second->GetGamePadType() != GAMEPAD_TYPE::DUALSENSE)
			continue;

		const CDualSenseDevice* pDS = static_cast<const CDualSenseDevice*>(kv.second.get());
		if (!pBest || pDS->GetLastInputTime() > bestTime)
		{
			pBest = pDS;
			bestTime = pDS->GetLastInputTime();
		}
	}

	return pBest;
}

bool CGamePadHub::_CreatePadDevice(const RAWINPUT& raw)
{
	HANDLE hDevice = raw.header.hDevice;
	RID_DEVICE_INFO newDeviceInfo;

	if (false == _GetDeviceInfo(raw.header.hDevice, newDeviceInfo)) return false;

	// DualSense
	if (newDeviceInfo.hid.dwVendorId == 0x054C && newDeviceInfo.hid.dwProductId == 0x0CE6)
	{
		// check dualsense bluetooth
		if (_CheckBTDeviceName(hDevice) || _CheckDualSenseBt(raw)) {
			m_mapDevices.insert(make_pair(hDevice, make_unique<CDualSenseDevice>(GAMEPAD_CONNECT_TYPE::BLUETOOTH, GAMEPAD_TYPE::DUALSENSE)));
		}
		else {
			m_mapDevices.insert(make_pair(hDevice, make_unique<CDualSenseDevice>(GAMEPAD_CONNECT_TYPE::USB, GAMEPAD_TYPE::DUALSENSE)));
		}
		return true;
	}
	return false;
}

bool CGamePadHub::_GetDeviceName(HANDLE device, wstring& outName)
{
	UINT size = 0;
	GetRawInputDeviceInfo(device, RIDI_DEVICENAME, nullptr, &size);
	if (size == 0) return false;
	outName.resize(size);
	return GetRawInputDeviceInfo(device, RIDI_DEVICENAME, outName.data(), &size) != (UINT)-1;
}

bool CGamePadHub::_GetDeviceInfo(HANDLE device, RID_DEVICE_INFO& pDeviceInfo)
{
	pDeviceInfo.cbSize = sizeof(RID_DEVICE_INFO);

	UINT size = pDeviceInfo.cbSize;
	if (GetRawInputDeviceInfo(device, RIDI_DEVICEINFO, &pDeviceInfo, &size) > 0)
		return true;
	return false;
}

bool CGamePadHub::_CheckBTDeviceName(HANDLE device)
{
	wstring name = L"";
	if (!_GetDeviceName(device, name)) return false;
	return name.find(L"BTHENUM") != std::wstring::npos;
}

bool CGamePadHub::_CheckDualSenseBt(const RAWINPUT& raw)
{
	const RAWHID& hid = raw.data.hid;
	if (hid.dwSizeHid < 70) return false;
	const uint8_t* data = reinterpret_cast<const uint8_t*>(hid.bRawData);
	return data[0] == 0x31;
}