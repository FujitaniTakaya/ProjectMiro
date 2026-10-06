/**
 * @file SoundDebugUI.h
 * @brief サウンドの音量調整用UI(ImGui)クラスの宣言
 * @details Master / BGM / SE / Voice の音量を、実行中にスライダーで調整できるウィンドウを表示する。
 *          BALLOON_IMGUI_ENABLED が定義されていない場合(Release)は、何もしない。
 *          F2キーで表示・非表示を切り替えられる。(F1はBalloonEngineのパラメータUI)
 *          ImGuiのフォントは日本語に対応していないので、ウィンドウの文字は英語にしている。
 */
#pragma once


namespace app
{
    /**
     * @brief サウンドの音量調整用UIクラス
     */
    class SoundDebugUI : public Noncopyable
    {
    public:
        SoundDebugUI();
        ~SoundDebugUI() = default;


    public:
        /**
         * @brief 描画関数
         * @note  ImGuiのフレーム内(NewFrame() と Render() の間)で呼ぶこと。Application::Update()から毎フレーム呼ぶ。
         *        SoundManagerの生成後に呼ぶこと。
         */
        void Draw();


        /**
         * @brief 表示・非表示を設定
         * @param isVisible 表示するかどうか
         */
        void SetVisible(const bool isVisible)
        {
            m_isVisible = isVisible;
        }


        /**
         * @brief 表示中かどうか
         * @return 表示中ならtrue
         */
        bool IsVisible() const
        {
            return m_isVisible;
        }


    private:
        /** 表示中かどうか */
        bool m_isVisible;
    };
} // namespace app
