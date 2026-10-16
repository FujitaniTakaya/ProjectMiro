/**
 * @file HandgunAmmoMenu.h
 * @brief ハンドガンの残段数UIを動かすMenu
 * @details 「残段数 / ∞」の、残段数の数字を動かす。(Assets/parameter/ui/HandgunAmmo.json)
 *          jsonに、次の名前のUIDigitを書く。それ以外のパーツ(ハンドガンの画像・「/」・「∞」)は動かさないので、コードでは取り出さない。
 *          - "AmmoNumber" : 残段数の数字。1の位がUIの座標で、桁が増えると左に伸びる
 */
#pragma once
#include <cstdint>

#include "Source/UI/Menu.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief ハンドガンの残段数UIを動かすMenu
         * @details SetAmmo() で残段数を受け取って、数字に反映する。予備弾は無限なので、残段数だけを表示する。
         */
        class HandgunAmmoMenu : public MenuBase
        {
        public:
            HandgunAmmoMenu();
            ~HandgunAmmoMenu() override;


        public:
            /**
             * @brief UIのパーツが作られた(作り直された)時の処理
             * @details 数字を取り出して、今の残段数を反映する。
             */
            void InitializeLogic() override;


        public:
            /**
             * @brief 残段数を設定
             * @param remainingAmmo マガジンの残弾数
             */
            void SetAmmo(const uint32_t remainingAmmo);

            /**
             * @brief 残段数を取得
             * @return 設定された残段数
             */
            uint32_t GetAmmo() const
            {
                return m_remainingAmmo;
            }


        private:
            /** 数字に、今の残段数を反映する */
            void ApplyAmmo();


        private:
            /** 残段数の数字。(所有はキャンバス) */
            UIDigit* m_ammoNumber;
            /** 残段数 */
            uint32_t m_remainingAmmo;
        };
    } // namespace ui
} // namespace app
