#include "pch.h"
#include "CAudioSystem.h"

using namespace FMOD;

#define FMOD_RESULT_OK(result) result == FMOD_RESULT::FMOD_OK

#ifndef SAFE_RELEASE_FMOD_SOUND
#define SAFE_RELEASE_FMOD_SOUND(p) if ((p) != nullptr) { (p)->release(); (p) = nullptr; }
#endif

CAudioSystem::CAudioSystem()
	: m_pSystem(nullptr)
{
	m_busGroups.fill(nullptr);
	m_busVolumes.fill(1.0f);
}

CAudioSystem::~CAudioSystem()
{
	Shutdown();
}

bool CAudioSystem::Initialize()
{
	if (m_pSystem)
		return true;

	FMOD_RESULT result = FMOD::System_Create(&m_pSystem);
	if (!FMOD_RESULT_OK(result))
		return false;

	result = m_pSystem->init(512, FMOD_INIT_NORMAL, nullptr);
	if (!FMOD_RESULT_OK(result))
		return false;

	result = m_pSystem->set3DSettings(1.0f, 1.0f, 1.0f);
	if (!FMOD_RESULT_OK(result))
		return false;

	if (!_CreateChannelGroups())
		return false;

	assert(m_pSystem);
	return true;
}

void CAudioSystem::Shutdown()
{
	for (auto& kv : m_mapSounds)
	{
		SAFE_RELEASE_FMOD_SOUND(kv.second);
	}
	m_mapSounds.clear();

	for (size_t i = 0; i < AUDIO_BUS_COUNT; ++i)
		m_busGroups[i] = nullptr;

	if (m_pSystem)
	{
		m_pSystem->close();
		m_pSystem->release();
		m_pSystem = nullptr;
	}
}

void CAudioSystem::Tick()
{
	_Dispatch();

	m_pSystem->update();
}

void CAudioSystem::Submit2D(SoundID soundID, EAudioBus bus/*= EAudioBus::SFX*/, float volume/*= 1.f*/, float pitch/*= 1.f*/)
{
	AudioRequest req;
	req.b3D = false;
	req.id = soundID;
	req.volume = volume;
	req.pitch = pitch;
	req.bus = bus;

	m_pendingRequests.push(req);
}

void CAudioSystem::Submit3D(SoundID soundID, const XMFLOAT3& pos, EAudioBus bus/*= EAudioBus::SFX*/, float volume/*= 1.f*/
	, float pitch/*= 1.f*/, float minDistance/*= 1.f*/, float maxDistance/*= 24.f*/)
{
	AudioRequest req;
	req.b3D = true;
	req.id = soundID;
	req.pos = pos;
	req.volume = volume;
	req.pitch = pitch;
	req.minDistance = minDistance;
	req.maxDistance = maxDistance;
	req.bus = bus;

	m_pendingRequests.push(req);
}

bool CAudioSystem::LoadSound(SoundID id, const char* path, bool b3D, bool bLoop/*=false*/, bool bStream/*=false*/)
{
	if (!m_pSystem || !path || !path[0])
		return false;

	auto it = m_mapSounds.find(id);
	if (it != m_mapSounds.end())
		return true;

	AudioLoadDesc newDesc;
	newDesc.b3D = b3D;
	newDesc.bLoop = bLoop;
	newDesc.bStream = bStream;

	FMOD_MODE mode = _MakeModeFlags(newDesc);

	FMOD::Sound* pSound = nullptr;
	FMOD_RESULT result;

	if (bStream)
		result = m_pSystem->createStream(path, mode, nullptr, &pSound);
	else
		result = m_pSystem->createSound(path, mode, nullptr, &pSound);

	if (!FMOD_RESULT_OK(result))
		return false;

	m_mapSounds.emplace(id, pSound);
	return true;
}

bool CAudioSystem::LoadSound(SoundID id, const char* path, const AudioLoadDesc& desc)
{
	return LoadSound(id, path, desc.b3D, desc.bLoop, desc.bStream);
}

FMOD::Channel* CAudioSystem::PlayBGMNow(SoundID id, const char* path, float volume, bool bLoop)
{
	if (!m_pSystem)
		return nullptr;

	auto iter = m_mapSounds.find(id);
	if (iter == m_mapSounds.end())
	{
		if (!LoadSound(id, path, false, false, true))
			return nullptr;

		iter = m_mapSounds.find(id);
	}

	FMOD::Sound* pSound = iter->second;
	if (!pSound)
		return nullptr;

	if (bLoop)
		pSound->setMode(FMOD_LOOP_NORMAL);
	else
		pSound->setMode(FMOD_LOOP_OFF);

	FMOD::Channel* pChannel = nullptr;
	FMOD_RESULT fr = m_pSystem->playSound(pSound, _GetBusGroup(EAudioBus::BGM), true, &pChannel);
	if (fr != FMOD_OK || !pChannel)
		return nullptr;

	pChannel->setVolume(volume);
	pChannel->setPaused(false);
	return pChannel;
}

void CAudioSystem::StopChannel(FMOD::Channel* pChannel, bool bFadeOut)
{
	UNREFERENCED_PARAMETER(bFadeOut);

	if (!pChannel)
		return;

	pChannel->stop();
}

bool CAudioSystem::IsChannelPlaying(FMOD::Channel* pChannel) const
{
	if (!pChannel)
		return false;

	bool bPlaying = false;
	if (pChannel->isPlaying(&bPlaying) != FMOD_OK)
		return false;

	return bPlaying;
}

void CAudioSystem::SetListener(const AudioListenerState& state)
{
	if (!m_pSystem)
		return;

	FMOD_VECTOR pos = _ToFMOD(state.pos);
	FMOD_VECTOR vel = _ToFMOD(state.vel);
	FMOD_VECTOR forward = _ToFMOD(state.forward);
	FMOD_VECTOR up = _ToFMOD(state.up);

	m_pSystem->set3DListenerAttributes(0, &pos, &vel, &forward, &up);
}

void CAudioSystem::SetVolume(EAudioBus bus, float volume)
{
	const int idx = static_cast<int>(bus);
	if (idx < 0 || idx >= static_cast<int>(AUDIO_BUS_COUNT))
		return;

	if (volume < 0.f) volume = 0.f;
	if (volume > 1.f) volume = 1.f;

	m_busVolumes[idx] = volume;

	FMOD::ChannelGroup* pGroup = _GetBusGroup(bus);
	if (pGroup)
	{
		pGroup->setVolume(volume);
	}
}

float CAudioSystem::GetVolume(EAudioBus bus) const
{
	const int idx = static_cast<int>(bus);
	if (idx < 0 || idx >= static_cast<int>(AUDIO_BUS_COUNT))
		return 1.0f;

	return m_busVolumes[idx];
}

void CAudioSystem::_Dispatch()
{
	size_t dispatchCount = 0;
	while (!m_pendingRequests.empty())
	{
		if (dispatchCount >= m_uDispatchPerFrame)
			break;

		const AudioRequest req = m_pendingRequests.front();
		m_pendingRequests.pop();

		_Play(req);
		++dispatchCount;
	}
}

FMOD::Channel* CAudioSystem::_Play(const AudioRequest& request)
{
	auto it = m_mapSounds.find(request.id);
	if (it == m_mapSounds.end())
		return nullptr;

	FMOD::Channel* pChannel = nullptr;
	FMOD_RESULT result = m_pSystem->playSound(it->second, nullptr, true, &pChannel);
	if (!FMOD_RESULT_OK(result))
		return nullptr;

	if (FMOD::ChannelGroup* pGroup = _GetBusGroup(request.bus))
	{
		pChannel->setChannelGroup(pGroup);
	}

	// 재생할 볼륨 * 속한 채널의 볼륨 * 마스터 볼륨	
	if (request.b3D)
	{
		FMOD_VECTOR pos = _ToFMOD(request.pos);
		FMOD_VECTOR vel = { 0.f, 0.f, 0.f };
		
		pChannel->setMode(FMOD_3D);
		pChannel->set3DAttributes(&pos, &vel);
		pChannel->set3DMinMaxDistance(request.minDistance, request.maxDistance);
	}

	pChannel->setVolume(request.volume);
	pChannel->setPitch(request.pitch);
	pChannel->setPaused(false);

	return pChannel;
}

FMOD::ChannelGroup* CAudioSystem::_GetBusGroup(EAudioBus bus)
{
	return m_busGroups[static_cast<size_t>(bus)];
}

bool CAudioSystem::_CreateChannelGroups()
{
	if (!m_pSystem)
		return false;

	FMOD_RESULT result = m_pSystem->getMasterChannelGroup(&m_busGroups[static_cast<size_t>(EAudioBus::MASTER)]);
	if (!FMOD_RESULT_OK(result))
		return false;

	result = m_pSystem->createChannelGroup("BGM", &m_busGroups[static_cast<size_t>(EAudioBus::BGM)]);
	if (!FMOD_RESULT_OK(result))
		return false;

	result = m_pSystem->createChannelGroup("SFX", &m_busGroups[static_cast<size_t>(EAudioBus::SFX)]);
	if (!FMOD_RESULT_OK(result))
		return false;

	result = m_pSystem->createChannelGroup("AMBIENT", &m_busGroups[static_cast<size_t>(EAudioBus::AMBIENT)]);
	if (!FMOD_RESULT_OK(result))
		return false;

	_GetBusGroup(EAudioBus::MASTER)->addGroup(_GetBusGroup(EAudioBus::BGM));
	_GetBusGroup(EAudioBus::MASTER)->addGroup(_GetBusGroup(EAudioBus::SFX));
	_GetBusGroup(EAudioBus::MASTER)->addGroup(_GetBusGroup(EAudioBus::AMBIENT));

	return true;
}

FMOD_MODE CAudioSystem::_MakeModeFlags(const AudioLoadDesc& desc) const
{
	FMOD_MODE mode = FMOD_DEFAULT;

	if (desc.bLoop)		mode |= FMOD_LOOP_NORMAL;
	else				mode |= FMOD_LOOP_OFF;

	if (desc.b3D)		mode |= FMOD_3D;
	else				mode |= FMOD_2D;

	return mode;
}

FMOD_VECTOR CAudioSystem::_ToFMOD(const XMFLOAT3& v)
{
	return FMOD_VECTOR{ v.x, v.y, v.z };
}