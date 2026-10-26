/**
 * @file MiroAnimationClipID.h
 * @brief Miroのアニメーションクリップ ID
 * @details Assets/animData/character/miro のアニメーションを、Miro.cpp の ANIMATION_DATA と同じ順に並べる。
 *          値はそのままアニメーションクリップの並びの番号になる。(ActorStateMachine::PlayAnimation() が番号として使う)
 *          先頭の待機は、共通ステートが EnCharacterAnimationClipID の番号で再生するため、同じ位置に置く。
 *          歩き・走りは、Miro専用のステート(MiroIState.h)が、移動方向(MiroMoveDirection.h)に合わせて選ぶ。
 * @note 歩き・走りのファイル名の左右(Left/Right)は、エンジン上の向き。3ds Max(右手系)から書き出した素材の左右とは逆になる。
 *       書き出し直したら、左右のファイル名を入れ替えること。
 */
#pragma once
#include <cstdint>

#include "Source/Actor/Character/CharacterAnimationClipID.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief Miroのアニメーションクリップ ID
         * @note 足す・並べ替えるときは、Miro.cpp の ANIMATION_DATA も同じ順にすること。(数は static_assert で確かめる)
         */
        enum class EnMiroAnimationClipID : uint8_t
        {
            /*
             * 共通のクリップ(EnCharacterAnimationClipID と同じ並び)
             */

            /** 待機 */
            Idle,


            /*
             * 歩き(walk/)
             */

            /** 歩き(前進) */
            WalkFwd,
            /** 歩き(前進・左) */
            WalkFwdLeft,
            /** 歩き(前進・右) */
            WalkFwdRight,
            /** 歩き(左) */
            WalkLeft,
            /** 歩き(右) */
            WalkRight,
            /** 歩き(後退) */
            WalkBwd,
            /** 歩き(後退・左) */
            WalkBwdLeft,
            /** 歩き(後退・右) */
            WalkBwdRight,


            /*
             * 走り(dash/)
             */

            /** 走り(前進) */
            RunFwd,
            /** 走り(前進・左) */
            RunFwdLeft,
            /** 走り(前進・右) */
            RunFwdRight,
            /** 走り(左) */
            RunLeft,
            /** 走り(右) */
            RunRight,
            /** 走り(後退) */
            RunBwd,
            /** 走り(後退・左) */
            RunBwdLeft,
            /** 走り(後退・右) */
            RunBwdRight,


            /*
             * ハンドガン(pistol/)
             */

            /** 撃つ 1 */
            GunShot01,
            /** 撃つ 2 */
            GunShot02,
            /** 撃つ 3 */
            GunShot03,
            /** 弾切れで撃つ */
            PistolDryFire,
            /** 構える */
            PistolEquip,
            /** リロード */
            PistolReload,

            Max
        };

        static_assert(static_cast<uint8_t>(EnMiroAnimationClipID::Idle) == static_cast<uint8_t>(EnCharacterAnimationClipID::Idle), "待機は共通のIDと同じ番号にすること");
    } // namespace actor
} // namespace app
