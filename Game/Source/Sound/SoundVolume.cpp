/**
 * @file SoundVolume.cpp
 * @brief 親子関係を持つ音量
 */
#include "stdafx.h"

#include "SoundVolume.h"


namespace app
{
    namespace
    {
        /** 音量の範囲 */
        constexpr float MIN_VOLUME = 0.0f;
        constexpr float MAX_VOLUME = 1.0f;
        /** 音量の初期値 */
        constexpr float DEFAULT_VOLUME = 1.0f;

        float ClampVolume(const float volume)
        {
            return std::clamp(volume, MIN_VOLUME, MAX_VOLUME);
        }
    } // namespace


    SoundVolume::SoundVolume(SoundVolume* parent)
        : m_parent(parent)
        , m_children()
        , m_volume(DEFAULT_VOLUME)
    {
        if (m_parent != nullptr)
        {
            m_parent->m_children.push_back(this);
        }
    }


    void SoundVolume::SetVolume(const float volume)
    {
        m_volume = ClampVolume(volume);

        NotifyVolumeChanged();
    }


    float SoundVolume::GetEffectiveVolume() const
    {
        if (m_parent == nullptr)
        {
            return m_volume;
        }
        return m_parent->GetEffectiveVolume() * m_volume;
    }


    void SoundVolume::NotifyVolumeChanged()
    {
        OnVolumeChanged();

        // 自分の音量は子の最終的な音量にも掛かるので、子にも伝える。
        for (SoundVolume* child : m_children)
        {
            child->NotifyVolumeChanged();
        }
    }
} // namespace app
