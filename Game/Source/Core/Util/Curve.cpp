/**
 * @file Curve.cpp
 * @brief イージング付きの補間(Curve)と二次ベジェ曲線
 */
#include "stdafx.h"

#include "Curve.h"


namespace app
{
    namespace util
    {
        namespace
        {
            /** 一度のUpdateで数える周回数の上限。uint16_tに収まる値 */
            constexpr float MAX_CYCLES = 65535.0f;
        } // namespace


        void CurveTimer::OnPeriodElapsed()
        {
            const float period = GetPeriod();
            if (m_repeatCount != InfiniteRepeat)
            {
                const float cycles = std::clamp(m_elapsed / period, 0.0f, MAX_CYCLES);
                const float remainCount = static_cast<float>(m_repeatCount - m_completedCount);
                if (cycles >= remainCount)
                {
                    Finish();
                    return;
                }
                m_completedCount = static_cast<uint16_t>(m_completedCount + static_cast<uint16_t>(cycles));
            }
            // 超過した時間は捨てずに持ち越す
            m_elapsed = std::fmod(m_elapsed, period);
        }


        void CurveTimer::Finish()
        {
            m_isPlaying = false;
            m_isFinished = true;
            if (m_endBehavior == EndBehavior::Reset)
            {
                // 頭に戻る。値は始点になる
                m_elapsed = 0.0f;
                m_completedCount = 0;
            }
            else
            {
                // 終了時の位置で止まる。Loop/Onceは終点、PingPongは始点
                m_elapsed = GetPeriod();
                m_completedCount = m_repeatCount;
            }
        }


        EasingType ToEasingType(const std::string_view name)
        {
            if (name == "EaseIn")
            {
                return EasingType::EaseIn;
            }
            if (name == "EaseOut")
            {
                return EasingType::EaseOut;
            }
            if (name == "EaseInOut")
            {
                return EasingType::EaseInOut;
            }
            return EasingType::Linear;
        }


        LoopMode ToLoopMode(const std::string_view name)
        {
            if (name == "Loop")
            {
                return LoopMode::Loop;
            }
            if (name == "PingPong")
            {
                return LoopMode::PingPong;
            }
            return LoopMode::Once;
        }


        // テンプレートの全メンバーを、使われていなくてもここでコンパイルする。
        // 型によってビルドが通らなくなるのを、使う側より先に見つけるため。
        template class Curve<float>;
        template class Curve<Vector2>;
        template class Curve<Vector3>;
        template class Curve<Vector4>;
        template class QuadraticBezierCurve<float>;
        template class QuadraticBezierCurve<Vector2>;
        template class QuadraticBezierCurve<Vector3>;
        template class QuadraticBezierCurve<Vector4>;
    } // namespace util
} // namespace app
