/**
 * @file ZombieAnimationClipID.h
 * @brief ゾンビのアニメーションクリップID
 * @details Assets/animData/character/zombie のアニメーションを、各ゾンビ(NormalZombie.cpp など)の ANIMATION_DATA と同じ順に並べる。
 *          値はそのままアニメーションクリップの並びの番号になる。(ActorStateMachine::PlayAnimation() が番号として使う)
 *          先頭の待機は、共通ステートが EnCharacterAnimationClipID の番号で再生するため、同じ位置に置く。
 *          歩き・走り・攻撃は、ゾンビ専用のステート(ZombieIState.h)が再生する。
 *          ゾンビの種類が違っても、同じ順に並べること。(ステートマシンが全種類で共通)
 */
#pragma once
#include <cstdint>

#include "Source/Actor/Character/CharacterAnimationClipID.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief ゾンビのアニメーションクリップID
         * @note 足す・並べ替えるときは、全ゾンビの ANIMATION_DATA も同じ順にすること。(数は static_assert で確かめる)
         *       ゾンビの種類だけが持つアニメーションは、この続きの番号から、種類ごとのenumで作る。
         */
        enum class EnZombieAnimationClipID : uint8_t
        {
            /** 待機(共通のクリップ。EnCharacterAnimationClipID と同じ並び) */
            Idle,
            /** 歩き */
            Walk,
            /** 走り */
            Run,
            /** 死亡(ループしない) */
            Death,
            /** 攻撃(ループしない) */
            Attack,

            Max
        };

        static_assert(static_cast<uint8_t>(EnZombieAnimationClipID::Idle) == static_cast<uint8_t>(EnCharacterAnimationClipID::Idle), "待機は共通のIDと同じ番号にすること");
    } // namespace actor
} // namespace app
