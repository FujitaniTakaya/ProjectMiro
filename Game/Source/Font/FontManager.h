/**
 * @file FontManager.h
 * @brief フォントの管理をするクラス
 * @details FONT_LIST のフォントを、起動時に全て BalloonEngine の SDFFontEngine に登録する。シングルトン。
 *          名前からフォントのID(FontId)を探して、FontRender::SetFont() などに渡す。
 *          例: FontRender::SetFont(FontManager::Get().Find("RoundedMplus"));
 */
#pragma once
#include <vector>


namespace app
{
    /**
     * @brief フォントの管理をするクラス
     */
    class FontManager : public Noncopyable
    {
    private:
        /**
         * @brief コンストラクタ
         * @details FONT_LIST の全てのフォントを、SDFFontEngine に登録する。(アトラスは、この時点で全て読み込まれる。)
         *          開けなかったフォントは、ログを出して飛ばす。
         */
        FontManager();
        ~FontManager();


    public:
        /**
         * @brief 名前からフォントのIDを探す
         * @param name フォントの名前(FONT_LIST の name)
         * @return フォントのID。見つからない場合や、読み込めなかったフォントは INVALID_FONT_ID
         */
        FontId Find(const char* name) const;

        /**
         * @brief 既定のフォントのIDを取得
         * @return FONT_LIST の最初のフォントのID。読み込めなかった場合は INVALID_FONT_ID
         */
        FontId GetDefaultFont() const;


    private:
        /** 登録したフォントのID。FONT_LIST と同じ並び。読み込めなかったフォントは INVALID_FONT_ID */
        std::vector<FontId> m_fontIds;


    public:
        /**
         * @brief インスタンスを生成する
         * @details RenderingEngine::Initialize() の後に呼ぶこと。
         *          起動時に全てのアトラスを読み込み、エンジンの資源(SDFFontEngine が持つフォント)を作るので、関数内staticにはせず、
         *          ApplicationのコンストラクタとデストラクタでCreateInstance()/DestroyInstance()を呼ぶ。
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
        static FontManager& Get();


    private:
        /** 唯一のインスタンス */
        static FontManager* m_instance;
    };
} // namespace app
