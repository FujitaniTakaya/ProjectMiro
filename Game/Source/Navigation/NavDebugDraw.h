/**
 * @file NavDebugDraw.h
 * @brief 経路探索のデバッグ表示
 * @details 経路探索の格子(歩ける・歩けないセル)と、ゲートからコアまでの経路を、画面に重ねて表示する。
 *          表示するかどうかなどは、ImGuiの「Navigation (DEBUG)」ウィンドウで切り替える。
 *          ImGuiが無い構成(BALLOON_IMGUI_ENABLEDが無い)では、何もしない。
 */
#pragma once
#include <vector>

#include "NavTypes.h"


namespace app
{
    namespace nav
    {
        /**
         * @brief 経路探索のデバッグ表示
         * @details 起動時の処理は無く、初めて呼ばれたときに作られるので、関数内staticの Get() で取得する。
         */
        class NavDebugDraw : public Noncopyable
        {
        public:
            /**
             * @brief インスタンスを取得する
             * @details 初めて呼んだときに作られ、プロセスの終了時に破棄される。生成・破棄の呼び出しは要らない。
             * @return インスタンス
             */
            static NavDebugDraw& Get();

            /**
             * @brief 毎フレームの表示
             * @details 設定のウィンドウと、格子・経路の重ね描きをする。経路は、始点それぞれから終点までを NavigationService で探して表示する。
             *          格子ができていなければ、ウィンドウにそう表示するだけ。
             * @param starts 経路の始点のワールド座標(ゲートの足元など)
             * @param goal 経路の終点のワールド座標(コアの足元など)
             */
            void Draw(const std::vector<Vector3>& starts, const Vector3& goal);


        private:
            /** コンストラクタ。Get() からだけ呼ばれる */
            NavDebugDraw();


        private:
            /** 格子のセルを表示するか */
            bool m_isGridVisible;
            /** 始点から終点までの経路を表示するか */
            bool m_isPathVisible;
            /** 格子を何セルおきに表示するか(1なら全部) */
            int m_gridStride;
            /** 始点それぞれから終点までの経路 */
            std::vector<std::vector<NavVec3>> m_paths;
            /** 経路を探した格子の幅。格子が作り直されたかを調べるのに使う */
            int m_pathGridWidth;
            /** 経路を探し直す必要があるか */
            bool m_isPathDirty;
        };
    } // namespace nav
} // namespace app
