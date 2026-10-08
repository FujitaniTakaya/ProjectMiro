/**
 * @file SoundDebugUI.cpp
 * @brief サウンドの音量調整用UI(ImGui)クラスの実装
 */
#include "stdafx.h"

#include "SoundDebugUI.h"

#include "SoundManager.h"


namespace app
{
#ifdef BALLOON_IMGUI_ENABLED
    namespace
    {
        /** スライダーの幅 */
        constexpr float SLIDER_WIDTH = 150.0f;
        /** ウィンドウの初回の位置とサイズ */
        constexpr float WINDOW_POS_X = 440.0f;
        constexpr float WINDOW_POS_Y = 60.0f;
        constexpr float WINDOW_WIDTH = 380.0f;


        /**
         * @brief 音量のスライダーと、親の音量を含めた実効音量を描画する
         * @param label スライダーの名前(ImGuiのIDにもなるので、重複させないこと)
         * @param volume 調整する音量
         */
        void DrawVolumeSlider(const char* label, SoundVolume& volume)
        {
            // NOTE: 毎フレーム現在の値を読むので、コード側から音量を変えても、スライダーに反映される。
            float value = volume.GetVolume();

            ImGui::SetNextItemWidth(SLIDER_WIDTH);
            if (ImGui::SliderFloat(label, &value, 0.0f, 1.0f, "%.2f"))
            {
                // 範囲外の丸めと、再生中の音への反映は、SoundVolume側が行う。
                volume.SetVolume(value);
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(effective %.2f)", volume.GetEffectiveVolume());
        }
    } // namespace
#endif // BALLOON_IMGUI_ENABLED


    SoundDebugUI::SoundDebugUI()
        : m_isVisible(true)
    {
    }


    void SoundDebugUI::Draw()
    {
#ifdef BALLOON_IMGUI_ENABLED
        // F2キーで表示・非表示を切り替える。
        if (ImGui::IsKeyPressed(ImGuiKey_F2, false))
        {
            m_isVisible = !m_isVisible;
        }
        if (!m_isVisible)
        {
            return;
        }

        // NOTE: "BalloonEngine"(エンジン)と"Parameter"(Game)は、既に使われている名前なので使わない。
        ImGui::SetNextWindowPos(ImVec2(WINDOW_POS_X, WINDOW_POS_Y), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, 0.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Sound", &m_isVisible))
        {
            ImGui::TextDisabled("F2: show / hide");

            SoundManager& sound = SoundManager::Get();
            DrawVolumeSlider("Master", sound.GetMaster());
            ImGui::Separator();
            DrawVolumeSlider("BGM", sound.GetBGM());
            DrawVolumeSlider("SE", sound.GetSE());
            DrawVolumeSlider("Voice", sound.GetVoice());
        }
        ImGui::End();
#endif // BALLOON_IMGUI_ENABLED
    }
} // namespace app
