/**
 * @file PlayerController.cpp
 * @brief パッド(キーボード)の入力から、キャラクターへの入力を作るController
 */
#include "stdafx.h"
#include "PlayerController.h"


namespace app
{
    namespace
    {
        /** XInputのスティックの最大値(-32768～32767を-1.0f～1.0fにするための除数) */
        constexpr float RAW_STICK_MAX = 32767.0f;
        /** 生のスティックの遊び(これ以下は、エンジンのスティックの値を使う) */
        constexpr float RAW_DEAD_ZONE = 0.1f;
        /** 移動入力として扱うスティックの倒し具合 */
        constexpr float MOVE_THRESHOLD = 0.1f;
        /** この倒し具合以下なら忍び足、それより倒すと走り */
        constexpr float SNEAK_THRESHOLD = 0.9f;
    }


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
        // 左スティック
        // コントローラーの倒し具合で挙動を変えるため、正規化前の生のスティックの入力を取る。
        // 倒されていなければ、エンジンの入力(キーボードの入力)を使う。
        // -------------------------------------------------------------
        const XINPUT_STATE& state = pad.GetXInputState();
        const float rawX = static_cast<float>(state.Gamepad.sThumbLX) / RAW_STICK_MAX;
        const float rawY = static_cast<float>(state.Gamepad.sThumbLY) / RAW_STICK_MAX;
        const float rawLength = sqrtf(rawX * rawX + rawY * rawY);

        float stickX = 0.0f;
        float stickY = 0.0f;
        float stickLength = 0.0f;
        if (rawLength > RAW_DEAD_ZONE)
        {
            stickX = rawX;
            stickY = rawY;
            stickLength = (std::min)(rawLength, 1.0f);
        }
        else
        {
            stickX = pad.GetLStickXF();
            stickY = pad.GetLStickYF();
            stickLength = (std::min)(sqrtf(stickX * stickX + stickY * stickY), 1.0f);

            // エンジンのキーボード入力は斜めが 0.5 + 0.5 に正規化され、長さが 0.707 になってしまう。
            // そのままだと斜めだけ忍び足になるので、キーボードの入力は常に全力で倒したことにする。
            if (stickLength > MOVE_THRESHOLD)
            {
                stickLength = 1.0f;
            }
        }


        // -------------------------------------------------------------
        // 移動(カメラの向き基準)
        // -------------------------------------------------------------
        if (stickLength > MOVE_THRESHOLD)
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

            // スティックの方向をカメラの向きに合わせて、倒し具合を長さにする
            Vector3 moveDirection = cameraRight * stickX + cameraForward * stickY;
            if (moveDirection.LengthSq() > FLT_EPSILON)
            {
                moveDirection.Normalize();
            }
            input.SetMove(moveDirection * stickLength);

            // 倒し具合かBボタンで、忍び足と走りを決める
            const bool isSneak = stickLength <= SNEAK_THRESHOLD || pad.IsPress(enButtonB);
            input.SetPress(EnInputButton::Sneak, isSneak);
            input.SetPress(EnInputButton::Dash, !isSneak);
        }


        // -------------------------------------------------------------
        // ボタン(押している状態だけ設定する。押した瞬間は ControllerSlot が計算する)
        // -------------------------------------------------------------
        input.SetPress(EnInputButton::Jump, pad.IsPress(enButtonA));
        input.SetPress(EnInputButton::Action1, pad.IsPress(enButtonX));
        input.SetPress(EnInputButton::Action2, pad.IsPress(enButtonY));

        return input;
    }
}
