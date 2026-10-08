/**
 * @file UIAnimation.h
 * @brief UIのパーツを動かすアニメーション
 * @details UIBaseに登録して使う。登録したアニメーションは、UIのUpdate()で更新される。(UIBase::UpdateAnimation())
 *          UIAnimationFactoryで、jsonの定義(UIAnimationParameter)から作れる。
 *          例: UIAnimationFactory::Attach<UITranslateAnimation>(ui, Hash32("slideIn"));
 *              ui->FindAnimation(Hash32("slideIn"))->PlayAnimation();
 */
#pragma once
#include <cstdint>
#include <functional>
#include <vector>

#include "Source/Util/Curve.h"


namespace app
{
    namespace ui
    {
        /** 前方宣言 */
        class UIBase;
        struct UIAnimationDef;


        /** 値をUIに反映する関数 */
        template <typename T>
        using UIAnimationApplyFunc = std::function<void(const T&)>;




        /**
         * @brief UIアニメーションの基底クラス
         */
        class UIAnimationBase : public Noncopyable
        {
        public:
            UIAnimationBase();
            virtual ~UIAnimationBase();


        public:
            /** 更新。再生中なら時間を進めて、値をUIに反映する。 */
            virtual void Update() = 0;

            /** 再生 */
            virtual void PlayAnimation() = 0;

            /** 停止。再生し直すと、続きから再生する。 */
            virtual void StopAnimation() = 0;

            /**
             * @brief 再生中かどうか
             * @return 再生中ならtrue
             */
            virtual bool IsPlayAnimation() const = 0;


        public:
            /**
             * @brief アニメーションを動かすUIを設定
             * @details UIBase::AddAnimation()が呼ぶ。
             * @param ui UI
             */
            void SetUI(UIBase* ui)
            {
                m_ui = ui;
            }


        protected:
            /** アニメーションを動かすUI */
            UIBase* m_ui;
        };




        /**
         * @brief 値(float Vector2 Vector3 Vector4)をイージングで動かすアニメーション
         * @details 動かした値は、Apply()でUIに反映する。UIに反映するクラスは、Apply()を書き換える。
         *          SetFunc()で関数を設定すると、書き換えなくてもApply()から呼ばれる。
         *          jsonの定義から作ったアニメーションは、PlayAnimation()の時に、jsonが更新されていれば定義を取り直す。
         *          SetParameter()で値を設定したアニメーションは、定義に従わなくなる。
         * @tparam T float Vector2 Vector3 Vector4
         */
        template <typename T>
        class UIValueAnimation : public UIAnimationBase
        {
        public:
            UIValueAnimation();
            ~UIValueAnimation() override;


        public:
            /** 更新。再生中なら時間を進めて、値をApply()でUIに反映する。終わった時の値も反映する。 */
            void Update() override;

            /** 再生。終わっていれば頭から再生し直す。 */
            void PlayAnimation() override;

            /** 停止 */
            void StopAnimation() override;

            /** 再生中かどうか */
            bool IsPlayAnimation() const override;


        public:
            /**
             * @brief アニメーションの値を設定
             * @details 頭に戻る。再生状態は変えない。jsonの定義には従わなくなる。
             * @param start 始める値
             * @param end 終わる値
             * @param timeSec 時間(秒)
             * @param type イージングの種類
             * @param loopMode ループの種類
             * @param repeatCount 繰り返す回数(Loop PingPong用)。util::InfiniteRepeatで無限
             */
            void SetParameter(
                const T& start,
                const T& end,
                const float timeSec,
                const util::EasingType type,
                const util::LoopMode loopMode,
                const uint16_t repeatCount = util::InfiniteRepeat
            );

            /**
             * @brief 終わった時の動作を設定
             * @details Hold: 終わった値で止まる。Reset: 始点に戻って止まる。既定はHold。
             * @param endBehavior 終わった時の動作
             */
            void SetEndBehavior(const util::EndBehavior endBehavior);

            /**
             * @brief jsonの定義に従わせる
             * @details UIAnimationFactoryが呼ぶ。以降、jsonが更新されたら、PlayAnimation()の時に定義を取り直す。
             * @param def 定義
             */
            void SetDefinition(const UIAnimationDef& def);

            /**
             * @brief 現在の値を取得
             * @return 現在の値
             */
            T GetCurrentValue() const;

            /**
             * @brief 値をUIに反映する関数を設定
             * @param func 関数
             */
            void SetFunc(const UIAnimationApplyFunc<T>& func);


        protected:
            /**
             * @brief 値をUIに反映する
             * @details 既定では、SetFunc()で設定した関数を呼ぶ。
             * @param value 現在の値
             */
            virtual void Apply(const T& value);


        private:
            /** jsonの定義を、このアニメーションに設定する */
            void ApplyDefinition(const UIAnimationDef& def);

            /** jsonが更新されていれば、定義を取り直す */
            void RefreshDefinition();


        private:
            /** カーブ */
            util::Curve<T> m_curve;
            /** 値をUIに反映する関数 */
            UIAnimationApplyFunc<T> m_applyFunc;
            /** 従っている定義のキー。従っていない場合は0 */
            uint32_t m_definitionKey;
            /** 定義を取った時のリビジョン */
            uint32_t m_definitionRevision;
        };


        // テンプレートの定義はUIAnimation.cppにある。
        extern template class UIValueAnimation<float>;
        extern template class UIValueAnimation<Vector2>;
        extern template class UIValueAnimation<Vector3>;
        extern template class UIValueAnimation<Vector4>;

        /** floatのアニメーション */
        using UIFloatAnimation = UIValueAnimation<float>;
        /** Vector2のアニメーション */
        using UIVector2Animation = UIValueAnimation<Vector2>;
        /** Vector3のアニメーション */
        using UIVector3Animation = UIValueAnimation<Vector3>;
        /** Vector4のアニメーション */
        using UIVector4Animation = UIValueAnimation<Vector4>;




        /**
         * @brief 色のアニメーション。UIBase::m_colorを動かす。(jsonの色は0〜255)
         */
        class UIColorAnimation : public UIVector4Animation
        {
        public:
            UIColorAnimation();
            ~UIColorAnimation() override;


        protected:
            void Apply(const Vector4& value) override;
        };




        /**
         * @brief 拡大のアニメーション。UIBase::m_transformのm_localTransformの拡大を動かす。
         */
        class UIScaleAnimation : public UIVector3Animation
        {
        public:
            UIScaleAnimation();
            ~UIScaleAnimation() override;


        protected:
            void Apply(const Vector3& value) override;
        };




        /**
         * @brief 座標のアニメーション。UIBase::m_transformのm_localTransformの座標を動かす。
         */
        class UITranslateAnimation : public UIVector3Animation
        {
        public:
            UITranslateAnimation();
            ~UITranslateAnimation() override;


        protected:
            void Apply(const Vector3& value) override;
        };




        /**
         * @brief 回転のアニメーション。UIBase::m_transformのm_localTransformの回転(Z軸回りの角度。度)を動かす。
         */
        class UIRotationAnimation : public UIFloatAnimation
        {
        public:
            UIRotationAnimation();
            ~UIRotationAnimation() override;


        protected:
            void Apply(const float& value) override;
        };




        /**
         * @brief シーケンス内の1ステップ
         */
        struct UIAnimationStep
        {
            /**
             * @brief コンストラクタ
             * @param animationKey 再生するアニメーションのキー
             * @param delayBefore このステップを始める前の待ち時間(秒)
             * @param onStart 始める時に呼ぶ関数
             * @param onComplete 終わった時に呼ぶ関数
             */
            UIAnimationStep(
                const uint32_t animationKey,
                const float delayBefore,
                std::function<void()> onStart,
                std::function<void()> onComplete
            );

            /** 再生するアニメーションのキー */
            uint32_t animationKey;
            /** このステップを始める前の待ち時間(秒) */
            float delayBefore;
            /** 始める時に呼ぶ関数 */
            std::function<void()> onStart;
            /** 終わった時に呼ぶ関数 */
            std::function<void()> onComplete;
        };




        /**
         * @brief UIのアニメーションを、順番に再生するクラス
         * @details Add()で並べた順に、UIに登録されたアニメーションを1つずつ再生する。前のアニメーションが終わったら、次を始める。
         *          Update()は呼び出し側が毎フレーム呼ぶこと。
         *          例: sequence.Add(Hash32("moveUp")).Add(Hash32("bounce"), 0.1f).OnComplete([]() { ... });
         *              sequence.Play(ui);
         *          NOTE: 終わらない(Loopで無限に繰り返す)アニメーションのステップで、シーケンスは止まったままになる。
         *          NOTE: 再生中のUIが破棄されると、アクセスできなくなる。Layoutの作り直しでUIが破棄されるので、
         *                MenuBase::InitializeLogic()で、Stop()すること。
         *          NOTE: 登録されていないアニメーションのステップは、飛ばす。再生中にアニメーションが取り除かれた時は、終わったものとして扱う。
         */
        class UIAnimationSequence
        {
        public:
            UIAnimationSequence();
            ~UIAnimationSequence();


        public:
            /**
             * @brief ステップを追加
             * @param animKey 再生するアニメーションのキー
             * @param delayBefore このステップを始める前の待ち時間(秒)
             * @return 自身
             */
            UIAnimationSequence& Add(const uint32_t animKey, const float delayBefore = 0.0f);

            /**
             * @brief 関数付きのステップを追加
             * @param animKey 再生するアニメーションのキー
             * @param delayBefore このステップを始める前の待ち時間(秒)
             * @param onStart ステップを始める時に呼ぶ関数
             * @param onComplete ステップが終わった時に呼ぶ関数
             * @return 自身
             */
            UIAnimationSequence& Add(
                const uint32_t animKey,
                const float delayBefore,
                std::function<void()> onStart,
                std::function<void()> onComplete = nullptr
            );

            /**
             * @brief 全てのステップが終わった時に呼ぶ関数を設定
             * @param callback 関数
             * @return 自身
             */
            UIAnimationSequence& OnComplete(std::function<void()> callback);

            /**
             * @brief 再生
             * @details 再生中の場合は、最初からやり直す。ステップが無い場合は、何もしない。
             * @param target アニメーションを再生するUI
             */
            void Play(UIBase* target);

            /** 停止。次のPlay()で、最初から再生する。 */
            void Stop();

            /**
             * @brief 更新
             * @param deltaTime 前のフレームからの経過時間(秒)
             */
            void Update(const float deltaTime);

            /** ステップを全て取り除いて、停止する */
            void Clear();

            /**
             * @brief 再生中かどうか
             * @return 再生中ならtrue
             */
            bool IsPlaying() const
            {
                return m_isPlaying;
            }


        private:
            /** 次のステップに進む。全て終わっていたら、シーケンスを終える。 */
            void AdvanceToNext();

            /** 現在のステップのアニメーションを始める。 */
            void StartCurrentStep();


        private:
            /** ステップ */
            std::vector<UIAnimationStep> m_steps;
            /** 再生しているステップの番号。再生していない時は-1 */
            int m_currentIndex;
            /** 再生中かどうか */
            bool m_isPlaying;
            /** ステップを始める前の、残りの待ち時間(秒) */
            float m_delayTimer;
            /** ステップを始める前に待っているかどうか */
            bool m_waitingDelay;
            /** 再生した回数。関数の中でPlay() Stop() Clear()が呼ばれたかを調べるのに使う。 */
            uint32_t m_playId;
            /** アニメーションを再生するUI */
            UIBase* m_target;
            /** 全てのステップが終わった時に呼ぶ関数 */
            std::function<void()> m_onSequenceComplete;
        };
    } // namespace ui
} // namespace app
