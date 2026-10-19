/**
 * @file TextEncoding.cpp
 * @brief 文字コードの変換
 */
#include "stdafx.h"

#include "TextEncoding.h"


namespace app
{
    namespace util
    {
        std::wstring Utf8ToWide(const std::string& utf8)
        {
            if (utf8.empty())
            {
                return std::wstring();
            }

            const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
            if (length <= 0)
            {
                return std::wstring();
            }

            std::wstring wide(static_cast<size_t>(length), L'\0');
            MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), static_cast<int>(utf8.size()), wide.data(), length);
            return wide;
        }
    } // namespace util
} // namespace app
