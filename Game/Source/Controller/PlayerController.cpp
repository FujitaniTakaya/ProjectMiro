/**
 * @file PlayerController.cpp
 * @brief パッド(キーボード)の入力から、キャラクターへの入力を作るController
 */
#include "stdafx.h"

#include "PlayerController.h"

#include <cstddef>
#include <cstdint>
#include <iterator>


namespace app
{
    namespace
    {
        /** プレイヤーの操作。役割で名前を付ける。ボタンとの対応は ACTION_BINDINGS で決める */
        enum class EnPlayerAction : uint8_t
        {
            /** ジャンプ */
            Jump,
            /** 走り。押している間は歩きではなく走る */
            Dash,
            /** リロード */
            Reload,
            /** 構える。押している間だけ立つ */
            Aim,
            /** 撃つ。押した瞬間(IsTrigger)で1発撃つ */
            Fire,

            Max
        };

        /** 操作に割り当てるボタン */
        struct ActionBinding
        {
            /** 読み取るパッドのボタン */
            EnButton padButton;
            /** 押下状態を設定する、キャラクターへの入力のボタン */
            EnInputButton inputButton;
        };

        /**
         * 操作とボタンの対応
         * EnPlayerAction の順に並べること。
         * Yボタンは、割り当てる操作がまだないので載せていない。
         */
        constexpr ActionBinding ACTION_BINDINGS[] = {
            // Jump: Aボタン
            { enButtonA, EnInputButton::ActionA },
            // Dash: Bボタン(キーボードはK)
            { enButtonB, EnInputButton::ActionB },
            // Reload: Xボタン(キーボードはL)
            { enButtonX, EnInputButton::ActionX },
            // Aim: ZL。エンジンでは左トリガー(enButtonLB2。キーボードはN)
            { enButtonLB2, EnInputButton::ActionLB2 },
            // Fire: ZR。エンジンでは右トリガー(enButtonRB2。キーボードは8)
            { enButtonRB2, EnInputButton::ActionRB2 },
        };
        static_assert(std::size(ACTION_BINDINGS) == static_cast<size_t>(EnPlayerAction::Max), "操作とボタンの対応の数をEnPlayerActionと合わせること");
    } // namespace


    PlayerController::PlayerController(const int padIndex)
        : m_padIndex(padIndex)
    {
    }


    CharacterInput PlayerController::Update(const ControllerContext& /*context*/)
    {
        CharacterInput input;

        // パッド番号が範囲外、またはパッドがない場合は無入力
        if (m_padIndex < 0 || m_padIndex >= static_cast<int>(g_pad.size()) || g_pad[m_padIndex] == nullptr)
        {
            return input;
        }
        GamePad& pad = *g_pad[m_padIndex];


        // カメラの水平方向の前を求める
        Vector3 cameraForward = Vector3::Front;
        if (g_camera3D != nullptr)
        {
            Vector3 toTarget = g_camera3D->GetTarget() - g_camera3D->GetPosition();
            toTarget.y = 0.0f;
            if (toTarget.LengthSq() > FLT_EPSILON)
            {
                toTarget.Normalize();
                cameraForward = toTarget;
            }
        }

        // 向きたい方向(カメラの向き)。移動方向と別の向きを持つキャラクターだけが読む
        input.SetLook(cameraForward);


        // -------------------------------------------------------------
        // 移動(左スティック。カメラの向き基準)
        // エンジンのGamePadは、遊び(デッドゾーン)を処理して、スティックの値を長さ1に正規化する。
        // スティックの倒し具合は取れないので、入力があれば常に全力で動く。(キーボードの斜めも同じ)
        // -------------------------------------------------------------
        const float stickX = pad.GetLStickXF();
        const float stickY = pad.GetLStickYF();
        if (stickX * stickX + stickY * stickY > FLT_EPSILON)
        {
            // カメラの水平方向の右を求める
            const Vector3 cameraRight(cameraForward.z, 0.0f, -cameraForward.x);

            // スティックの方向をカメラの向きに合わせる
            Vector3 moveDirection = cameraRight * stickX + cameraForward * stickY;
            if (moveDirection.LengthSq() > FLT_EPSILON)
            {
                moveDirection.Normalize();
            }
            input.SetMove(moveDirection);
        }


        // -------------------------------------------------------------
        // ボタン(押している状態だけ設定する。押した瞬間は ControllerSlot が計算する)
        // 走り(Dash)は、スティックを倒していなくてもBボタンをそのまま写す。
        // 走りに切り替えるかどうかは、移動入力も見る読む側が決める。
        // -------------------------------------------------------------
        for (const ActionBinding& binding : ACTION_BINDINGS)
        {
            input.SetPress(binding.inputButton, pad.IsPress(binding.padButton));
        }

        return input;
    }
} // namespace app
