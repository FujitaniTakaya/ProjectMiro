/**
 * @file CoreParameter.h
 * @brief Coreのパラメーター
 * @details ParamHolderに EnParamID::Core で登録する。jsonは Assets/parameter/actor/Core.json。
 *          例: const CoreParameter& param = ParamHolder::Get().GetParameter<CoreParameter>(EnParamID::Core);
 */
#pragma once
#include "Source/Core/Json/JsonView.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief Coreのパラメーター
         */
        struct CoreParameter
        {
            CoreParameter();


            // [PARAM_GEN begin members] 自動生成(tools/param_gen.py)。手で編集しない
            /** 最大耐久値 */
            float maxHp;
            /** 耐久値バーの基準点の高さ(コアの中心から上へ。cm) */
            float hpBarOffsetY;
            // [PARAM_GEN end members]
        };




        /*************************************************************/


        /**
         * @brief jsonからCoreのパラメーターを読み込む
         * @details 無いキー・型が違うキーは、パラメーターの初期値のままになる。
         * @param root jsonのルート
         * @param[out] param 読み込み先。デフォルト構築された状態で渡される
         */
        void LoadCoreParameter(const JsonView& root, CoreParameter& param);

        /**
         * @brief Coreのパラメーターを、デバッグ画面(ParamDebugUI)に表示する
         * @param param 表示するパラメーター
         */
        void DrawCoreParameter(const CoreParameter& param);
    } // namespace actor
} // namespace app
