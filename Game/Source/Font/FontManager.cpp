/**
 * @file FontManager.cpp
 * @brief フォントの管理をするクラス
 */
#include "stdafx.h"

#include "FontManager.h"

#include "FontList.h"


namespace app
{
    FontManager* FontManager::m_instance = nullptr;


    FontManager::FontManager()
    {
        m_fontIds.reserve(sizeof(FONT_LIST) / sizeof(FONT_LIST[0]));

        // 表の全てのフォントを、SDFFontEngineに登録する。
        for (const FontInformation& info : FONT_LIST)
        {
            const FontId fontId = SDFFontEngine::Get().LoadFont(info.atlasPngPath, info.atlasJsonPath);
            if (fontId == INVALID_FONT_ID)
            {
                K2_LOG("フォントの登録に失敗しました。name=%s\n", info.name);
            }
            // 失敗したフォントも、表と同じ並びを保つために無効なIDで入れる。
            m_fontIds.push_back(fontId);
        }
    }


    FontManager::~FontManager()
    {}


    FontId FontManager::Find(const char* name) const
    {
        if (name == nullptr)
        {
            return INVALID_FONT_ID;
        }

        for (size_t i = 0; i < m_fontIds.size(); i++)
        {
            if (IsSameFontName(FONT_LIST[i].name, name))
            {
                return m_fontIds[i];
            }
        }
        return INVALID_FONT_ID;
    }


    FontId FontManager::GetDefaultFont() const
    {
        return m_fontIds.empty() ? INVALID_FONT_ID : m_fontIds.front();
    }


    void FontManager::CreateInstance()
    {
        K2_ASSERT(m_instance == nullptr, "FontManagerは既に生成されている。");
        if (m_instance == nullptr)
        {
            m_instance = new FontManager();
        }
    }


    void FontManager::DestroyInstance()
    {
        delete m_instance;
        m_instance = nullptr;
    }


    FontManager& FontManager::Get()
    {
        K2_ASSERT(m_instance != nullptr, "FontManager::CreateInstance()を先に呼ぶこと。");
        return *m_instance;
    }
} // namespace app
