/**
 * @file FontRender.cpp
 * @brief フォント描画クラスの実装
 */
#include "BalloonEnginePreCompile.h"

#include "FontRender.h"

#include <cwchar>


namespace nsBalloonEngine
{
    FontRender::FontRender()
        : m_fontId(INVALID_FONT_ID)
        , m_text{}
        , m_position(0.0f, 0.0f)
        , m_rotation(0.0f)
        , m_scale(1.0f, 1.0f)
        , m_pivot(0.5f, 0.5f)
        , m_color(Vector4::White)
        , m_isDrawShadow(false)
        , m_shadowOffset(0.0f)
        , m_shadowColor(0.0f, 0.0f, 0.0f, 0.0f)
        , m_textAlign(TextAlign::Left)
        , m_lineSpacing(1.2f)
    {}


    FontRender::~FontRender()
    {}


    void FontRender::SetText(const wchar_t* text)
    {
        if (text == nullptr)
        {
            m_text[0] = L'\0';
            return;
        }
        wcsncpy_s(m_text, MAX_TEXT_SIZE, text, _TRUNCATE);
    }


    void FontRender::Draw(RenderContext& rc)
    {
        SDFFontEngine& engine = SDFFontEngine::Get();
        if (!engine.IsValidFont(m_fontId) || m_text[0] == L'\0')
        {
            return;
        }

        engine.SetShadowParam(m_isDrawShadow, m_shadowOffset, m_shadowColor);
        engine.SetTextAlign(m_textAlign);
        engine.SetLineSpacing(m_lineSpacing);

        engine.BeginDraw(rc, m_rotation, m_position);
        engine.Draw(m_fontId, m_text, m_position, m_color, m_scale, m_pivot);
        engine.EndDraw(rc);
    }
} // namespace nsBalloonEngine
