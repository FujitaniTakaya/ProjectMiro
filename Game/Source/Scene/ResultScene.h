/**
 * @file ResultScene.h
 * @brief リザルトシーン
 * @details インゲームの結果を表示するシーン。左に、守衛時間・倒したゾンビの数・スコアの内訳(ScoreRule.h)を出す。
 *          結果は、インゲームが GameResult に書いたものを、Start() で読む。右下に、「タイトルへ・もういちど」の二択がある。
 *          上下(十字キー・左スティック・W/S)で選び、Aボタン(J)かスタートボタン(Enter)で決める。
 *          - タイトルへ : タイトルシーンへ進む
 *          - もういちど : もういちどインゲームシーンを遊ぶ
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
        /** 「タイトルへ」か「もういちど」が決まって、次のシーンへの遷移を要求するかどうか */
        bool m_isSceneRequested;
        /** 遷移を要求する次のシーン。m_isSceneRequested が true の時だけ意味がある */
        uint32_t m_nextSceneId;
    };
} // namespace app
