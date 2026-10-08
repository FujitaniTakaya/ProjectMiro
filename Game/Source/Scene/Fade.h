/**
 * @file Fade.h
 * @brief 画面の暗転・明転をするクラス
 * @details シーンの遷移(SceneManager)で使う。シングルトン。
 *          フェードアウトが完了した後は、FadeIn()を呼ぶまで画面は暗いままになる。
 *          例: Fade::Get().FadeOut(1.0f);
 *              if (Fade::Get().IsFadeOutComplete()) { ... }
 *          NOTE: 描画はRenderUI()の中でRender()を呼ぶこと。Render()の中だと、ポストプロセスに上書きされて見えなくなる。
 */
#pragma once


namespace app
{
    /**
     * @brief 画面の暗転・明転をするクラス
     */
    class Fade : public Noncopyable
    {
    private:
        Fade();
        ~Fade();


    public:
        /** @brief 更新 */
        void Update();


        /**
         * @brief 描画
         * @param rc レンダーコンテキスト
         */
        void Render(RenderContext& rc);


        /**
         * @brief フェードアウトを開始する
         * @param duration フェードアウトが完了するまでの時間(秒)
         */
        void FadeOut(float duration);


        /**
         * @brief フェードインを開始する
         * @param duration フェードインが完了するまでの時間(秒)
         */
        void FadeIn(float duration);


        /** 現在フェードをしているか */
        bool IsFading() const
        {
            return m_state != FadeState::None;
        }


        /** フェードアウトが完了したか(画面が完全に暗い状態) */
        bool IsFadeOutComplete() const
        {
            return m_state == FadeState::FadeOut && m_timer >= m_duration;
        }


    public:
        /**
         * @brief インスタンスを生成する
         * @details スプライトを読み込むので、エンジンの初期化後に呼ぶこと。
         */
        static void CreateInstance();


        /**
         * @brief インスタンスを破棄する
         */
        static void DestroyInstance();


        /**
         * @brief インスタンスを取得する
         * @return インスタンス。CreateInstance()を呼ぶ前に取得してはいけない。
         */
        static Fade& Get();


    private:
        /**
         * @brief フェードの状態
         */
        enum class FadeState
        {
            /** 何もしていない状態 */
            None,
            /** フェードイン(暗い→明るい) */
            FadeIn,
            /** フェードアウト(明るい→暗い) */
            FadeOut,
        };


    private:
        /**
         * @brief 暗幕のアルファ値を計算する
         * @return 0(透明)〜1(不透明)
         */
        float CalcAlpha() const;


    private:
        /** 暗幕のスプライト */
        Sprite m_fadeSprite;
        /** フェードの状態 */
        FadeState m_state;
        /** フェードの経過時間(フェードアウト中は増え、フェードイン中は減る) */
        float m_timer;
        /** フェードが完了するまでの時間 */
        float m_duration;


    private:
        /** 唯一のインスタンス */
        static Fade* m_instance;
    };
}
