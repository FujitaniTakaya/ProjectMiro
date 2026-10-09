/**
 * @file CharacterAnimationClipID.h
 * @brief キャラクター共通のアニメーションクリップID
 * @details 値はそのままアニメーションクリップの並びの番号になる。(ActorStateMachine::PlayAnimation() が番号として使う)
 *          共通ステートは、この値だけでアニメーションを再生する。
 *          そのため、各キャラクターの ModelData::animationData の先頭を、この順に並べること。
 *          キャラクター固有のアニメーションは、この続きの番号から別のenumで作る。
 *          例: enum class EnMiroAnimationClipID : uint8_t { Attack = CHARACTER_ANIMATION_CLIP_COUNT, Max };
 */
#pragma once
#include <cstdint>


namespace app
{
    namespace actor
    {
        /**
         * @brief キャラクター共通のアニメーションクリップID
         * @note 全キャラクターが持つものだけを置く。足すときは、全キャラクターの animationData の同じ位置にも足すこと。
         */
        enum class EnCharacterAnimationClipID : uint8_t
        {
            /** 待機 */
            Idle,
            /** 歩き */
            Walk,
            /** 走り */
            Run,

            Max
        };

        /**
         * 共通のアニメーションクリップの数
         * 固有のアニメーションIDの開始番号になる。(enum class は整数に暗黙変換できないため、定数に受けて使う)
         */
        constexpr uint8_t CHARACTER_ANIMATION_CLIP_COUNT = static_cast<uint8_t>(EnCharacterAnimationClipID::Max);
    } // namespace actor
} // namespace app
