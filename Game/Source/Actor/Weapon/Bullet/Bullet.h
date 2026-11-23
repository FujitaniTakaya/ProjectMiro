/**
 * @file Bullet.h
 * @brief 弾
 */
#pragma once
#include "BulletHit.h"
#include "Source/Actor/Actor.h"


namespace app
{
    namespace actor
    {
        struct BulletParameter;


        /**
         * @brief 弾
         * @details 発射された向きへ、一定の速さでまっすぐ進む。発射してから BulletParameter::lifeTime 秒たつと、IsExpired() が true になる。
         *          毎フレーム、前の位置から今の位置まで、半径 BulletParameter::hitRadius の球を動かして当たりを調べる。(SweepSphere)
         *          何かに当たったら、当たった所で止まって HasHit() が true になる。当たった相手に何をするかは、BulletManager の処理が行う。
         *          使い方は、Init() → StartWrapper() → 毎フレーム UpdateWrapper()・RenderWrapper()。BulletManager が行うので、外から直接作らない。
         * @note 重力は無い。
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
             * @brief 当たりを調べる時に無視する物理オブジェクトを設定する
             * @details 発射した人の体を渡すと、自分に当たらなくなる。StartWrapper() の前でも後でもよい。
             * @param ignore 無視する物理オブジェクト。無視しないなら nullptr
             */
            inline void SetIgnoreObject(const btCollisionObject* ignore)
            {
                m_ignore = ignore;
            }

            /**
             * @brief 寿命が尽きたか
             * @return 発射してからの経過時間が lifeTime 以上ならtrue
             */
            bool IsExpired() const;

            /**
             * @brief 何かに当たったか
             * @return 当たって止まったら true
             */
            inline bool HasHit() const
            {
                return m_isHit;
            }

            /**
             * @brief 当たった情報を取得
             * @note HasHit() が true の時だけ意味がある。
             * @return 当たった情報
             */
            inline const BulletHit& GetHit() const
            {
                return m_hit;
            }

            /**
             * @brief 役目を終えたか
             * @return 何かに当たったか、寿命が尽きたら true。BulletManager が、破棄するかの判定に使う
             */
            inline bool IsFinished() const
            {
                return m_isHit || IsExpired();
            }


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
            /** 当たりを調べる時に無視する物理オブジェクト(発射した人の体)。無ければ nullptr */
            const btCollisionObject* m_ignore;
            /** 何かに当たって止まったか */
            bool m_isHit;
            /** 当たった情報(m_isHit が true の時だけ意味がある) */
            BulletHit m_hit;
        };
    } // namespace actor
} // namespace app
