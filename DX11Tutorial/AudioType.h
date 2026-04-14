#pragma once
#include <cstdint>
#include <DirectXMath.h>

using SoundID = uint64_t;
using SoundEventID = uint64_t;

enum class EAudioBus
{
	MASTER = 0,
	BGM,
	SFX,
	AMBIENT,

	COUNT,
};
constexpr size_t AUDIO_BUS_COUNT = static_cast<size_t>(EAudioBus::COUNT);

struct AudioListenerState
{
	DirectX::XMFLOAT3 pos{};
	DirectX::XMFLOAT3 vel{};
	DirectX::XMFLOAT3 forward{ 0.f, 0.f, 1.f };
	DirectX::XMFLOAT3 up{ 0.f, 1.f, 0.f };
};

struct AudioPlayDesc
{
	bool bLoop = false;
	bool b3D = false;

	float volume = 1.f;
	float pitch = 1.f;

	float minDistance = 1.f;
	float maxDistance = 32.f;
	EAudioBus bus = EAudioBus::SFX;
};

struct AudioLoadDesc
{
	bool bLoop = false;
	bool b3D = false;
	bool bStream = false;
};

struct AudioLoadElemDesc
{
	SoundID id;
	string objectPath;
	AudioLoadDesc desc;
};

struct AudioRequest
{
	bool b3D = false;
	SoundID id = 0;

	XMFLOAT3 pos{};

	float volume = 1.f;
	float pitch = 1.f;

	float minDistance = 1.f;
	float maxDistance = 24.f;

	EAudioBus bus = EAudioBus::SFX;
};