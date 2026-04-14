#pragma once
#include "AudioType.h"

class CAudioSystem
{
public:
	CAudioSystem();
	~CAudioSystem();

	bool Initialize();
	void Shutdown();
	void Tick();

public: // queue-budget
	void Submit2D(SoundID soundID, EAudioBus bus = EAudioBus::SFX, float volume = 1.f, float pitch = 1.f);
	void Submit3D(SoundID soundID, const XMFLOAT3& pos, EAudioBus bus = EAudioBus::SFX, float volume = 1.f
		, float pitch = 1.f, float minDistance = 1.f, float maxDistance = 24.f);

public:
	bool LoadSound(SoundID id, const char* path, bool b3D, bool bLoop = false, bool bStream = false);
	bool LoadSound(SoundID id, const char* path, const AudioLoadDesc& desc);

	FMOD::Channel* PlayBGMNow(SoundID id, const char* path, float volume = 1.f, bool bLoop = false);
	void StopChannel(FMOD::Channel* pChannel, bool bFadeOut = false);
	bool IsChannelPlaying(FMOD::Channel* pChannel) const;

	void SetListener(const AudioListenerState& state);

	void SetVolume(EAudioBus bus, float volume);
	float GetVolume(EAudioBus bus) const;

public:
	inline void SetDispatchPerFrame(size_t uPerFrame) { m_uDispatchPerFrame = uPerFrame; }
	inline size_t GetDispatchPerFrame() const { return m_uDispatchPerFrame; }
	
private:
	void _Dispatch();

	FMOD::Channel* _Play(const AudioRequest& request);

	FMOD::ChannelGroup* _GetBusGroup(EAudioBus bus);
	bool _CreateChannelGroups();
	FMOD_MODE _MakeModeFlags(const AudioLoadDesc& desc) const;
	FMOD_VECTOR _ToFMOD(const XMFLOAT3& v);

private:
	FMOD::System* m_pSystem = nullptr;
	
	unordered_map<SoundID, FMOD::Sound*> m_mapSounds;

	array<FMOD::ChannelGroup*, AUDIO_BUS_COUNT> m_busGroups{};
	array<float, AUDIO_BUS_COUNT> m_busVolumes{};

	queue<AudioRequest> m_pendingRequests;
	size_t m_uDispatchPerFrame = 16;
};