/**
 * @file NavDebugDraw.cpp
 * @brief 経路探索のデバッグ表示
 */
#include "stdafx.h"

#include "NavDebugDraw.h"

#include <algorithm>
#include <cmath>

#include "NavGrid.h"
#include "NavigationService.h"


namespace app
{
    namespace nav
    {
#ifdef BALLOON_IMGUI_ENABLED
        namespace
        {
            /** 歩けるセルの色 */
            constexpr ImU32 WALKABLE_COLOR = IM_COL32(60, 220, 90, 160);
            /** 歩けないセルの色 */
            constexpr ImU32 BLOCKED_COLOR = IM_COL32(230, 60, 60, 140);
            /** 経路の線の色 */
            constexpr ImU32 PATH_COLOR = IM_COL32(255, 230, 40, 255);
            /** ウェイポイントの色 */
            constexpr ImU32 WAYPOINT_COLOR = IM_COL32(40, 200, 255, 255);
            /** セルを表す点の半径(ピクセル) */
            constexpr float CELL_DOT_RADIUS = 2.0f;
            /** ウェイポイントを表す点の半径(ピクセル) */
            constexpr float WAYPOINT_RADIUS = 5.0f;
            /** 経路の線の太さ(ピクセル) */
            constexpr float PATH_THICKNESS = 3.0f;
            /** 経路を地面から少し浮かせて描く高さ(cm)。地面に埋もれて見えなくならないように */
            constexpr float PATH_LIFT = 20.0f;
            /** 格子の表示間隔(セル数)の最小 */
            constexpr int MIN_GRID_STRIDE = 1;
            /** 格子の表示間隔(セル数)の最大 */
            constexpr int MAX_GRID_STRIDE = 8;


            /**
             * @brief ワールド座標を、ImGuiの画面座標にする
             * @param worldPosition ワールド座標
             * @param[out] outScreen ImGuiの画面座標(左上が原点)
             * @return 画面の中に見える位置なら true。カメラの後ろ・視錐台の外なら false
             */
            bool WorldToScreen(const Vector3& worldPosition, ImVec2& outScreen)
            {
                if (g_camera3D == nullptr)
                {
                    return false;
                }

                // カメラの後ろ(w <= 0)や視錐台の外の点は、割り算の結果が画面の反対側になってしまうので、先に弾く
                Vector4 clip(worldPosition.x, worldPosition.y, worldPosition.z, 1.0f);
                g_camera3D->GetViewProjectionMatrix().Apply(clip);
                if (clip.w <= 0.0f || std::fabs(clip.x) > clip.w || std::fabs(clip.y) > clip.w || clip.z < 0.0f || clip.z > clip.w)
                {
                    return false;
                }

                const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
                outScreen.x = (clip.x / clip.w * 0.5f + 0.5f) * displaySize.x;
                outScreen.y = (1.0f - (clip.y / clip.w * 0.5f + 0.5f)) * displaySize.y;
                return true;
            }
        } // namespace
#endif // BALLOON_IMGUI_ENABLED


        NavDebugDraw::NavDebugDraw()
            : m_isGridVisible(false)
            , m_isPathVisible(true)
            , m_gridStride(3)
            , m_paths()
            , m_pathGridWidth(0)
            , m_isPathDirty(true)
        {
        }


        NavDebugDraw& NavDebugDraw::Get()
        {
            static NavDebugDraw instance;
            return instance;
        }


        void NavDebugDraw::Draw(const std::vector<Vector3>& starts, const Vector3& goal)
        {
#ifdef BALLOON_IMGUI_ENABLED
            const NavigationService& navigation = NavigationService::Get();
            const NavGrid& grid = navigation.GetGrid();

            // 設定のウィンドウ
            if (ImGui::Begin("Navigation (DEBUG)"))
            {
                if (!navigation.IsReady())
                {
                    ImGui::TextUnformatted("格子がありません");
                }
                else
                {
                    ImGui::Text("格子 %d x %d  セル %.1fcm", grid.GetWidth(), grid.GetDepth(), grid.GetCellSize());
                }
                ImGui::Checkbox("セルを表示(緑=歩ける 赤=歩けない)", &m_isGridVisible);
                ImGui::SliderInt("セルの間隔", &m_gridStride, MIN_GRID_STRIDE, MAX_GRID_STRIDE);
                ImGui::Checkbox("始点から終点までの経路を表示", &m_isPathVisible);
                if (ImGui::Button("経路を探し直す"))
                {
                    m_isPathDirty = true;
                }
            }
            ImGui::End();

            if (!navigation.IsReady())
            {
                m_paths.clear();
                m_pathGridWidth = 0;
                m_isPathDirty = true;
                return;
            }

            // 経路は、格子が作り直されたときと、ボタンを押したときに探し直す
            if (m_isPathDirty || m_pathGridWidth != grid.GetWidth() || m_paths.size() != starts.size())
            {
                m_paths.assign(starts.size(), std::vector<NavVec3>());
                for (size_t i = 0; i < starts.size(); ++i)
                {
                    navigation.FindPath(starts[i], goal, m_paths[i]);
                }
                m_pathGridWidth = grid.GetWidth();
                m_isPathDirty = false;
            }

            ImDrawList* drawList = ImGui::GetBackgroundDrawList();

            // セル
            if (m_isGridVisible)
            {
                const int stride = std::clamp(m_gridStride, MIN_GRID_STRIDE, MAX_GRID_STRIDE);
                for (int cellZ = 0; cellZ < grid.GetDepth(); cellZ += stride)
                {
                    for (int cellX = 0; cellX < grid.GetWidth(); cellX += stride)
                    {
                        const NavGrid::Cell& cell = grid.GetCell(cellX, cellZ);
                        ImVec2 screen;
                        if (WorldToScreen(Vector3(grid.CellCenterX(cellX), cell.height, grid.CellCenterZ(cellZ)), screen))
                        {
                            drawList->AddCircleFilled(screen, CELL_DOT_RADIUS, cell.walkable ? WALKABLE_COLOR : BLOCKED_COLOR, 4);
                        }
                    }
                }
            }

            // 経路
            if (m_isPathVisible)
            {
                for (const std::vector<NavVec3>& path : m_paths)
                {
                    ImVec2 previous;
                    bool hasPrevious = false;
                    for (const NavVec3& waypoint : path)
                    {
                        ImVec2 screen;
                        const bool isVisible = WorldToScreen(Vector3(waypoint.x, waypoint.y + PATH_LIFT, waypoint.z), screen);
                        if (isVisible)
                        {
                            if (hasPrevious)
                            {
                                drawList->AddLine(previous, screen, PATH_COLOR, PATH_THICKNESS);
                            }
                            drawList->AddCircleFilled(screen, WAYPOINT_RADIUS, WAYPOINT_COLOR, 8);
                            previous = screen;
                        }
                        hasPrevious = isVisible;
                    }
                }
            }
#else
            (void)starts;
            (void)goal;
#endif // BALLOON_IMGUI_ENABLED
        }
    } // namespace nav
} // namespace app
