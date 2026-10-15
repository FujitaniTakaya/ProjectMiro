/**
 * @file Actor.h
 * @brief 見た目が存在するオブジェクトの基底クラス
 */
#pragma once
#include "IObject.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief 見た目が存在するオブジェクトの基底クラス
         * @note ステータスは持たない。必要になったら派生クラス側、または後からここに追加する。
         */
        class Actor : public IObject
        {
        public:
            /**
             * @brief トランスフォームを取得
             * @return トランスフォームの参照
             */
            inline const Transform& GetTransform() const
            {
                return m_transform;
            }
            /**
             * @brief モデルレンダーを取得
             * @return モデルレンダーの参照
             */
            inline ModelRender& GetModelRender()
            {
                return m_modelRender;
            }
            /**
             * @brief 座標を設定
             * @param position 座標
             */
            void SetPosition(const Vector3& position)
            {
                m_transform.m_position = position;
            }
            /**
             * @brief 回転を設定
             * @param rotation 回転
             */
            void SetRotation(const Quaternion& rotation)
            {
                m_transform.m_rotation = rotation;
            }
            /**
             * @brief 拡大率を設定
             * @param scale 拡大率
             */
            void SetScale(const Vector3& scale)
            {
                m_transform.m_scale = scale;
            }
            /**
             * @brief アニメーション再生速度の設定
             * @param speed 再生速度(1.0f=通常速度)
             */
            void SetAnimationSpeed(const float speed)
            {
                m_modelRender.SetAnimationSpeed(speed);
            }


        public:
            Actor() = default;
            virtual ~Actor() = default;


        protected:
            /** 初期化処理 */
            virtual void Start() override = 0;
            /** 更新処理 */
            virtual void Update() override = 0;
            /** 描画処理 */
            virtual void Render(RenderContext& rc) override = 0;


        protected:
            /** トランスフォーム */
            Transform m_transform;
            /** モデルレンダー */
            ModelRender m_modelRender;
        };
    } // namespace actor
} // namespace app
