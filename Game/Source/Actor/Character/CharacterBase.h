/**
 * @file CharacterBase.h
 * @brief キャラクターの基底クラス
 */
#pragma once
#include <cstdint>
#include <memory>

#include "Source/Actor/Actor.h"


namespace app
{
    namespace actor
    {
        /** アニメーションデータ */
        struct AnimationData
        {
            /** ファイルネーム */
            const char* fileName;
            /** ループフラグ */
            const bool isLoop;
        };




        /************************************/


        /** モデルデータ */
        struct ModelData
        {
            /** ファイルネーム */
            const char* fileName;
            /** アニメーションデータ */
            const AnimationData* animationData;
            /** モデルの上方向 */
            const EnModelUpAxis upAxis;
            /** アニメーションクリップの数 */
            const uint8_t clipNum;
        };




        /************************************/


        /**
         * @brief キャラクターの基底クラス
         * @details モデルとアニメーションクリップはInit()で同期ロードする。
         *          Start()は実装していないので、派生クラスのStart()でInit()を呼ぶこと。
         */
        class CharacterBase : public Actor
        {
        public:
            /**
             * @brief キャラクターコントローラーを取得
             * @note 素のk2EngineLowのCharacterControllerには重力の設定がない。
             *       Execute()に渡す移動速度のyに、呼び出し側で重力を加えること。
             * @return キャラクターコントローラーのポインタ
             */
            inline CharacterController* GetCharacterController()
            {
                return &m_characterController;
            }
            /**
             * @brief 水平方向(XZ)の現在の移動速度を取得
             * @note 加速・減速(慣性)の途中の値。垂直方向はGetVerticalVelocity()で取る。
             *       ステートマシンではなくキャラクターが持つので、ステートマシンを付け替えても引き継がれる。
             * @return 現在の移動速度
             */
            inline const Vector3& GetCurrentVelocity() const
            {
                return m_currentVelocity;
            }
            /**
             * @brief 水平方向(XZ)の現在の移動速度を設定
             * @param velocity 現在の移動速度
             */
            inline void SetCurrentVelocity(const Vector3& velocity)
            {
                m_currentVelocity = velocity;
            }
            /**
             * @brief 垂直方向(Y)の現在の速度を取得
             * @return 垂直方向の速度。上向きが正
             */
            inline float GetVerticalVelocity() const
            {
                return m_verticalVelocity;
            }
            /**
             * @brief 垂直方向(Y)の現在の速度を設定
             * @note ジャンプは、上向きの速度を設定して行う。
             * @param verticalVelocity 垂直方向の速度。上向きが正
             */
            inline void SetVerticalVelocity(const float verticalVelocity)
            {
                m_verticalVelocity = verticalVelocity;
            }


        public:
            CharacterBase();
            virtual ~CharacterBase() override;


        protected:
            /**
             * @brief 初期化処理
             * @details モデル・アニメーションクリップのロードと、キャラクターコントローラーの初期化を行う。
             * @note キャラクターコントローラーは現在のトランスフォームの座標に作られる。
             *       呼ぶ前にSetPosition()で初期座標を設定しておくこと。
             * @param data モデルデータ
             * @param radius キャラクターコントローラーの半径
             * @param height キャラクターコントローラーの高さ
             */
            void Init(const ModelData& data, const float radius, const float height);


        protected:
            /** 更新処理 */
            virtual void Update() override;
            /** 描画処理 */
            virtual void Render(RenderContext& rc) override;


        protected:
            /** アニメーションクリップ */
            std::unique_ptr<AnimationClip[]> m_animationClips;
            /** キャラクターコントローラー */
            CharacterController m_characterController;
            /** 水平方向(XZ)の現在の移動速度 */
            Vector3 m_currentVelocity;
            /** 垂直方向(Y)の現在の速度 */
            float m_verticalVelocity;
        };
    } // namespace actor
} // namespace app
