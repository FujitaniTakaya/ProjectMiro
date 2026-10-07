/**
 * @file ParamDebugUI.cpp
 * @brief パラメーターの状態確認用UI(ImGui)クラスの実装
 */
#include "stdafx.h"

#include "ParamDebugUI.h"

#include <cstring>
#include "ParamHolder.h"


namespace app
{
#ifdef BALLOON_IMGUI_ENABLED
    namespace
    {
        /** ウィンドウの初回の位置とサイズ。(SoundDebugUIのウィンドウと重ならない場所) */
        constexpr float WINDOW_POS_X = 440.0f;
        constexpr float WINDOW_POS_Y = 220.0f;
        constexpr float WINDOW_WIDTH = 680.0f;

        /** 列の幅 */
        constexpr float COLUMN_WIDTH_ID = 28.0f;
        constexpr float COLUMN_WIDTH_STATE = 160.0f;
        constexpr float COLUMN_WIDTH_LOADS = 44.0f;
        constexpr float COLUMN_WIDTH_LAST_LOAD = 80.0f;
        constexpr float COLUMN_WIDTH_RELOAD = 60.0f;

        /** 登録できるパラメーターの数 */
        constexpr size_t PARAM_COUNT = static_cast<size_t>(EnParamID::Max);

        /** 状態の色 */
        const ImVec4 COLOR_OK = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
        const ImVec4 COLOR_ERROR = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);


        /**
         * @brief 型名から、"struct " や "class " を取り除く
         * @details typeid(T).name()は、"struct app::MiniMapParameter" のような名前を返す。
         */
        const char* StripTypeName(const char* typeName)
        {
            if (std::strncmp(typeName, "struct ", 7) == 0) return typeName + 7;
            if (std::strncmp(typeName, "class ", 6) == 0) return typeName + 6;
            return typeName;
        }


        /**
         * @brief 読み込み状態を、色付きで描画する
         */
        void DrawState(const ParamDebugInfo& info)
        {
            if (!info.m_isLoaded)
            {
                // 1度も読み込めていない。(ファイルがない、jsonが壊れている、など)
                ImGui::TextColored(COLOR_ERROR, "NOT LOADED");
            }
            else if (info.m_isLastLoadFailed)
            {
                // 保存したjsonが壊れている、など。前に読み込めた値のまま。
                ImGui::TextColored(COLOR_ERROR, "FAILED (old value kept)");
            }
            else
            {
                ImGui::TextColored(COLOR_OK, "OK");
            }
        }
    } // namespace
#endif // BALLOON_IMGUI_ENABLED


    ParamDebugUI::ParamDebugUI()
        : m_isVisible(true)
    {
    }


    void ParamDebugUI::Draw()
    {
#ifdef BALLOON_IMGUI_ENABLED
        // F3キーで表示・非表示を切り替える。
        if (ImGui::IsKeyPressed(ImGuiKey_F3, false))
        {
            m_isVisible = !m_isVisible;
        }
        if (!m_isVisible)
        {
            return;
        }

        // NOTE: "BalloonEngine"(エンジン)と"Parameter"(Game)と"Sound"は、既に使われている名前なので使わない。
        ImGui::SetNextWindowPos(ImVec2(WINDOW_POS_X, WINDOW_POS_Y), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, 0.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Params", &m_isVisible))
        {
            ImGui::TextDisabled("F3: show / hide");

            ParamHolder& holder = ParamHolder::Get();

            // 登録されているパラメーターの数などを数える
            int registeredCount = 0;
            int loadedCount = 0;
            int failedCount = 0;
            int drawFuncCount = 0;
            for (size_t i = 0; i < PARAM_COUNT; ++i)
            {
                ParamDebugInfo info = {};
                if (!holder.GetDebugInfo(static_cast<EnParamID>(i), info)) continue;

                ++registeredCount;
                if (info.m_isLoaded) ++loadedCount;
                if (info.m_isLastLoadFailed) ++failedCount;
                if (info.m_hasDrawFunc) ++drawFuncCount;
            }

            if (registeredCount == 0)
            {
                ImGui::Text("No parameters registered.");
            }
            else
            {
                if (ImGui::Button("Reload all"))
                {
                    for (size_t i = 0; i < PARAM_COUNT; ++i)
                    {
                        holder.Reload(static_cast<EnParamID>(i));
                    }
                }
                ImGui::SameLine();
                ImGui::Text("Loaded %d / %d, failed %d", loadedCount, registeredCount, failedCount);

                // 一覧
                constexpr ImGuiTableFlags TABLE_FLAGS = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp;
                if (ImGui::BeginTable("params", 7, TABLE_FLAGS))
                {
                    ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, COLUMN_WIDTH_ID);
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableSetupColumn("File");
                    ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, COLUMN_WIDTH_STATE);
                    ImGui::TableSetupColumn("Loads", ImGuiTableColumnFlags_WidthFixed, COLUMN_WIDTH_LOADS);
                    ImGui::TableSetupColumn("Last load", ImGuiTableColumnFlags_WidthFixed, COLUMN_WIDTH_LAST_LOAD);
                    ImGui::TableSetupColumn("Reload", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHeaderLabel, COLUMN_WIDTH_RELOAD);
                    ImGui::TableHeadersRow();

                    for (size_t i = 0; i < PARAM_COUNT; ++i)
                    {
                        const EnParamID id = static_cast<EnParamID>(i);
                        ParamDebugInfo info = {};
                        if (!holder.GetDebugInfo(id, info)) continue;

                        ImGui::TableNextRow();
                        // NOTE: ボタンのIDが行ごとに重ならないようにする。
                        ImGui::PushID(static_cast<int>(i));

                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%d", static_cast<int>(i));
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextUnformatted(StripTypeName(info.m_typeName));
                        ImGui::TableSetColumnIndex(2);
                        ImGui::TextUnformatted(info.m_path->c_str());
                        ImGui::TableSetColumnIndex(3);
                        DrawState(info);
                        ImGui::TableSetColumnIndex(4);
                        ImGui::Text("%u", info.m_loadCount);
                        ImGui::TableSetColumnIndex(5);
                        if (info.m_secondsSinceLastLoad < 0.0f)
                        {
                            ImGui::TextDisabled("-");
                        }
                        else
                        {
                            ImGui::Text("%.1f s ago", info.m_secondsSinceLastLoad);
                        }
                        ImGui::TableSetColumnIndex(6);
                        if (ImGui::SmallButton("Reload"))
                        {
                            holder.Reload(id);
                        }

                        ImGui::PopID();
                    }
                    ImGui::EndTable();
                }

                // 値の表示関数が登録されているパラメーターは、開くと値が見られる
                if (drawFuncCount > 0)
                {
                    ImGui::Separator();
                    ImGui::TextDisabled("Values");
                    for (size_t i = 0; i < PARAM_COUNT; ++i)
                    {
                        const EnParamID id = static_cast<EnParamID>(i);
                        ParamDebugInfo info = {};
                        if (!holder.GetDebugInfo(id, info) || !info.m_hasDrawFunc) continue;

                        ImGui::PushID(static_cast<int>(i));
                        if (ImGui::TreeNode("value", "[%d] %s", static_cast<int>(i), StripTypeName(info.m_typeName)))
                        {
                            holder.DrawValue(id);
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                }
            }
        }
        ImGui::End();
#endif // BALLOON_IMGUI_ENABLED
    }
} // namespace app
