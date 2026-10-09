/**
 * @file CharacterStateMachine.h
 * @brief キャラクターのステートマシン
 */
#pragma once
#include "CharacterBase.h"
#include "Source/Actor/ActorStateMachine.h"
#include "Source/MoveBehavior/CharacterInput.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief キャラクターのステートマシン
         * @details キャラクター共通の処理(入力の受け取り、移動、遷移判定の部品)をまとめる。
         *          現在の速度は持ち主の CharacterBase が持つので、ステートマシンを付け替えても引き継がれる。
         *          持ち主に求めるのは CharacterBase の公開機能だけなので、CharacterBase の派生ならどれにでも付けられる。
         *          使い方: 持ち主が毎フレーム、SetInput() で入力を渡してから Update() を呼ぶ。
         *          移動するステートは、自分の Update() で SetMoveSpeed() と Move() を呼ぶ。
         */
        class CharacterStateMachine : public ActorStateMachine
        {
        public:
            /**
             * @brief 今フレームの入力を設定する
             * @note 持ち主が、Update() の前に毎フレーム呼ぶこと。
             *       ControllerSlot::Update() が返した入力をそのまま渡せる。
             * @param input 今フレームの入力
             */
            inline void SetInput(const CharacterInput& input)
            {
                m_input = input;
            }
            /**
             * @brief 今フレームの入力を取得する
             * @return 入力
             */
            inline const CharacterInput& GetInput() const
            {
                return m_input;
            }
            /**
             * @brief 移動速度を設定する
             * @details 入力の倒し具合が最大のときの速さ。倒し具合が小さければ、その割合だけ遅くなる。
             *          ステートごとに決める値(歩き・走りなど)なので、移動するステートが Enter() や Update() で設定する。
             * @param moveSpeed 移動速度
             */
            inline void SetMoveSpeed(const float moveSpeed)
            {
                m_moveSpeed = moveSpeed;
            }
            /**
             * @brief 移動速度を取得する
             * @return 移動速度
             */
            inline float GetMoveSpeed() const
            {
                return m_moveSpeed;
            }
            /**
             * @brief 水平方向(XZ)の現在の移動速度を取得する
             * @details 慣性で加速・減速している途中の値。
             * @return 現在の移動速度
             */
            inline const Vector3& GetCurrentVelocity() const
            {
                return m_ownerCharacter->GetCurrentVelocity();
            }
            /**
             * @brief 垂直方向(Y)の速度を設定する
             * @note ジャンプは、上向きの速度を設定して行う。重力は Move() が足す。
             * @param verticalVelocity 垂直方向の速度。上向きが正
             */
            inline void SetVerticalVelocity(const float verticalVelocity)
            {
                m_ownerCharacter->SetVerticalVelocity(verticalVelocity);
            }


        public:
            /**
             * @brief 移動処理
             * @details 入力と移動速度から、慣性(加速・減速)をつけて速度を決め、重力を足して、
             *          キャラクターコントローラーで壁・地面との衝突を解決する。入力がある間は、入力の方向を向く。
             *          結果は持ち主の座標と向きに反映される。
             * @note 持ち主の座標が正なので、外から SetPosition() されていても、その座標から動く。
             *       移動するステートが毎フレーム呼ぶこと。
             */
            void Move();


        protected:
            /**
             * @brief 移動入力があるかどうか(歩き・走りなどへの遷移判定に使う)
             * @return 移動入力があれば true
             */
            inline bool CanChangeMoveState() const
            {
                return m_input.IsMoving();
            }
            /**
             * @brief 走りに切り替えられるかどうか
             * @return 移動入力があり、ダッシュを押していれば true
             */
            inline bool CanChangeRunState() const
            {
                return CanChangeMoveState() && m_input.IsPress(EnInputButton::Dash);
            }
            /**
             * @brief 地面についているかどうか
             * @return 地面についていれば true
             */
            inline bool IsOnGround() const
            {
                return m_ownerCharacter->GetCharacterController()->IsOnGround();
            }

            /**
             * @brief ステートの変更先を取得する
             * @details 何もしない(遷移しない)。派生クラスでオーバーライドして遷移を決める。
             * @return 変更先のステートポインタ
             */
            virtual IState* GetChangeState() override;


        public:
            /**
             * @brief コンストラクタ
             * @param ownerCharacter 持ち主のキャラクター
             */
            explicit CharacterStateMachine(CharacterBase* ownerCharacter);
            virtual ~CharacterStateMachine() override = default;


        protected:
            /** 持ち主のキャラクター */
            CharacterBase* m_ownerCharacter;
            /** 今フレームの入力 */
            CharacterInput m_input;
            /** 移動速度(入力の倒し具合が最大のときの速さ) */
            float m_moveSpeed;
        };
    } // namespace actor
} // namespace app
