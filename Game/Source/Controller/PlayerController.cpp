/**
 * @file PlayerController.cpp
 * @brief パッド(キーボード)の入力から、キャラクターへの入力を作るController
 */
#include "stdafx.h"

#include "PlayerController.h"


namespace app
{
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


        // -------------------------------------------------------------
        // 移動(左スティック。カメラの向き基準)
        // エンジンのGamePadは、遊び(デッドゾーン)を処理して、スティックの値を長さ1に正規化する。
        // スティックの倒し具合は取れないので、入力があれば常に全力で動く。(キーボードの斜めも同じ)
        // -------------------------------------------------------------
        const float stickX = pad.GetLStickXF();
        const float stickY = pad.GetLStickYF();
        if (stickX * stickX + stickY * stickY > FLT_EPSILON)
        {
            // カメラの水平方向の前と右を求める
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
            const Vector3 cameraRight(cameraForward.z, 0.0f, -cameraForward.x);

            // スティックの方向をカメラの向きに合わせる
            Vector3 moveDirection = cameraRight * stickX + cameraForward * stickY;
            if (moveDirection.LengthSq() > FLT_EPSILON)
            {
                moveDirection.Normalize();
            }
            input.SetMove(moveDirection);

            // Bボタンを押している間は走り、押していなければ歩き
            input.SetPress(EnInputButton::Dash, pad.IsPress(enButtonB));
        }


        // -------------------------------------------------------------
        // ボタン(押している状態だけ設定する。押した瞬間は ControllerSlot が計算する)
        // -------------------------------------------------------------
        input.SetPress(EnInputButton::Jump, pad.IsPress(enButtonA));
        input.SetPress(EnInputButton::Action1, pad.IsPress(enButtonX));
        input.SetPress(EnInputButton::Action2, pad.IsPress(enButtonY));

        return input;
    }
} // namespace app
