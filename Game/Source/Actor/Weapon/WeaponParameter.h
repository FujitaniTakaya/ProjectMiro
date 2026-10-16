/**
 * @file WeaponParameter.h
 * @brief ウェポンのパラメーター
 * @details 武器ごとにParamHolderへ登録して使う。登録は武器の実装(ハンドガンなど)で行う。
 *          例: const WeaponParameter& param = ParamHolder::Get().GetParameter<WeaponParameter>(EnParamID::Handgun);
 *              weapon.Init(param);
 */
#pragma once
#include <cstdint>

#include "Source/Core/Json/JsonView.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief ウェポンのパラメーター
         * @note 値は全て0で始まる。jsonに無いキーは0のままになる。
         */
        struct WeaponParameter
        {
            WeaponParameter();


            /** リロード時間(秒) */
            float reloadTime;
            /** マガジンサイズ(満タンの弾数) */
            uint32_t magazineSize;
        };




        /*************************************************************/


        /**
         * @brief jsonからウェポンのパラメーターを読み込む
         * @details 無いキー・型が違うキーは、パラメーターの初期値のままになる。
         * @param root jsonのルート
         * @param[out] param 読み込み先。デフォルト構築された状態で渡される
         */
        void LoadWeaponParameter(const JsonView& root, WeaponParameter& param);

        /**
         * @brief ウェポンのパラメーターを、デバッグ画面(ParamDebugUI)に表示する
         * @param param 表示するパラメーター
         */
        void DrawWeaponParameter(const WeaponParameter& param);
    } // namespace actor
} // namespace app
