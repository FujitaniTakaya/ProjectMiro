/**
 * @file FontRender.h
 * @brief フォント描画クラスの宣言
 * @details SDFFontEngine で登録したフォントで、文字列を描画する。フォントは SetFont() でいつでも切り替えられる。
 *          描画は即時なので、Game側のUIの描画(RenderUI)の中で Draw() を呼ぶこと。
 *          RenderingEngine::Execute() より前に呼ぶと、ポストプロセスに上書きされて見えなくなる。
 */
#pragma once
#include "IRenderObject.h"
#include "SDFFontEngine.h"


namespace nsBalloonEngine
{
    /**
     * @brief フォント描画クラス
     */
    class FontRender : public IRenderObject
    {
    public:
        /** 文字列の最大文字数(終端を含む) */
        static constexpr int MAX_TEXT_SIZE = 256;


    public:
        FontRender();
        ~FontRender() override;


    public:
        /**
         * @brief 描画
         * @details フォント・文字列が無効な場合は、何も描画しない。
         * @param rc レンダーコンテキスト
         */
        void Draw(RenderContext& rc);


    public:
        /**
         * @brief フォントを設定
         * @param fontId SDFFontEngine::LoadFont() が返したフォントID
         */
        void SetFont(const FontId fontId)
        {
            m_fontId = fontId;
        }

        /**
         * @brief フォントを取得
         * @return フォントID
         */
        FontId GetFont() const
        {
            return m_fontId;
        }

        /**
         * @brief 表示する文字列を設定
         * @details '\n' で改行する。MAX_TEXT_SIZE - 1 文字を超えた分は切り捨てる。
         * @param text 文字列
         */
        void SetText(const wchar_t* text);

        /**
         * @brief 表示している文字列を取得
         * @return 文字列
         */
        const wchar_t* GetText() const
        {
            return m_text;
        }

        /**
         * @brief 座標を設定
         * @param position 画面の中心を原点にした座標(y上が正)
         */
        void SetPosition(const Vector2& position)
        {
            m_position = position;
        }

        /**
         * @brief 座標を設定
         * @param x X座標
         * @param y Y座標
         */
        void SetPosition(const float x, const float y)
        {
            m_position = Vector2(x, y);
        }

        /**
         * @brief 座標を取得
         * @return 座標
         */
        const Vector2& GetPosition() const
        {
            return m_position;
        }

        /**
         * @brief 回転を設定
         * @param rotation Z軸回りの回転(ラジアン)。座標の位置が回転の中心になる。
         */
        void SetRotation(const float rotation)
        {
            m_rotation = rotation;
        }

        /**
         * @brief 回転を取得
         * @return 回転(ラジアン)
         */
        float GetRotation() const
        {
            return m_rotation;
        }

        /**
         * @brief 拡大率を設定
         * @param scale 拡大率。1でフォントのem(アトラス生成時の -size)ぶんの高さ。
         */
        void SetScale(const float scale)
        {
            m_scale = Vector2(scale, scale);
        }

        /**
         * @brief 拡大率を設定
         * @param scale 拡大率(x=横幅、y=縦幅)
         */
        void SetScale(const Vector2& scale)
        {
            m_scale = scale;
        }

        /**
         * @brief 拡大率を取得
         * @return 拡大率
         */
        const Vector2& GetScale() const
        {
            return m_scale;
        }

        /**
         * @brief 基点を設定
         * @param pivot 基点。(0, 0)=左下、(0.5, 0.5)=中央、(1, 1)=右上。
         */
        void SetPivot(const Vector2& pivot)
        {
            m_pivot = pivot;
        }

        /**
         * @brief 基点を取得
         * @return 基点
         */
        const Vector2& GetPivot() const
        {
            return m_pivot;
        }

        /**
         * @brief 色を設定
         * @param red 赤成分
         * @param green 緑成分
         * @param blue 青成分
         * @param alpha アルファ成分
         */
        void SetColor(const float red, const float green, const float blue, const float alpha)
        {
            m_color = Vector4(red, green, blue, alpha);
        }

        /**
         * @brief 色を設定
         * @param color 色
         */
        void SetColor(const Vector4& color)
        {
            m_color = color;
        }

        /**
         * @brief 色を取得
         * @return 色
         */
        const Vector4& GetColor() const
        {
            return m_color;
        }

        /**
         * @brief 影のパラメーターを設定
         * @param enable 影を描画するか
         * @param offset 影のオフセット(ピクセル)
         * @param color 影の色
         */
        void SetShadowParam(const bool enable, const float offset, const Vector4& color)
        {
            m_isDrawShadow = enable;
            m_shadowOffset = offset;
            m_shadowColor = color;
        }

        /**
         * @brief 水平アライメントを設定
         * @param align Left / Center / Right
         */
        void SetTextAlign(const TextAlign align)
        {
            m_textAlign = align;
        }

        /**
         * @brief 行間倍率を設定
         * @param lineSpacing 1.0=隙間なし、1.2=1.2倍(既定)
         */
        void SetLineSpacing(const float lineSpacing)
        {
            m_lineSpacing = lineSpacing;
        }


    private:
        /** フォントID */
        FontId m_fontId;
        /** 描画する文字列 */
        wchar_t m_text[MAX_TEXT_SIZE];
        /** 座標 */
        Vector2 m_position;
        /** 回転(ラジアン) */
        float m_rotation;
        /** 拡大率(x=横幅、y=縦幅) */
        Vector2 m_scale;
        /** 基点 */
        Vector2 m_pivot;
        /** 色 */
        Vector4 m_color;
        /** 影を描画するか */
        bool m_isDrawShadow;
        /** 影のオフセット(ピクセル) */
        float m_shadowOffset;
        /** 影の色 */
        Vector4 m_shadowColor;
        /** 水平アライメント */
        TextAlign m_textAlign;
        /** 行間倍率 */
        float m_lineSpacing;
    };
} // namespace nsBalloonEngine
