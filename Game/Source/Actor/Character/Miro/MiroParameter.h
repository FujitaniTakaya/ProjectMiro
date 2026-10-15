/**
 * @file MiroParameter.h
 * @brief Miroのパラメーター
 * @details ParamHolderに EnParamID::Miro で登録する。jsonは Assets/parameter/actor/Miro.json。
 *          例: const MiroParameter& param = ParamHolder::Get().GetParameter<MiroParameter>(EnParamID::Miro);
 */
#pragma once
#include "Source/Core/Json/JsonView.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief Miroのパラメーター
         * @note radius と height は Init() で一度だけ使うので、ホットリロードしても次に起動するまで反映されない。
         *       歩き・走りの速度は、共通ステートが移動するようになったら使う。
         */
        struct MiroParameter
        {
            MiroParameter();


            /** 歩きの移動速度(入力の倒し具合が最大のときの速さ) */
            float walkSpeed;
            /** 走りの移動速度(入力の倒し具合が最大のときの速さ) */
            float runSpeed;
            /** キャラクターコントローラーの半径 */
            float radius;
            /** キャラクターコントローラーの高さ */
            float height;
        };




        /*************************************************************/


        /**
         * @brief jsonからMiroのパラメーターを読み込む
         * @details 無いキー・型が違うキーは、パラメーターの初期値のままになる。
         * @param root jsonのルート
         * @param[out] param 読み込み先。デフォルト構築された状態で渡される
         */
        void LoadMiroParameter(const JsonView& root, MiroParameter& param);

        /**
         * @brief Miroのパラメーターを、デバッグ画面(ParamDebugUI)に表示する
         * @param param 表示するパラメーター
         */
        void DrawMiroParameter(const MiroParameter& param);
    } // namespace actor
} // namespace app
