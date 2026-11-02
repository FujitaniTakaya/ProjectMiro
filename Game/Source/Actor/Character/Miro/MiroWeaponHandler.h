/**
 * @file MiroWeaponHandler.h
 * @brief Miroが武器を使うための処理
 * @details Miroは武器を知らない。撃つ・リロードに要る処理は、持ち主(シーン)が武器に結びつけて渡す。
 *          例: miro.SetWeaponHandler({ fire, canReload, isEmpty, reload }) のように、ラムダで渡す。
 */
#pragma once
#include <functional>


namespace app
{
    namespace actor
    {
        /**
         * @brief Miroが武器を使うための処理
         * @details 設定していない(空の)処理は、「撃てない」「リロードできない」「弾切れではない」として扱われる。
         * @note 渡した処理が参照するものは、Miroが生きている間、有効であること。
         */
        struct MiroWeaponHandler
        {
            /** 1発撃つ。撃てたら true、撃てない(弾切れ・リロード中など)なら false を返すこと */
            std::function<bool()> fire;
            /** リロードできるか。リロード中でも満タンでもなければ true を返すこと */
            std::function<bool()> canReload;
            /** マガジンが空か。残弾が0なら true を返すこと */
            std::function<bool()> isEmpty;
            /** リロードを始める */
            std::function<void()> reload;
        };
    } // namespace actor
} // namespace app
