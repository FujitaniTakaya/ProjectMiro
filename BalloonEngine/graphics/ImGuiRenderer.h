/**
 * @file ImGuiRenderer.h
 * @brief ImGuiの初期化・フレーム処理・描画を担当するクラスの宣言
 * @details RenderingEngine が内部から呼ぶ。Game側が直接触る必要はない。
 */
#pragma once


namespace nsBalloonEngine
{
    /**
     * @brief ImGuiの初期化・フレーム処理・描画を担当するクラス
     */
    class ImGuiRenderer : public Noncopyable
    {
    private:
        ImGuiRenderer() = default;
        ~ImGuiRenderer() = default;


    public:
        /**
         * @brief 初期化関数
         * @details ImGuiのコンテキスト作成、DX12/Win32バックエンドの初期化、
         *          ウィンドウプロシージャへの接続(自動フック)を行う。
         * @param hwnd ウィンドウハンドル。nullptrの場合は、呼び出しスレッドのウィンドウから探す。
         */
        void Initialize(HWND hwnd = nullptr);

        /**
         * @brief フレーム開始関数
         */
        void NewFrame();

        /**
         * @brief 描画関数
         * @note  バックバッファが描画先に設定された状態で、EndFrameより前に呼ぶこと。
         */
        void Render();

        /**
         * @brief 終了関数
         * @note  グラフィックスエンジンを破棄する前に呼ぶこと。
         */
        void Finalize();

        /**
         * @brief 初期化済みかどうか
         */
        bool IsInitialized() const
        {
            return m_isInitialized;
        }


    private:
        /** 初期化済みフラグ */
        bool m_isInitialized = false;
        /** ImGui用のSRVディスクリプタヒープ */
        ComPtr<ID3D12DescriptorHeap> m_srvHeap;
        /** フックしたウィンドウ */
        HWND m_hwnd = nullptr;


    public:
        /**
         * @brief インスタンスを取得
         * @return インスタンス
         */
        static ImGuiRenderer& Get()
        {
            static ImGuiRenderer instance;
            return instance;
        }
    };
} // namespace nsBalloonEngine
