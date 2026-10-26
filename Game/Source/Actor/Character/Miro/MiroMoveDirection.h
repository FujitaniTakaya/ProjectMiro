/**
 * @file MiroMoveDirection.h
 * @brief Miroの移動方向(体の向きから見た8方向)
 * @details Miroは移動方向と関係なく、カメラの方向(Look)を向いたまま動く。(ストレイフ)
 *          アニメーションは、向いている方向から見て、どちらへ動いているかの8方向から選ぶ。
 */
#pragma once
#include <cfloat>
#include <cmath>
#include <cstdint>


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
    } // namespace actor
} // namespace app
