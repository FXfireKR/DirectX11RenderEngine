#pragma once

class CAudioSystem;

class CBgmController
{
private:
    enum class EState : uint8_t
    {
        IDLE,
        FADE_IN,
        PLAYING,
        FADE_OUT
    };

public:
    void Initialize(CAudioSystem* pAudio);
    void AddBgmTracks();
    void Start();
    void Update(float fDelta);

public:
    inline void SetUserVolume(float volume) { m_fUserVolume = std::clamp(volume, 0.0f, 1.0f); }
    inline float GetUserVolume() const { return m_fUserVolume; }

private:
    int _PickNextTrackIndex() const;
    void _PlayTrack(int index);

private:
    CAudioSystem* m_pAudio = nullptr;
    vector<SoundClipDef> m_vecTracks;

    FMOD::Channel* m_pCurrentChannel = nullptr;

    int m_iCurrentTrack = -1;
    int m_iNextTrack = -1;

    EState m_eState = EState::IDLE;

    float m_fCurrentGain = 0.0f;
    float m_fMaxGain = 1.0f;
    float m_fUserVolume = 0.65f;

    float m_fFadeInSec = 15.f;
    float m_fFadeOutSec = 25.0f;
    float m_fFadeOutLeadSec = 1.6f;
};