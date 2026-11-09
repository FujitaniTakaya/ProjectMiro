/**
 * @file ZombieAnimationClipID.h
 * @brief ゾンビ固有のアニメーションクリップID
 * @details 共通のアニメーション(待機・歩き・走り。CharacterAnimationClipID.h)の続きの番号から始まる。
 *          ゾンビの ModelData::animationData は、先頭を共通の3つ(EnCharacterAnimationClipID の順)、
 *          その後ろをこのenumの順に並べること。
 */
#pragma once
#include <cstdint>

#include "Source/Actor/Character/CharacterAnimationClipID.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief ゾンビ固有のアニメーションクリップID
         * @note 足すときは、全ゾンビの animationData の同じ位置にも足すこと。
         *       ゾンビの種類だけが持つアニメーションは、この続きの番号から、種類ごとのenumで作る。
         */
        enum class EnZombieAnimationClipID : uint8_t
        {
            /** 攻撃 */
            Attack = CHARACTER_ANIMATION_CLIP_COUNT,

            Max
        };
    } // namespace actor
} // namespace app
