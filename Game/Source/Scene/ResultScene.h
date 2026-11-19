/**
 * @file ResultScene.h
 * @brief リザルトシーン
 * @details インゲームの結果を表示するシーン。今は、右下の「タイトルへ・ゲームをやめる」の二択だけがある。(結果の表示は未実装)
 *          上下(十字キー・左スティック・W/S)で選び、Aボタン(J)かスタートボタン(Enter)で決める。
 *          - タイトルへ : タイトルシーンへ進む
 *          - ゲームをやめる : ゲームを終了する
 *          フェード中は、選ぶことも決めることもできない。
 */
#pragma once
#include <cstdint>

#include "IScene.h"
#include "Source/UI/Common/MenuInput.h"
#include "Source/UI/Foundation/SystemPacket.h"


namespace app
{
    namespace ui
    {
        class ResultMenu;
    } // namespace ui

    /**
     * @brief リザルトシーン
     */
    class ResultScene : public IScene
    {
        appScene(ResultScene);


    public:
        ResultScene();
        ~ResultScene();


        bool Start() override;
        void Update() override;
        void PauseUpdate() override;
        void Render(RenderContext& rc) override;
        void RenderUI(RenderContext& rc) override;

        bool RequestScene(uint32_t& id, float& waitTime) override;


    private:
        /** リザルト画面のUI(二択・選択カーソル) */
        ui::UIPacket<ui::ResultMenu> m_resultPacket;
        /** 項目を選ぶ方向入力 */
        ui::MenuInput m_menuInput;
        /** 「タイトルへ」が決まって、タイトルシーンへの遷移を要求するかどうか */
        bool m_isTitleRequested;
    };
} // namespace app
