/**
 * @file Curve.h
 * @brief イージング付きの補間(Curve)と二次ベジェ曲線
 */
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string_view>


namespace app
{
    namespace util
    {
        /**
         * @brief イージングの種類
         * @details 線形補間、イーズイン(加速)、イーズアウト(減速)、イーズインアウト(加速して減速)
         */
        enum class EasingType : uint8_t
        {
            Linear,
            EaseIn,
            EaseOut,
            EaseInOut
        };


        /**
         * @brief ループの種類
         * @details 片道、周回(始点→終点を繰り返す)、往復(始点→終点→始点を繰り返す)
         */
        enum class LoopMode : uint8_t
        {
            Once,
            Loop,
            PingPong
        };


        /**
         * @brief 再生が最後まで終わったときの動作
         * @details Hold: 終了時の値を保持して止まる。次のPlay()で頭から再生し直す。
         *          Reset: 終了した瞬間に頭(始点)へ戻って止まる。そのままPlay()で再生し直せる。
         */
        enum class EndBehavior : uint8_t
        {
            Hold,
            Reset
        };


        /** 繰り返し回数に指定すると、止まらずに繰り返し続ける */
        constexpr uint16_t InfiniteRepeat = 0;


        /**
         * @brief イージングを適用する
         * @param type イージングの種類
         * @param t 進行度。0.0f〜1.0f
         * @return イージング後の進行度。t=0で0、t=1で1になる。
         */
        inline float ApplyEasing(const EasingType type, const float t)
        {
            switch (type)
            {
            case EasingType::EaseIn:
                return t * t;
            case EasingType::EaseOut:
                return t * (2.0f - t);
            case EasingType::EaseInOut:
                return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
            case EasingType::Linear:
            default:
                return t;
            }
        }


        /**
         * @brief 文字列からイージングの種類を取得する
         * @param name "Linear" "EaseIn" "EaseOut" "EaseInOut"
         * @return イージングの種類。知らない文字列はLinear
         */
        EasingType ToEasingType(const std::string_view name);


        /**
         * @brief 文字列からループの種類を取得する
         * @param name "Once" "Loop" "PingPong"
         * @return ループの種類。知らない文字列はOnce
         */
        LoopMode ToLoopMode(const std::string_view name);


        namespace detail
        {
            /** floatの線形補間。t=0でa、t=1でbにぴったり一致する */
            inline float LerpValue(const float a, const float b, const float t)
            {
                return a * (1.0f - t) + b * t;
            }

            /** Vector2の線形補間 */
            inline Vector2 LerpValue(const Vector2& a, const Vector2& b, const float t)
            {
                const float s = 1.0f - t;
                return Vector2(a.x * s + b.x * t, a.y * s + b.y * t);
            }

            /** Vector3の線形補間 */
            inline Vector3 LerpValue(const Vector3& a, const Vector3& b, const float t)
            {
                const float s = 1.0f - t;
                return Vector3(a.x * s + b.x * t, a.y * s + b.y * t, a.z * s + b.z * t);
            }

            /** Vector4の線形補間 */
            inline Vector4 LerpValue(const Vector4& a, const Vector4& b, const float t)
            {
                const float s = 1.0f - t;
                return Vector4(a.x * s + b.x * t, a.y * s + b.y * t, a.z * s + b.z * t, a.w * s + b.w * t);
            }


            /** floatの重み付き和。p0*w0 + p1*w1 + p2*w2 */
            inline float WeightedSum(const float p0, const float p1, const float p2, const float w0, const float w1, const float w2)
            {
                return p0 * w0 + p1 * w1 + p2 * w2;
            }

            /** Vector2の重み付き和 */
            inline Vector2 WeightedSum(const Vector2& p0, const Vector2& p1, const Vector2& p2, const float w0, const float w1, const float w2)
            {
                return Vector2(
                    p0.x * w0 + p1.x * w1 + p2.x * w2,
                    p0.y * w0 + p1.y * w1 + p2.y * w2
                );
            }

            /** Vector3の重み付き和 */
            inline Vector3 WeightedSum(const Vector3& p0, const Vector3& p1, const Vector3& p2, const float w0, const float w1, const float w2)
            {
                return Vector3(
                    p0.x * w0 + p1.x * w1 + p2.x * w2,
                    p0.y * w0 + p1.y * w1 + p2.y * w2,
                    p0.z * w0 + p1.z * w1 + p2.z * w2
                );
            }

            /** Vector4の重み付き和 */
            inline Vector4 WeightedSum(const Vector4& p0, const Vector4& p1, const Vector4& p2, const float w0, const float w1, const float w2)
            {
                return Vector4(
                    p0.x * w0 + p1.x * w1 + p2.x * w2,
                    p0.y * w0 + p1.y * w1 + p2.y * w2,
                    p0.z * w0 + p1.z * w1 + p2.z * w2,
                    p0.w * w0 + p1.w * w1 + p2.w * w2
                );
            }
        } // namespace detail


        /**
         * @brief Curveの時間管理
         * @details 再生・停止・ループ・繰り返し回数・終了後の動作をまとめて持つ。
         *          値の補間はしない。進行度(0.0f〜1.0f)を返すだけ。
         */
        class CurveTimer
        {
        public:
            CurveTimer()
                : m_duration(1.0f)
                , m_elapsed(0.0f)
                , m_repeatCount(ONCE_REPEAT_COUNT)
                , m_completedCount(0)
                , m_loopMode(LoopMode::Once)
                , m_endBehavior(EndBehavior::Hold)
                , m_isPlaying(false)
                , m_isFinished(false)
            {
            }


            /**
             * @brief 時間の設定をして、頭に戻す
             * @details 再生状態(再生中かどうか)と終了後の動作は変えない。
             * @param durationSec 1周にかける時間(秒)。PingPongは片道にかける時間
             * @param loopMode ループの種類
             * @param repeatCount 繰り返す回数。InfiniteRepeatで無限。Onceは常に1回
             */
            void Setup(const float durationSec, const LoopMode loopMode, const uint16_t repeatCount)
            {
                // NOTE: Windows.hのmax/minマクロと衝突するため、std::maxは使わない
                m_duration = durationSec > MIN_DURATION ? durationSec : MIN_DURATION;
                m_loopMode = loopMode;
                m_repeatCount = loopMode == LoopMode::Once ? ONCE_REPEAT_COUNT : repeatCount;
                Rewind();
            }


            /**
             * @brief 再生する
             * @details 終了済みなら頭に戻してから再生する。途中で止めていたなら続きから再生する。
             */
            void Play()
            {
                if (m_isFinished)
                {
                    Rewind();
                }
                m_isPlaying = true;
            }


            /** 止める。途中で止めただけなので、Play()で続きから再生できる */
            void Stop()
            {
                m_isPlaying = false;
            }


            /** 頭に戻して止める。終了後の動作に関係なく、必ず頭に戻る */
            void Reset()
            {
                Rewind();
                m_isPlaying = false;
            }


            /**
             * @brief 時間を進める
             * @param deltaTime 進める時間(秒)。0以下は何もしない
             */
            void Update(const float deltaTime)
            {
                // NaNも弾くため、「0より大きい」の否定で判定する
                if (!m_isPlaying || !(deltaTime > 0.0f))
                {
                    return;
                }

                m_elapsed += deltaTime;
                if (m_elapsed >= GetPeriod())
                {
                    // 1周以上進んだときだけ呼ぶ。毎フレームの経路を小さく保つため、別関数にしている。
                    OnPeriodElapsed();
                }
            }


            /** 終了後の動作を設定する */
            void SetEndBehavior(const EndBehavior endBehavior)
            {
                m_endBehavior = endBehavior;
            }


            /** 終了後の動作を取得する */
            EndBehavior GetEndBehavior() const
            {
                return m_endBehavior;
            }


            /** 再生中か取得する */
            bool IsPlaying() const
            {
                return m_isPlaying;
            }


            /**
             * @brief 直近の再生が最後まで終わったか取得する
             * @details 終了するとtrue。Play() Reset() Setup()でfalseに戻る。
             *          無限に繰り返しているときや、途中で止めたときはfalse。
             */
            bool IsFinished() const
            {
                return m_isFinished;
            }


            /**
             * @brief 進行度を取得する
             * @return 0.0f〜1.0f。PingPongの復路は1.0fから0.0fへ戻る
             */
            float GetProgress() const
            {
                if (m_loopMode == LoopMode::PingPong && m_elapsed > m_duration)
                {
                    return (m_duration * 2.0f - m_elapsed) / m_duration;
                }
                return m_elapsed / m_duration;
            }


        private:
            /** 1周の長さ(秒)。PingPongは往復で1周 */
            float GetPeriod() const
            {
                return m_loopMode == LoopMode::PingPong ? m_duration * 2.0f : m_duration;
            }


            /** 頭に戻す。再生状態は変えない */
            void Rewind()
            {
                m_elapsed = 0.0f;
                m_completedCount = 0;
                m_isFinished = false;
            }


            /**
             * @brief 1周以上進んだときの処理
             * @details 何周進んだかを数えて、回数を使い切ったら終了する。使い切っていなければ、超過した時間を持ち越して頭に折り返す。
             *          大きなdeltaTimeで何周もまたいでも、回数がずれないようにしている。
             */
            void OnPeriodElapsed();


            /** 最後まで終わったときの処理 */
            void Finish();


        private:
            /** 時間の最小値(秒)。0で割らないための下限 */
            static constexpr float MIN_DURATION = 0.0001f;
            /** Onceの繰り返し回数。片道を1回だけ再生する */
            static constexpr uint16_t ONCE_REPEAT_COUNT = 1;

            /** 時間の間隔(秒) */
            float m_duration;
            /** 現在の周の経過時間(秒)。0〜1周の長さ */
            float m_elapsed;
            /** 繰り返す回数。InfiniteRepeatで無限 */
            uint16_t m_repeatCount;
            /** 終わった周回数 */
            uint16_t m_completedCount;
            /** ループの種類 */
            LoopMode m_loopMode;
            /** 終了後の動作 */
            EndBehavior m_endBehavior;
            /** 再生中か */
            bool m_isPlaying;
            /** 直近の再生が最後まで終わったか */
            bool m_isFinished;
        };


        /**
         * @brief 始点から終点までをイージングで補間するCurve
         * @tparam T float Vector2 Vector3 Vector4
         */
        template <typename T>
        class Curve
        {
        public:
            Curve()
                : m_startValue()
                , m_endValue()
                , m_timer()
                , m_easingType(EasingType::Linear)
            {
            }


            /**
             * @brief 初期化。頭に戻る
             * @details 再生状態は変えないので、再生するにはPlay()を呼ぶ。
             * @param start 始める数値
             * @param end 終わる数値
             * @param timeSec 時間(秒)
             * @param type イージングの種類
             * @param loopMode ループの種類
             * @param repeatCount 繰り返す回数(Loop PingPong用)。InfiniteRepeatで無限
             */
            void Initialize(
                const T& start,
                const T& end,
                const float timeSec,
                const EasingType type = EasingType::EaseOut,
                const LoopMode loopMode = LoopMode::Once,
                const uint16_t repeatCount = InfiniteRepeat
            )
            {
                m_startValue = start;
                m_endValue = end;
                m_easingType = type;
                m_timer.Setup(timeSec, loopMode, repeatCount);
            }


            /** 再生する。終了済みなら頭から再生し直す */
            void Play()
            {
                m_timer.Play();
            }

            /** 止める。Play()で続きから再生できる */
            void Stop()
            {
                m_timer.Stop();
            }

            /** 頭に戻して止める */
            void Reset()
            {
                m_timer.Reset();
            }

            /** 更新 */
            void Update(const float deltaTime)
            {
                m_timer.Update(deltaTime);
            }

            /** 終了後の動作を設定する */
            void SetEndBehavior(const EndBehavior endBehavior)
            {
                m_timer.SetEndBehavior(endBehavior);
            }

            /** 再生中か取得する */
            bool IsPlaying() const
            {
                return m_timer.IsPlaying();
            }

            /** 直近の再生が最後まで終わったか取得する */
            bool IsFinished() const
            {
                return m_timer.IsFinished();
            }


            /** 現在の値を取得する */
            T GetCurrentValue() const
            {
                const float rate = ApplyEasing(m_easingType, m_timer.GetProgress());
                return detail::LerpValue(m_startValue, m_endValue, rate);
            }


        private:
            /** 始める数値 */
            T m_startValue;
            /** 終わる数値 */
            T m_endValue;
            /** 時間管理 */
            CurveTimer m_timer;
            /** イージングの種類 */
            EasingType m_easingType;
        };


        /** float型のCurve */
        using FloatCurve = Curve<float>;
        /** Vector2型のCurve */
        using Vector2Curve = Curve<Vector2>;
        /** Vector3型のCurve */
        using Vector3Curve = Curve<Vector3>;
        /** Vector4型のCurve */
        using Vector4Curve = Curve<Vector4>;


        /**
         * @brief 二次ベジェ曲線
         * @details 始点、制御点、終点の3点で曲線を描く。制御点は曲がる方向と強さを決める点で、曲線は通らない。
         *          Curveと同じ時間管理を持つ(イージングは無し)。
         * @tparam T float Vector2 Vector3 Vector4
         */
        template <typename T>
        class QuadraticBezierCurve
        {
        public:
            QuadraticBezierCurve()
                : m_startValue()
                , m_controlValue()
                , m_endValue()
                , m_timer()
            {
            }


            /**
             * @brief 初期化。頭に戻る
             * @details 再生状態は変えないので、再生するにはPlay()を呼ぶ。
             * @param start 始める位置
             * @param control 制御点。この点に向かって曲がる
             * @param end 終わる位置
             * @param timeSec 時間(秒)
             * @param loopMode ループの種類
             * @param repeatCount 繰り返す回数(Loop PingPong用)。InfiniteRepeatで無限
             */
            void Initialize(
                const T& start,
                const T& control,
                const T& end,
                const float timeSec,
                const LoopMode loopMode = LoopMode::Once,
                const uint16_t repeatCount = InfiniteRepeat
            )
            {
                m_startValue = start;
                m_controlValue = control;
                m_endValue = end;
                m_timer.Setup(timeSec, loopMode, repeatCount);
            }


            /** 再生する。終了済みなら頭から再生し直す */
            void Play()
            {
                m_timer.Play();
            }

            /** 止める。Play()で続きから再生できる */
            void Stop()
            {
                m_timer.Stop();
            }

            /** 頭に戻して止める */
            void Reset()
            {
                m_timer.Reset();
            }

            /** 更新 */
            void Update(const float deltaTime)
            {
                m_timer.Update(deltaTime);
            }

            /** 終了後の動作を設定する */
            void SetEndBehavior(const EndBehavior endBehavior)
            {
                m_timer.SetEndBehavior(endBehavior);
            }

            /** 再生中か取得する */
            bool IsPlaying() const
            {
                return m_timer.IsPlaying();
            }

            /** 直近の再生が最後まで終わったか取得する */
            bool IsFinished() const
            {
                return m_timer.IsFinished();
            }


            /** 現在の値(座標)を取得する */
            T GetCurrentValue() const
            {
                // (1-t)^2 * P0 + 2(1-t)t * P1 + t^2 * P2。成分ごとに計算するので、演算子の無い型でも使える。
                // t=0で重みが(1,0,0)、t=1で(0,0,1)になるので、両端は始点・終点にぴったり一致する。
                const float t = m_timer.GetProgress();
                const float u = 1.0f - t;
                return detail::WeightedSum(m_startValue, m_controlValue, m_endValue, u * u, 2.0f * u * t, t * t);
            }


        private:
            /** 始める数値(P0) */
            T m_startValue;
            /** 制御点(P1) */
            T m_controlValue;
            /** 終わる数値(P2) */
            T m_endValue;
            /** 時間管理 */
            CurveTimer m_timer;
        };


        /** Vector2型の二次ベジェ曲線 */
        using Vector2BezierCurve = QuadraticBezierCurve<Vector2>;
        /** Vector3型の二次ベジェ曲線 */
        using Vector3BezierCurve = QuadraticBezierCurve<Vector3>;
    } // namespace util
} // namespace app
