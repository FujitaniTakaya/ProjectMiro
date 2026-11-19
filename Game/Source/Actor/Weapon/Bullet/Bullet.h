/**
 * @file Bullet.h
 * @brief 弾
 */
#pragma once
#include "Source/Actor/Actor.h"


namespace app
{
    namespace actor
    {
        struct BulletParameter;


        /**
         * @brief 弾
         * @details 発射された向きへ、一定の速さでまっすぐ進む。発射してから BulletParameter::lifeTime 秒たつと、IsExpired() が true になる。
         *          使い方は、Init() → StartWrapper() → 毎フレーム UpdateWrapper()・RenderWrapper()。BulletManager が行うので、外から直接作らない。
         * @note 当たり判定はまだ無い。重力も無い。
         */
        class Bullet : public Actor
        {
        public:
            Bullet();
            virtual ~Bullet() = default;


            /**
             * @brief 発射の条件を設定する
             * @details StartWrapper() の前に呼ぶこと。
             * @param rotation 発射角度。この回転で Vector3::Front を回した向きに進む
             * @param position 発射位置
             * @param speed 発射スピード(cm/秒)
             */
            void Init(const Quaternion& rotation, const Vector3& position, const float speed);

            /**
             * @brief 寿命が尽きたか
             * @return 発射してからの経過時間が lifeTime 以上ならtrue
             */
            bool IsExpired() const;


        protected:
            void Start() override;
            void Update() override;
            void Render(RenderContext& rc) override;


        private:
            /**
             * @brief モデルに渡すトランスフォームを作る
             * @details 飛ぶ向き(m_transform の回転)に、modelRotation(度)の補正を掛けた回転にする。飛ぶ向き自体は変えない。
             * @return モデルに渡すトランスフォーム
             */
            Transform MakeRenderTransform() const;


        private:
            /** パラメーター(ParamHolderが持つもの。アプリが終わるまで使える)。毎フレーム読むので、ホットリロードがすぐ反映される */
            const BulletParameter& m_parameter;
            /** 発射スピード(cm/秒) */
            float m_speed;
            /** 発射してからの経過時間(秒) */
            float m_elapsedTime;
        };
    } // namespace actor
} // namespace app
