#include "pch.h"
#include "CBgmController.h"
#include "CAudioSystem.h"

void CBgmController::Initialize(CAudioSystem* pAudio)
{
    m_pAudio = pAudio;
    m_vecTracks.clear();
    m_pCurrentChannel = nullptr;
    m_iCurrentTrack = -1;
    m_iNextTrack = -1;
    m_eState = EState::IDLE;
    m_fCurrentGain = 0.0f;
    m_fMaxGain = 1.0f;
}

void CBgmController::AddBgmTracks()
{
    // TODO : split index 가 가능한 형태 (A).(B).(C)로 바꾸면 좋을 듯.
    ResolvedSound reSound{};
    if (BlockResDB.ResolveEvent("music.overworld.cherry_grove", reSound))
    {
        auto* soundPack = BlockResDB.FindEvent("music.overworld.cherry_grove");
        if (soundPack)
        {
            m_vecTracks = soundPack->clips;
        }
    }

    Start();
}

void CBgmController::Start()
{
    if (m_vecTracks.empty())
        return;

    const int first = _PickNextTrackIndex();
    _PlayTrack(first);
}

void CBgmController::Update(float fDelta)
{
    if (!m_pAudio || !m_pCurrentChannel)
        return;

    bool bPlaying = m_pAudio->IsChannelPlaying(m_pCurrentChannel);
    if (!bPlaying)
    {
        if (m_iNextTrack >= 0)
            _PlayTrack(m_iNextTrack);
        return;
    }

    unsigned int posMs = 0;
    unsigned int lenMs = 0;
    m_pCurrentChannel->getPosition(&posMs, FMOD_TIMEUNIT_MS);

    FMOD::Sound* pSound = nullptr;
    m_pCurrentChannel->getCurrentSound(&pSound);
    if (pSound)
        pSound->getLength(&lenMs, FMOD_TIMEUNIT_MS);

    const float remainSec = (lenMs > posMs) ? (lenMs - posMs) / 1000.0f : 0.0f;

    switch (m_eState)
    {
    case EState::FADE_IN:
    {
        m_fCurrentGain += (fDelta / m_fFadeInSec);
        if (m_fCurrentGain >= m_fMaxGain)
        {
            m_fCurrentGain = m_fMaxGain;
            m_eState = EState::PLAYING;
        }
    } break;

    case EState::PLAYING:
    {
        if (remainSec <= m_fFadeOutLeadSec)
            m_eState = EState::FADE_OUT;
    } break;

    case EState::FADE_OUT:
    {
        m_fCurrentGain -= (fDelta / m_fFadeOutSec);
        if (m_fCurrentGain <= 0.0f)
        {
            m_fCurrentGain = 0.0f;
            m_pAudio->StopChannel(m_pCurrentChannel, false);
            m_pCurrentChannel = nullptr;

            if (m_iNextTrack >= 0)
                _PlayTrack(m_iNextTrack);
            return;
        }
    } break;

    case EState::IDLE:
    default:
        break;
    }

    if (m_pCurrentChannel)
        m_pCurrentChannel->setVolume(m_fCurrentGain * m_fUserVolume);
}

int CBgmController::_PickNextTrackIndex() const
{
    if (m_vecTracks.empty())
        return -1;

    if (m_vecTracks.size() == 1)
        return 0;

    int idx = rand() % static_cast<int>(m_vecTracks.size());
    if (idx == m_iCurrentTrack)
        idx = (idx + 1) % static_cast<int>(m_vecTracks.size());

    return idx;
}

void CBgmController::_PlayTrack(int index)
{
    if (!m_pAudio)
        return;
    if (index < 0 || index >= static_cast<int>(m_vecTracks.size()))
        return;

    const auto& currentBGM = m_vecTracks[index];
    m_pCurrentChannel = m_pAudio->PlayBGMNow(currentBGM.soundID, currentBGM.objectPath.c_str(), 0.0f);
    if (!m_pCurrentChannel)
        return;

    m_iCurrentTrack = index;
    m_iNextTrack = _PickNextTrackIndex();
    m_fCurrentGain = 0.0f;
    m_fMaxGain = currentBGM.volumeMul;
    m_eState = EState::FADE_IN;
}