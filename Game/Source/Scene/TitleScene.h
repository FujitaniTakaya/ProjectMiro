/**
 * @file TitleScene.h
 * @brief タイトルシーン
 * @details ゲームの最初に表示するシーン。仮のタイトル画面で、右上のタイトルの下に「はじめる・オプション・おわる」が並ぶ。
 *          上下(十字キー・左スティック・W/S)で選び、Aボタン(J)かスタートボタン(Enter)で決める。
 *          - はじめる : 次のシーンへ進む
 *          - オプション : オプション画面(OptionMenu)を開く。背景は、直前のタイトル画面を少しぼかしたもの。Bボタン(K)で閉じて、タイトル画面に戻る
 *          - おわる : ゲームを終了する
 *          画面ごとのMenuをシーンが持ち、TitleState で、更新と描画をするMenuを切り替える。
 *          NOTE: 次のシーンは、InGameScene の中身ができるまでの仮で、遊べる DebugScene にしている。
 */
#pragma once
#include <cstdint>

#include "IScene.h"
#include "Source/UI/BlurredBackdrop.h"
#include "Source/UI/MenuInput.h"
#include "Source/UI/SystemPacket.h"


namespace app
{
    namespace ui
    {
        class OptionMenu;
        class TitleMenu;
        enum class EnTitleItem : uint8_t;
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
        /**
         * @brief 今、更新と描画をする画面
         */
        enum class TitleState : uint8_t
        {
            Title, //!< タイトル画面
            Option //!< オプション画面
        };


    private:
        /** @brief タイトル画面の更新。項目を選んで、決める */
        void TitleUpdate();

        /** @brief オプション画面の更新。画面が閉じられたら、タイトル画面に戻る */
        void OptionUpdate();

        /**
         * @brief 項目を決めた時の処理
         * @param item 決めた項目
         */
        void Decide(const ui::EnTitleItem item);


    private:
        /** タイトル画面のUI(背景・ロゴ・項目・選択カーソル) */
        ui::UIPacket<ui::TitleMenu> m_titlePacket;
        /** オプション画面のUI(背景・見出し・項目・設定画面) */
        ui::UIPacket<ui::OptionMenu> m_optionPacket;
        /** タイトル画面の項目を選ぶ方向入力。(オプション画面の入力は、OptionMenuが持つ) */
        ui::MenuInput m_menuInput;
        /** オプション画面の背景。オプション画面に入る直前のタイトル画面を、ぼかしたもの */
        ui::BlurredBackdrop m_backdrop;
        /** 今、更新と描画をする画面 */
        TitleState m_state;
        /** オプション画面に入って、背景にするタイトル画面を、まだ取り込んでいないかどうか。取り込みは描画の時に行う */
        bool m_isCapturePending;
        /** 「はじめる」が決まって、次のシーンへの遷移を要求するかどうか */
        bool m_isStartRequested;
    };
} // namespace app
