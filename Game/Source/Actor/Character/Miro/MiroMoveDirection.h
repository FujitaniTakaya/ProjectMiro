/**
 * @file MiroMoveDirection.h
 * @brief Miroの移動方向(体の向きから見た8方向)
 * @details Miroは移動方向と関係なく、カメラの方向(Look)を向いたまま動く。(ストレイフ)
 *          アニメーションは、向いている方向から見て、どちらへ動いているかの8方向から選ぶ。
 */
#pragma once
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>

#include "MiroAnimationClipID.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief 向いている方向から見た、移動方向
         * @details 前から右回りに並べる。45°ごとに1つ。
         */
        enum class EnMiroMoveDirection : uint8_t
        {
            /** 前 */
            Fwd,
            /** 前・右 */
            FwdRight,
            /** 右 */
            Right,
            /** 後ろ・右 */
            BwdRight,
            /** 後ろ */
            Bwd,
            /** 後ろ・左 */
            BwdLeft,
            /** 左 */
            Left,
            /** 前・左 */
            FwdLeft,

            Max
        };


        /**
         * @brief 歩き・走りのクリップの族(足の周期が揃っているもの)
         * @details 同じ族の中では、足の出る位相(クリップの割合)が揃っている。方向を切り替える時は、同じ族なら再生位置の割合を引き継げば、足が合う。
         *          前進と後退、歩きと走りは、足の出る位相がずれているので、別の族にする。(tka の足ボーンの周期から確かめた)
         */
        enum class EnMiroGait : uint8_t
        {
            /** 歩きでも走りでもない(待機、銃のクリップなど) */
            None,
            /** 歩き・前進(前、前・左、前・右) */
            WalkFwd,
            /** 歩き・後退(後ろ、後ろ・左、後ろ・右) */
            WalkBwd,
            /** 走り・前進 */
            RunFwd,
            /** 走り・後退 */
            RunBwd,
        };


        /**
         * @brief クリップの番号から、歩き・走りの族を求める
         * @details 真横(WalkLeft/WalkRight/RunLeft/RunRight)のクリップは、GetMiroWalkClip() などが使わず、足の位相も前進と合わないので、None にする。
         * @param clip アニメーションクリップの番号(EnMiroAnimationClipID の値)。-1 など Miro のクリップでない番号は None
         * @return 族
         */
        inline EnMiroGait GetMiroGait(const int clip)
        {
            switch (clip)
            {
            case static_cast<int>(EnMiroAnimationClipID::WalkFwd):
            case static_cast<int>(EnMiroAnimationClipID::WalkFwdLeft):
            case static_cast<int>(EnMiroAnimationClipID::WalkFwdRight):
                return EnMiroGait::WalkFwd;
            case static_cast<int>(EnMiroAnimationClipID::WalkBwd):
            case static_cast<int>(EnMiroAnimationClipID::WalkBwdLeft):
            case static_cast<int>(EnMiroAnimationClipID::WalkBwdRight):
                return EnMiroGait::WalkBwd;
            case static_cast<int>(EnMiroAnimationClipID::RunFwd):
            case static_cast<int>(EnMiroAnimationClipID::RunFwdLeft):
            case static_cast<int>(EnMiroAnimationClipID::RunFwdRight):
                return EnMiroGait::RunFwd;
            case static_cast<int>(EnMiroAnimationClipID::RunBwd):
            case static_cast<int>(EnMiroAnimationClipID::RunBwdLeft):
            case static_cast<int>(EnMiroAnimationClipID::RunBwdRight):
                return EnMiroGait::RunBwd;
            default:
                return EnMiroGait::None;
            }
        }


        /**
         * @brief 向いている方向から見た、移動方向を求める
         * @details 向いている方向と移動方向のなす角を、45°ごと(前を中心に±22.5°)に丸めて8方向にする。
         *          右は、PlayerController が使う cameraRight と同じ向き。(向いている方向を (x, z) として (z, -x))
         *          どちらかの水平方向の長さが0なら、前にする。
         * @param look 向いている方向。yは無視される。長さは問わない
         * @param move 移動方向。yは無視される。長さは問わない
         * @return 8方向のどれか
         */
        inline EnMiroMoveDirection CalcMiroMoveDirection(const Vector3& look, const Vector3& move)
        {
            constexpr float QUARTER_PI = Math::PI * 0.25f;
            constexpr uint8_t DIRECTION_COUNT = static_cast<uint8_t>(EnMiroMoveDirection::Max);

            // 向いている方向と移動方向の長さの2乗を求める。0に近い場合は前にする。
            const float lookLengthSq = look.x * look.x + look.z * look.z;
            const float moveLengthSq = move.x * move.x + move.z * move.z;
            if (lookLengthSq <= FLT_EPSILON || moveLengthSq <= FLT_EPSILON)
            {
                return EnMiroMoveDirection::Fwd;
            }

            // 向いている方向の前と右への成分。(長さは両方に同じだけ掛かるので、角度には影響しない)
            const float forwardAmount = move.x * look.x + move.z * look.z;
            const float rightAmount = move.x * look.z - move.z * look.x;
            const float angle = std::atan2(rightAmount, forwardAmount);

            // 45°ごとに丸める。(-4〜4。-4と4は同じ後ろ)
            const int sector = static_cast<int>(std::lround(angle / QUARTER_PI));
            return static_cast<EnMiroMoveDirection>((sector + DIRECTION_COUNT) % DIRECTION_COUNT);
        }


        /**
         * @brief 向いている方向から見た移動方向に合う、歩きのクリップを取得する
         * @details 歩きのステートと、構え・リロード中の下半身が使う。
         * @note 真横(Right/Left)は、WalkRight/WalkLeft ではなく斜め前のクリップを使う。
         *       WalkRight/WalkLeft は体が約60°横を向いたまま動く素材で、真横に歩くと後退のように見えるため。
         *       体を正面に向けた横歩きの素材ができたら、WalkRight/WalkLeft に戻す。
         * @param direction 向いている方向から見た移動方向
         * @return 歩きのアニメーションクリップID
         */
        inline EnMiroAnimationClipID GetMiroWalkClip(const EnMiroMoveDirection direction)
        {
            // EnMiroMoveDirection の順に並べること
            constexpr EnMiroAnimationClipID WALK_CLIPS[] = {
                EnMiroAnimationClipID::WalkFwd,
                EnMiroAnimationClipID::WalkFwdRight,
                EnMiroAnimationClipID::WalkFwdRight,
                EnMiroAnimationClipID::WalkBwdRight,
                EnMiroAnimationClipID::WalkBwd,
                EnMiroAnimationClipID::WalkBwdLeft,
                EnMiroAnimationClipID::WalkFwdLeft,
                EnMiroAnimationClipID::WalkFwdLeft,
            };
            static_assert(std::size(WALK_CLIPS) == static_cast<size_t>(EnMiroMoveDirection::Max), "歩きのクリップの数を方向と合わせること");
            return WALK_CLIPS[static_cast<size_t>(direction)];
        }


        /**
         * @brief 向いている方向から見た移動方向に合う、走りのクリップを取得する
         * @note 真横(Right/Left)は、歩きと同じ理由で、RunRight/RunLeft ではなく斜め前のクリップを使う。
         * @param direction 向いている方向から見た移動方向
         * @return 走りのアニメーションクリップID
         */
        inline EnMiroAnimationClipID GetMiroRunClip(const EnMiroMoveDirection direction)
        {
            // EnMiroMoveDirection の順に並べること
            constexpr EnMiroAnimationClipID RUN_CLIPS[] = {
                EnMiroAnimationClipID::RunFwd,
                EnMiroAnimationClipID::RunFwdRight,
                EnMiroAnimationClipID::RunFwdRight,
                EnMiroAnimationClipID::RunBwdRight,
                EnMiroAnimationClipID::RunBwd,
                EnMiroAnimationClipID::RunBwdLeft,
                EnMiroAnimationClipID::RunFwdLeft,
                EnMiroAnimationClipID::RunFwdLeft,
            };
            static_assert(std::size(RUN_CLIPS) == static_cast<size_t>(EnMiroMoveDirection::Max), "走りのクリップの数を方向と合わせること");
            return RUN_CLIPS[static_cast<size_t>(direction)];
        }
    } // namespace actor
} // namespace app
