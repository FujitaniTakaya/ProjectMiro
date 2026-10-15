/**
 * @file CharacterStateMachine.cpp
 * @brief キャラクターのステートマシン
 */
#include "stdafx.h"

#include "CharacterStateMachine.h"

#include <algorithm>


namespace app
{
    namespace actor
    {
        namespace
        {
            /** 重力の値 */
            constexpr float GRAVITY = -9.8f * 150.0f;
            /** 地面の高さ(床を作るまでの仮の値。この高さ以下では重力を受けない) */
            constexpr float GROUND_HEIGHT = 0.0f;
            /** 加速度(目標速度への到達の早さ) */
            constexpr float ACCELERATION = 10.0f;
            /** 摩擦係数(入力がないときの減速の早さ) */
            constexpr float FRICTION = 5.0f;
            /** 補間の割合の最大値 */
            constexpr float MAX_LERP_FACTOR = 1.0f;
        } // namespace


        void CharacterStateMachine::Update()
        {
            // 持ち主の座標と向きから、今フレームの入力を作る
            const Transform& transform = m_ownerActor->GetTransform();
            ControllerContext context;
            context.position = transform.m_position;
            Vector3 forward = Vector3::Front;
            transform.m_rotation.Apply(forward);
            context.forward = forward;
            m_controllers.Update(context);

            // 入力をもとに、ステートを更新する
            StateMachineBase::Update();
        }


        void CharacterStateMachine::Move()
        {
            const float deltaTime = g_gameTime->GetFrameDeltaTime();
            const CharacterInput& input = GetInput();
            const Vector3& inputMove = input.GetMove();

            // --- 慣性と摩擦の計算 ---
            // 入力の長さは倒し具合(0.0f〜1.0f)なので、そのまま速度の割合になる
            Vector3 velocity = m_ownerCharacter->GetCurrentVelocity();
            if (input.IsMoving())
            {
                const Vector3 targetVelocity = inputMove * m_moveSpeed;
                const float lerpFactor = std::min<float>(MAX_LERP_FACTOR, ACCELERATION * deltaTime);
                velocity.Lerp(lerpFactor, velocity, targetVelocity);
            }
            else
            {
                const float lerpFactor = std::min<float>(MAX_LERP_FACTOR, FRICTION * deltaTime);
                velocity.Lerp(lerpFactor, velocity, Vector3::Zero);
            }
            m_ownerCharacter->SetCurrentVelocity(velocity);

            // --- キャラクターコントローラーによるコリジョン解決 ---
            // 持ち主の座標が正なので、外から SetPosition() されていてもその座標から動くよう、毎回渡す
            CharacterController* characterController = m_ownerCharacter->GetCharacterController();
            characterController->SetPosition(m_ownerActor->GetTransform().m_position);

            // 素のCharacterControllerには重力がないので、垂直方向の速度に重力を足して渡す。
            // 床がまだないので、高さがGROUND_HEIGHT以下で落下中(または静止中)なら、地面にいるとみなして重力を受けない。
            // (上向きの速度があるとき、つまりジャンプ中は、重力を受ける)
            float verticalVelocity = m_ownerCharacter->GetVerticalVelocity();
            if (m_ownerActor->GetTransform().m_position.y <= GROUND_HEIGHT && verticalVelocity <= 0.0f)
            {
                verticalVelocity = 0.0f;
            }
            else
            {
                verticalVelocity += GRAVITY * deltaTime;
            }
            Vector3 moveSpeed(velocity.x, verticalVelocity, velocity.z);
            const Vector3 nextPosition = characterController->Execute(moveSpeed, deltaTime);
            // 着地すると、Execute() が垂直方向の速度を0に戻す
            m_ownerCharacter->SetVerticalVelocity(moveSpeed.y);
            m_ownerActor->SetPosition(nextPosition);

            // --- 回転 ---
            // 入力がある間だけ、入力の方向へ向ける。(入力がない間は、向きを変えない)
            if (input.IsMoving())
            {
                Vector3 inputDirection = inputMove;
                inputDirection.y = 0.0f;
                inputDirection.Normalize();

                Quaternion rotation = m_ownerActor->GetTransform().m_rotation;
                rotation.SetRotationYFromDirectionXZ(inputDirection);
                m_ownerActor->SetRotation(rotation);
            }
        }


        IState* CharacterStateMachine::GetChangeState()
        {
            // 何もしない
            return nullptr;
        }


        CharacterStateMachine::CharacterStateMachine(CharacterBase* ownerCharacter)
            : ActorStateMachine(ownerCharacter)
            , m_ownerCharacter(ownerCharacter)
            , m_controllers()
            , m_moveSpeed(0.0f)
        {
        }
    } // namespace actor
} // namespace app
