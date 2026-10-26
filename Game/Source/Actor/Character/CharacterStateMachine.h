/**
 * @file CharacterStateMachine.h
 * @brief キャラクターのステートマシン
 */
#pragma once
#include "CharacterBase.h"
#include "Source/Actor/ActorStateMachine.h"
#include "Source/MoveBehavior/CharacterInput.h"
#include "Source/MoveBehavior/ControllerSlot.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief キャラクターのステートマシン
         * @details キャラクター共通の処理(入力の作成、移動、遷移判定の部品)をまとめる。
         *          入力を作る ControllerSlot を持つ。Update() で、持ち主の座標と向きから今フレームの入力を作ってから、ステートを更新する。
         *          現在の速度は持ち主の CharacterBase が持つので、ステートマシンを付け替えても引き継がれる。
         *          持ち主に求めるのは CharacterBase の公開機能だけなので、CharacterBase の派生ならどれにでも付けられる。
         *          使い方: GetControllerSlot() でControllerを付け(プレイヤーならパッド、敵ならAI)、持ち主が毎フレーム Update() を呼ぶ。
         *          移動するステートは、自分の Update() で SetMoveSpeed() と Move() を呼ぶ。
         *          歩き・走りの速度はキャラクターごとに違うので、派生クラスが GetWalkSpeed() / GetRunSpeed() で返す。
         */
        class CharacterStateMachine : public ActorStateMachine
        {
        public:
            /**
             * @brief 今フレームの入力を取得する
             * @return Update() で作った入力。Controllerが付いていなければ無入力
             */
            inline const CharacterInput& GetInput() const
            {
                return m_controllers.GetInput();
            }
            /**
             * @brief コントローラーの入れ物を取得する
             * @details Attach() / Detach() でControllerを付け替えられる。
             * @return コントローラーの入れ物
             */
            inline ControllerSlot& GetControllerSlot()
            {
                return m_controllers;
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

            /**
             * @brief 歩きの移動速度を取得する
             * @details キャラクターごとの値なので、派生クラスが返す。歩きステートが Update() で SetMoveSpeed() に渡す。
             * @return 歩きの移動速度(入力の倒し具合が最大のときの速さ)
             */
            virtual float GetWalkSpeed() const = 0;
            /**
             * @brief 走りの移動速度を取得する
             * @details キャラクターごとの値なので、派生クラスが返す。走りステートが Update() で SetMoveSpeed() に渡す。
             * @return 走りの移動速度(入力の倒し具合が最大のときの速さ)
             */
            virtual float GetRunSpeed() const = 0;


        public:
            /**
             * @brief ステートマシンを更新する
             * @details 持ち主の座標と向きから今フレームの入力を作り、そのあとでステートを更新する。
             * @note 持ち主が毎フレーム呼び出すこと
             */
            virtual void Update() override;

            /**
             * @brief 移動処理
             * @details 入力と移動速度から、慣性(加速・減速)をつけて速度を決め、重力を足して、
             *          キャラクターコントローラーで壁・地面との衝突を解決する。最後に UpdateRotation() で向きを決める。
             *          結果は持ち主の座標と向きに反映される。
             * @note 持ち主の座標が正なので、外から SetPosition() されていても、その座標から動く。
             *       移動するステートが毎フレーム呼ぶこと。
             * @note 床がまだないので、高さが0以下のときは地面にいるとみなして、重力を受けない。
             */
            void Move();


        protected:
            /**
             * @brief 移動入力があるかどうか(歩き・走りなどへの遷移判定に使う)
             * @return 移動入力があれば true
             */
            inline bool CanChangeMoveState() const
            {
                return GetInput().IsMoving();
            }
            /**
             * @brief 走りに切り替えられるかどうか
             * @return 移動入力があり、ダッシュを押していれば true
             */
            inline bool CanChangeRunState() const
            {
                return CanChangeMoveState() && GetInput().IsPress(EnInputButton::Dash);
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
             * @brief 向きを決める
             * @details Move() が最後に呼ぶ。既定は、移動入力がある間だけ、入力の方向へ向ける。(入力がない間は、向きを変えない)
             *          向きの決め方が違うキャラクターは、オーバーライドする。(例: Miroは、移動方向と関係なくカメラの方向を向く)
             */
            virtual void UpdateRotation();
            /**
             * @brief 持ち主を、水平方向(XZ)の指定の方向へ向ける
             * @details UpdateRotation() のオーバーライドから使う。
             * @param direction 向ける方向。yは無視される。水平方向の長さが0なら、向きを変えない
             */
            void FaceDirection(const Vector3& direction);

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
            /** コントローラーの入れ物(今フレームの入力もここが持つ) */
            ControllerSlot m_controllers;
            /** 移動速度(入力の倒し具合が最大のときの速さ) */
            float m_moveSpeed;
        };
    } // namespace actor
} // namespace app
