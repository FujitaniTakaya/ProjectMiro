/**
 * @file SoundGroup.cpp
 * @brief 音を再生するサウンドのグループ
 */
#include "stdafx.h"

#include "SoundGroup.h"


namespace app
{
    namespace
    {
        /** NewGOするときのオブジェクト名 */
        constexpr const char* SOUND_GO_NAME = "sound";

        /**
         * 次に割り当てるハンドル
         * グループをまたいで重複しないように、全てのグループで共有する。(別のグループの音を止めてしまわないため。)
         */
        SoundHandle soundHandleCount = 0;
    } // namespace


    SoundGroup::SoundGroup(SoundVolume* parent)
        : SoundVolume(parent)
        , m_sounds()
    {
    }


    SoundGroup::~SoundGroup()
    {
        // 再生中の音を止める。(SoundSourceの破棄はエンジンが行う。)
        StopAll();
    }


    void SoundGroup::Update(const float deltaTime)
    {
        for (auto it = m_sounds.begin(); it != m_sounds.end();)
        {
            PlayingSound& sound = it->second;

            // NOTE: 再生が終わったSoundSourceは、エンジンが自分でDeleteGOする。
            //       実際のdeleteは次のExecuteUpdateの冒頭なので、その前にリストから外しておく。
            if (!sound.m_source->IsPlaying())
            {
                DeleteGO(sound.m_source);
                it = m_sounds.erase(it);
                continue;
            }

            // フェードを進める。一時停止中の音は、フェードも止まる。
            if (!sound.m_isPaused)
            {
                if (sound.m_fader.IsFading())
                {
                    sound.m_fader.Update(deltaTime);
                    sound.m_source->SetVolume(CalcSoundVolume(sound));
                }

                // フェードアウトが終わったら止める。
                if (sound.m_isStopAtFadeEnd && !sound.m_fader.IsFading())
                {
                    StopSource(sound.m_source);
                    it = m_sounds.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }


    SoundHandle SoundGroup::Play(const EnSoundID id, const float volumeMagnification, const bool isLoop, const float fadeInSeconds)
    {
        if (!IsPlayableSoundID(id))
        {
            return INVALID_SOUND_HANDLE;
        }
        if (soundHandleCount == INVALID_SOUND_HANDLE)
        {
            K2_ASSERT(false, "サウンドの再生が多いです。\n");
            return INVALID_SOUND_HANDLE;
        }

        // フェードインする場合は、割合0から始めて、目標の1.0まで上げていく。
        const bool isFadeIn = fadeInSeconds > 0.0f;
        const float initialFadeRate = isFadeIn ? 0.0f : 1.0f;

        SoundSource* source = CreateAndPlay(id, isLoop, GetEffectiveVolume() * volumeMagnification * initialFadeRate);
        if (source == nullptr)
        {
            return INVALID_SOUND_HANDLE;
        }

        const SoundHandle handle = soundHandleCount++;
        auto result = m_sounds.emplace(handle, PlayingSound(source, volumeMagnification, initialFadeRate));
        if (isFadeIn)
        {
            result.first->second.m_fader.FadeTo(1.0f, fadeInSeconds);
        }
        return handle;
    }


    void SoundGroup::Stop(const SoundHandle handle)
    {
        auto it = m_sounds.find(handle);
        if (it == m_sounds.end())
        {
            return;
        }
        StopSource(it->second.m_source);
        m_sounds.erase(it);
    }


    void SoundGroup::StopAll()
    {
        for (auto& it : m_sounds)
        {
            StopSource(it.second.m_source);
        }
        m_sounds.clear();
    }


    void SoundGroup::FadeOut(const SoundHandle handle, const float seconds)
    {
        if (seconds <= 0.0f)
        {
            Stop(handle);
            return;
        }

        auto it = m_sounds.find(handle);
        if (it == m_sounds.end())
        {
            return;
        }
        StartFadeOut(it->second, seconds);
    }


    void SoundGroup::FadeOutAll(const float seconds)
    {
        if (seconds <= 0.0f)
        {
            StopAll();
            return;
        }

        for (auto& it : m_sounds)
        {
            StartFadeOut(it.second, seconds);
        }
    }


    void SoundGroup::Pause()
    {
        for (auto& it : m_sounds)
        {
            PauseSound(it.second);
        }
    }


    void SoundGroup::Resume()
    {
        for (auto& it : m_sounds)
        {
            ResumeSound(it.second);
        }
    }


    void SoundGroup::Pause(const SoundHandle handle)
    {
        auto it = m_sounds.find(handle);
        if (it != m_sounds.end())
        {
            PauseSound(it->second);
        }
    }


    void SoundGroup::Resume(const SoundHandle handle)
    {
        auto it = m_sounds.find(handle);
        if (it != m_sounds.end())
        {
            ResumeSound(it->second);
        }
    }


    void SoundGroup::OnVolumeChanged()
    {
        for (auto& it : m_sounds)
        {
            it.second.m_source->SetVolume(CalcSoundVolume(it.second));
        }
    }


    bool SoundGroup::IsPlayableSoundID(const EnSoundID id)
    {
        if (id < EnSoundID::Max)
        {
            return true;
        }

        // Noneは「鳴らさない」の意味なので黙って弾く。それ以外の範囲外は、呼び出し側のバグ。
        K2_ASSERT(id == EnSoundID::None, "範囲外のサウンドIDです。");
        return false;
    }


    float SoundGroup::CalcSoundVolume(const PlayingSound& sound) const
    {
        return GetEffectiveVolume() * sound.m_volumeMagnification * sound.m_fader.Get();
    }


    SoundSource* SoundGroup::CreateAndPlay(const EnSoundID id, const bool isLoop, const float volume)
    {
        SoundSource* source = NewGO<SoundSource>(0, SOUND_GO_NAME);
        source->Init(static_cast<int>(id));

        // 登録されていないIDなどで音源を作れなかった場合は、SetVolume()などでnullptrを触ってしまうので再生しない。
        if (source->GetXAudio2SourceVoice() == nullptr)
        {
            DeleteGO(source);
            return nullptr;
        }
        source->SetVolume(volume);
        source->Play(isLoop);
        return source;
    }


    void SoundGroup::StopSource(SoundSource* source)
    {
        source->Stop();
        DeleteGO(source);
    }


    void SoundGroup::PauseSound(PlayingSound& sound)
    {
        // 再生が終わった音は止めない。(再開したときに、最初から鳴り直してしまうため。)
        if (sound.m_isPaused || !sound.m_source->IsPlaying())
        {
            return;
        }
        sound.m_source->Pause();
        sound.m_isPaused = true;
    }


    void SoundGroup::ResumeSound(PlayingSound& sound)
    {
        if (!sound.m_isPaused)
        {
            return;
        }
        sound.m_isPaused = false;

        // NOTE: 再生中のSoundSourceにPlay()を呼ぶと、続きから再生される。
        if (sound.m_source->IsPlaying())
        {
            sound.m_source->Play(sound.m_source->GetLoopFlag());
        }
    }


    void SoundGroup::StartFadeOut(PlayingSound& sound, const float seconds)
    {
        sound.m_isStopAtFadeEnd = true;
        sound.m_fader.FadeTo(0.0f, seconds);
    }
} // namespace app
