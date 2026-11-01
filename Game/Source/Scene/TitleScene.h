/**
 * @file TitleScene.h
 * @brief タイトルシーン
 * @details ゲームの最初に表示するシーン。仮のタイトル画面で、スタートボタン(Enter) か Aボタン(J) を押すと次のシーンへ進む。
 *          NOTE: 次のシーンは、InGameScene の中身ができるまでの仮で、遊べる DebugScene にしている。
 */
#pragma once
#include <cstdint>

#include "IScene.h"
#include "Source/UI/SystemPacket.h"


namespace app
{
    namespace ui
    {
        class TitleMenu;
    } // namespace ui

    /**
     * @brief タイトルシーン
     */
    class TitleScene : public IScene
    {
        appScene(TitleScene);


    public:
        TitleScene();
        ~TitleScene();


        bool Start() override;
        void Update() override;
        void PauseUpdate() override;
        void Render(RenderContext& rc) override;
        void RenderUI(RenderContext& rc) override;

        bool RequestScene(uint32_t& id, float& waitTime) override;


    private:
        /** タイトル画面のUI(背景・ロゴ・サブタイトル・「PRESS START」) */
        ui::UIPacket<ui::TitleMenu> m_titlePacket;
        /** スタートが押されて、次のシーンへの遷移を要求するかどうか */
        bool m_isStartRequested;
    };
} // namespace app
