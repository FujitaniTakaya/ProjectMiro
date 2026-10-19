/**
 * @file FontList.h
 * @brief ゲームで使うフォントの一覧
 * @details フォントを足すときは、FONT_LISTに「名前」「アトラス画像(PNG)のパス」「グリフ情報(JSON)のパス」を1行で登録する。
 *          アトラスは tools/update_font.bat で作る。(msdf-atlas-gen の出力。詳しくは BalloonEngine の SDFFontEngine.h を参照。)
 *          登録したフォントは、FontManager が起動時に全て読み込む。名前は、UIText の json の "font" で使う。
 *          表の最初のフォントが、"font" を省略したときの既定のフォントになる。
 */
#pragma once
#include <cstddef>


namespace app
{
    /**
     * @brief フォントの情報
     */
    struct FontInformation
    {
        /** @brief フォントの名前。UIText の json の "font" に書く名前。 */
        const char* name;
        /** @brief アトラス画像(PNG)のパス */
        const char* atlasPngPath;
        /** @brief グリフ情報(JSON)のパス */
        const char* atlasJsonPath;


        /** @brief コンストラクタ */
        constexpr FontInformation(const char* fontName, const char* pngPath, const char* jsonPath)
            : name(fontName)
            , atlasPngPath(pngPath)
            , atlasJsonPath(jsonPath)
        {}
    };



    inline constexpr FontInformation FONT_LIST[] = {
        FontInformation("RoundedMplus", "Assets/font/rounded-mplus-1c-heavy.png", "Assets/font/rounded-mplus-1c-heavy.json")
    };



    /**
     * @brief 2つの文字列が同じかどうか(コンパイル時)
     * @param a 文字列
     * @param b 文字列
     * @return 同じならtrue
     */
    constexpr bool IsSameFontName(const char* a, const char* b)
    {
        while (*a != '\0' && *a == *b)
        {
            ++a;
            ++b;
        }
        return *a == *b;
    }



    /**
     * @brief 表の中に、同じ名前の行がないかを調べる(コンパイル時)
     * @details 名前から探すときは最初の行が選ばれるので、重複した行は黙って無視されてしまう。それを防ぐためのもの。
     * @param list 表
     * @return 重複がなければtrue
     */
    template <size_t N>
    constexpr bool IsUniqueFontName(const FontInformation (&list)[N])
    {
        for (size_t i = 0; i < N; i++)
        {
            for (size_t j = i + 1; j < N; j++)
            {
                if (IsSameFontName(list[i].name, list[j].name))
                {
                    return false;
                }
            }
        }
        return true;
    }

    static_assert(IsUniqueFontName(FONT_LIST), "FONT_LIST に、同じ名前の行がある。");

} // namespace app
