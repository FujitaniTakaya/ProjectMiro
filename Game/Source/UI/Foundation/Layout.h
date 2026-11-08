/**
 * @file Layout.h
 * @brief jsonからUIのパーツを作って配置する
 * @details jsonの"canvas"の"elements"に、UIのパーツを並べて書く。上に書いたものから順に描画される。
 *          jsonが更新されたら、UIのパーツを作り直す。(デバッグビルドのみ。HotReloadManagerを使っている。)
 *
 *          {
 *              "canvas": {
 *                  "elements": [
 *                      { "type": "UIIcon", "name": "Logo", "asset": "Assets/xxx.dds", "width": 400, "height": 300,
 *                        "position": [0, 0, 0], "scale": [1, 1, 1], "rotation": 0, "color": [255, 255, 255, 255] }
 *                  ]
 *              }
 *          }
 *
 *          "type"は、UIIcon、UIButton、UIGauge、UIText、UIDummy。"name"は、MenuBaseからUIのパーツを取り出す時の名前になる。
 *          "rotation"は、Z軸回りの角度(度)。"color"は0〜255。UIGaugeとUIText は、"pivot": [x, y]も指定できる。
 *          UITextは、フォントで文字を表示する。
 *              "text"     : 文字列(UTF-8)。"\n"で改行する。
 *              "font"     : フォントの名前(FontList.hのFONT_LISTの名前)。省略は、FONT_LISTの最初のフォント。
 *              "size"     : 文字の大きさ(ピクセル)。省略は、フォントの標準の大きさ。
 *              "align"    : 水平アライメント。"Left"(省略時) / "Center" / "Right"。複数行のときに効く。
 *              "lineSpacing" : 行間の倍率(省略は1.2)。
 *              "shadowOffset" : 影のずれ(ピクセル)。省略か0なら影なし。"shadowColor"は影の色(0〜255、省略は黒)。
 *              "pivot"    : 基点。(0, 0)が左下、(1, 1)が右上。省略は(0.5, 0.5)。"position"が基点の位置になる。
 *          例: { "type": "UIText", "name": "Infinity", "font": "RoundedMplus", "text": "∞", "size": 64, "position": [620, -460, 0] }
 */
#pragma once
#include <memory>
#include <string>

#include "Menu.h"
#include "Source/Core/HotReload/HotReloadHandle.h"
#include "Source/Core/Json/JsonView.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief jsonからUIのパーツを作って配置するクラス
         */
        class Layout : public Noncopyable
        {
        public:
            Layout();
            ~Layout();


        public:
            /**
             * @brief 初期化
             * @details Menuを作って、jsonを読み込み、UIのパーツを作る。
             *          jsonが読み込めなかった場合は、UIのパーツは作られない。(デバッグビルドでは、読み込めるようになった時に作られる。)
             * @tparam TMenu 作るMenuの型(MenuBaseを継承していること)
             * @param path jsonファイルのパス
             */
            template <typename TMenu>
            void Initialize(const std::string& path)
            {
                Initialize(std::make_unique<TMenu>(), path);
            }

            /** 更新 */
            void Update();

            /**
             * @brief 描画
             * @param rc レンダーコンテキスト
             */
            void Render(RenderContext& rc);


        public:
            /**
             * @brief Menuを取得
             * @tparam T 取得するMenuの型
             * @return Menu。初期化されていない場合や、型が違う場合はnullptr。
             */
            template <typename T>
            T* GetMenu() const
            {
                return dynamic_cast<T*>(m_menu.get());
            }


        private:
            /**
             * @brief 初期化
             * @param menu Menu。Layoutが所有する。
             * @param path jsonファイルのパス
             */
            void Initialize(std::unique_ptr<MenuBase> menu, const std::string& path);

            /**
             * @brief jsonからUIのパーツを作る
             * @details すでに同じ名前のUIのパーツがある場合は、作り直す。
             *          作り終えたら、MenuのInitializeLogic()を呼ぶ。
             * @param root jsonのルート
             */
            void Build(const JsonView& root);


        private:
            /** Menu */
            std::unique_ptr<MenuBase> m_menu;
            /** HotReloadManagerに登録したハンドル */
            HotReloadHandle m_hotReloadHandle;
        };
    } // namespace ui
} // namespace app
