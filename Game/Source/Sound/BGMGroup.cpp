/**
 * @file BGMGroup.cpp
 * @brief BGMのサウンドのグループ
 */
#include "stdafx.h"

#include "BGMGroup.h"


namespace app
{
    BGMGroup::BGMGroup(SoundVolume* parent)
        : SoundGroup(parent)
    {
    }


    SoundHandle BGMGroup::Play(const EnSoundID id, const float volumeMagnification, const float fadeSeconds)
    {
        // 再生できないIDなら、再生中のBGMを止めずに終わる。
        if (!IsPlayableSoundID(id))
        {
            return INVALID_SOUND_HANDLE;
        }

        // BGMはゲーム上に1つしか存在しないので、再生中のBGMがあれば、フェードアウトさせて置き換える。
        // (0秒以下なら、すぐに止まる。)
        FadeOutAll(fadeSeconds);

        return SoundGroup::Play(id, volumeMagnification, /*isLoop=*/true, fadeSeconds);
    }
} // namespace app
