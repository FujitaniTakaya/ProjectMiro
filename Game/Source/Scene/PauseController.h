/**
 * @file PauseController.h
 * @brief ポーズ画面の流れを動かすクラス
 * @details ゲームプレイ中にスタートボタンで開く。ゲーム画面がぼけて、「オプション」「タイトルへ」が出る。
 *          上下(十字キー・左スティック・W/S)で選び、Aボタンで決める。Bボタンかスタートボタンでゲームに戻る。
 *          - オプション : オプション画面(OptionMenu)を開く。Bボタンで閉じて、ポーズ画面に戻る
 *          - タイトルへ : タイトルへの遷移を要求する。(IsTitleRequested() が true になる。遷移そのものは SceneManager が行う)
 *          ポーズ中は SceneManager がシーンの Update() を止めるので、ゲームは止まる。
 *          ぼけ具合と画面の透明度は同じ値 m_fade で動かし、開く・閉じる時に 0〜1 を行き来する。
 *          ゲームに戻る時は、ぼけが戻りきり、BボタンとスタートボタンがはなされるまでClosedにならない。
 *          (押したまま戻ると、プレイヤーの操作(Bはダッシュ)が、戻った瞬間に押されたことになってしまう)
 */
#pragma once
#include <cstdint>

#include "Source/UI/MenuInput.h"
#include "Source/UI/SystemPacket.h"


namespace app
{
    namespace ui
    {
        class OptionMenu;
        class PauseMenu;
        enum class EnPauseItem : uint8_t;
    } // namespace ui

    /**
     * @brief ポーズ画面の流れを動かすクラス
     */
    class PauseController : public Noncopyable
    {
    public:
        PauseController();
        ~PauseController();


    public:
        /**
         * @brief ポーズ画面を開く
         * @details Closed のときだけ有効。ぼけ始める。呼んだフレームの入力は受け取らない。
         */
        void Open();

        /**
         * @brief 更新
         * @details ポーズ中(IsPaused() が true の間)に、毎フレーム呼ぶ。
         */
        void Update();

        /**
         * @brief UIの描画
         * @details ポーズ中だけ描く。ポストプロセスの後に呼ぶこと。
         * @param rc レンダーコンテキスト
         */
        void RenderUI(RenderContext& rc);

        /**
         * @brief 閉じた状態に戻す
         * @details ぼけも、すぐ解く。シーンが切り替わった時に呼ぶ。
         */
        void Reset();


    public:
        /**
         * @brief ポーズ中かどうか
         * @details 開き始めてから、ゲームに戻りきるまで true。この間は、シーンの Update() を止めること。
         * @return ポーズ中ならtrue
         */
        bool IsPaused() const
        {
            return m_state != PauseState::Closed;
        }

        /**
         * @brief タイトルへの遷移を要求しているかどうか
         * @details 「タイトルへ」を決めると true になる。Reset() で false に戻る。
         * @return 要求していればtrue
         */
        bool IsTitleRequested() const
        {
            return m_state == PauseState::ToTitle;
        }


    public:
        /**
         * @brief ポーズ画面を開く操作(スタートボタン)をしたかどうか
         * @return このフレームに押したらtrue
         */
        static bool IsOpenTriggered();


    private:
        /**
         * @brief ポーズ画面の状態
         */
        enum class PauseState : uint8_t
        {
            Closed,  //!< 閉じている(ゲームが動いている)
            Opening, //!< ぼけ始めている。入力は受け付けない
            Menu,    //!< 「オプション」「タイトルへ」を選んでいる
            Option,  //!< オプション画面を開いている
            Closing, //!< ゲームに戻っている。ぼけが戻りきり、ボタンがはなされるまで待つ
            ToTitle  //!< 「タイトルへ」を決めた。遷移が終わるまで、そのまま見せる
        };


    private:
        /**
         * @brief 開き始めの更新。ぼけが最大になったら、項目を選べるようにする
         * @param deltaTime 前のフレームからの経過時間(秒)
         */
        void OpeningUpdate(const float deltaTime);

        /**
         * @brief 項目を選ぶ画面の更新。項目を選んで、決める。Bボタンかスタートボタンで、ゲームに戻り始める
         * @param deltaTime 前のフレームからの経過時間(秒)
         */
        void MenuUpdate(const float deltaTime);

        /** @brief オプション画面の更新。画面が閉じられたら、項目を選ぶ画面に戻る */
        void OptionUpdate();

        /**
         * @brief ゲームに戻っている間の更新。ぼけが戻りきり、ボタンがはなされたら、閉じる
         * @param deltaTime 前のフレームからの経過時間(秒)
         */
        void ClosingUpdate(const float deltaTime);

        /**
         * @brief 項目を決めた時の処理
         * @param item 決めた項目
         */
        void Decide(const ui::EnPauseItem item);

        /** @brief ぼけ具合(エンジン)と、ポーズ画面の透明度に、m_fade を反映する */
        void ApplyFade();


    private:
        /** ポーズ画面のUI(背景・見出し・項目・選択カーソル) */
        ui::UIPacket<ui::PauseMenu> m_menuPacket;
        /** オプション画面のUI(背景・見出し・項目・設定画面) */
        ui::UIPacket<ui::OptionMenu> m_optionPacket;
        /** 項目を選ぶ方向入力。(オプション画面の入力は、OptionMenuが持つ) */
        ui::MenuInput m_menuInput;
        /** 状態 */
        PauseState m_state;
        /** ぼけ具合とポーズ画面の透明度(0〜1)。0で通常の画面、1で完全にぼけてポーズ画面が見える */
        float m_fade;
    };
} // namespace app
