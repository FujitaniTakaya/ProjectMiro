/**
 * @file PlayerCameraParameter.h
 * @brief PlayerCameraのパラメーター
 * @details ParamHolderに EnParamID::PlayerCamera で登録する。jsonは Assets/parameter/camera/PlayerCamera.json。
 *          例: const PlayerCameraParameter& param = ParamHolder::Get().GetParameter<PlayerCameraParameter>(EnParamID::PlayerCamera);
 */
#pragma once
#include "Source/Core/Json/JsonView.h"


namespace app
{
    namespace camera
    {
        /**
         * @brief PlayerCameraのパラメーター
         * @details 注視点は、ターゲットの座標から「カメラの右方向 × targetOffsetRight」と「上方向 × targetOffsetUp」だけずらした位置。
         *          TPSの肩越しの視点にするため、右上にずらす。
         * @note toCameraPos は PlayerCamera::Start() でだけ使うので、ホットリロードしても次に起動するまで反映されない。
         *       他の値は、毎フレーム読み直すので、ホットリロードですぐ反映される。
         */
        struct PlayerCameraParameter
        {
            PlayerCameraParameter();


            // [PARAM_GEN begin members] 自動生成(tools/param_gen.py)。手で編集しない
            /** 注視点を、ターゲットの座標から上にずらす量 */
            float targetOffsetUp;
            /** 注視点を、ターゲットの座標からカメラの右方向にずらす量(水平方向。左右に回すと一緒に回る。初期値は仮の値で、実機を見て調整する) */
            float targetOffsetRight;
            /** 注視点から視点までのベクトルの初期値 */
            Vector3 toCameraPos;
            /** 右スティックを倒したときの回転速度(度/秒。初期値は、1フレームあたり1.3度を60fpsで回す速さ) */
            float rotateSpeed;
            /** 注視点から視点までのベクトルを正規化したときの、y成分の下限(これより下に回さない。上向きすぎの制限) */
            float minToCameraDirY;
            /** 注視点から視点までのベクトルを正規化したときの、y成分の上限(これより上に回さない。下向きすぎの制限) */
            float maxToCameraDirY;
            /** ニアクリップ */
            float nearClip;
            /** ファークリップ */
            float farClip;
            // [PARAM_GEN end members]
        };




        /*************************************************************/


        /**
         * @brief jsonからPlayerCameraのパラメーターを読み込む
         * @details 無いキー・型が違うキーは、パラメーターの初期値のままになる。
         *          使えない値(制限の大小が逆、真上・真下を向く制限、クリップの前後が逆など)は、K2_LOGを出して初期値に戻す。
         * @param root jsonのルート
         * @param[out] param 読み込み先。デフォルト構築された状態で渡される
         */
        void LoadPlayerCameraParameter(const JsonView& root, PlayerCameraParameter& param);

        /**
         * @brief PlayerCameraのパラメーターを、デバッグ画面(ParamDebugUI)に表示する
         * @param param 表示するパラメーター
         */
        void DrawPlayerCameraParameter(const PlayerCameraParameter& param);
    } // namespace camera
} // namespace app
