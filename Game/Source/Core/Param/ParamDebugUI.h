/**
 * @file ParamDebugUI.h
 * @brief パラメーターの状態確認用UI(ImGui)クラスの宣言
 * @details ParamHolderに登録された全てのパラメーターの、読み込み状態を一覧で表示するウィンドウを出す。
 *          型名・ファイル・読み込み状態(OK / FAILED / NOT LOADED)・読み込み回数・最後に読み込めてからの経過時間と、再読み込みボタンがある。
 *          値の表示関数が登録されているパラメーターは、行を開くと値も見られる。(ParamList.cppで登録する。)
 *          BALLOON_IMGUI_ENABLED が定義されていない場合(Release)は、何もしない。
 *          F3キーで表示・非表示を切り替えられる。(F1はBalloonEngineのパラメータUI、F2はSoundDebugUI)
 *          ImGuiのフォントは日本語に対応していないので、ウィンドウの文字は英語にしている。
 */
#pragma once


namespace app
{
    /**
     * @brief パラメーターの状態確認用UIクラス
     */
    class ParamDebugUI : public Noncopyable
    {
    public:
        ParamDebugUI();
        ~ParamDebugUI() = default;


    public:
        /**
         * @brief 描画関数
         * @note  ImGuiのフレーム内(NewFrame() と Render() の間)で呼ぶこと。Application::Update()から毎フレーム呼ぶ。
         *        ParamHolderの生成後に呼ぶこと。
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
