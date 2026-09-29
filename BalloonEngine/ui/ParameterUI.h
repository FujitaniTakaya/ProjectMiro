/**
 * @file ParameterUI.h
 * @brief エンジンのパラメータ調整用UI(ImGui)クラスの宣言
 * @details ライト、ブルーム、被写界深度、G-Bufferデバッグ表示を、実行中に調整できるウィンドウを表示する。
 *          BALLOON_IMGUI_ENABLED が定義されている場合は、RenderingEngine が毎フレーム自動で呼ぶ。
 *          F1キーで表示・非表示を切り替えられる。
 */
#pragma once


namespace nsBalloonEngine
{
    /**
     * @brief エンジンのパラメータ調整用UIクラス
     */
    class ParameterUI : public Noncopyable
    {
    private:
        ParameterUI() = default;
        ~ParameterUI() = default;


    public:
        /**
         * @brief 描画関数
         * @note  ImGuiのフレーム内(NewFrame() と Render() の間)で呼ぶこと。
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
        /** ディレクションライトの項目を描画 */
        void DrawDirectionLight();
        /** 環境光の項目を描画 */
        void DrawAmbientLight();
        /** ポイントライトの項目を描画 */
        void DrawPointLight();
        /** スポットライトの項目を描画 */
        void DrawSpotLight();
        /** ブルームの項目を描画 */
        void DrawBloom();
        /** 被写界深度の項目を描画 */
        void DrawDepthOfField();
        /** デバッグの項目を描画 */
        void DrawDebug();


    private:
        /** 表示中かどうか */
        bool m_isVisible = true;


    public:
        /**
         * @brief インスタンスを取得
         * @return インスタンス
         */
        static ParameterUI& Get()
        {
            static ParameterUI instance;
            return instance;
        }
    };
} // namespace nsBalloonEngine
