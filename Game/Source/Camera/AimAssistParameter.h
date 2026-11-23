/**
 * @file AimAssistParameter.h
 * @brief AimAssistのパラメーター
 * @details ParamHolderに EnParamID::AimAssist で登録する。jsonは Assets/parameter/camera/AimAssist.json。
 *          例: const AimAssistParameter& param = ParamHolder::Get().GetParameter<AimAssistParameter>(EnParamID::AimAssist);
 */
#pragma once
#include "Source/Core/Json/JsonView.h"


namespace app
{
    namespace camera
    {
        /**
         * @brief AimAssistのパラメーター
         * @details 構え中に、視線の近くにいるゾンビがあると、右スティックの回転を遅くし(slowScale)、スティックを倒している間は視線を狙う点へ寄せる(pullRate)。
         *          値はすべて毎フレーム読み直すので、ホットリロードですぐ反映される。
         */
        struct AimAssistParameter
        {
            AimAssistParameter();


            // [PARAM_GEN begin members] 自動生成(tools/param_gen.py)。手で編集しない
            /** 狙う点の高さ(ゾンビの足元から。cm)。胸のあたり */
            float targetHeight;
            /** カメラからこの距離(cm)より遠いゾンビは、アシストの対象にしない */
            float maxDistance;
            /** 視線から狙う点までの距離(cm)がこれ以内なら、アシストが効く。中心に近いほど強い */
            float assistRadius;
            /** 視線が狙う点のど真ん中にある時の、回転速度の倍率(1でオフ。小さいほど遅い) */
            float slowScale;
            /** 引き寄せの強さ。スティックを倒している間、残りの角度のうち、1秒あたりこの割合を詰める(0でオフ) */
            float pullRate;
            // [PARAM_GEN end members]
        };




        /*************************************************************/


        /**
         * @brief jsonからAimAssistのパラメーターを読み込む
         * @details 無いキー・型が違うキーは、パラメーターの初期値のままになる。
         *          使えない値(距離・半径が0以下、引き寄せの強さが負)は、K2_LOGを出して初期値に戻す。減速の倍率は、0.05〜1に収める。
         * @param root jsonのルート
         * @param[out] param 読み込み先。デフォルト構築された状態で渡される
         */
        void LoadAimAssistParameter(const JsonView& root, AimAssistParameter& param);

        /**
         * @brief AimAssistのパラメーターを、デバッグ画面(ParamDebugUI)に表示する
         * @param param 表示するパラメーター
         */
        void DrawAimAssistParameter(const AimAssistParameter& param);
    } // namespace camera
} // namespace app
