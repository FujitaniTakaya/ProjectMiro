/**
 * @file TextEncoding.h
 * @brief 文字コードの変換
 */
#pragma once
#include <string>


namespace app
{
    namespace util
    {
        /**
         * @brief UTF-8の文字列を、ワイド文字列(UTF-16)に変換する
         * @details jsonから読んだ文字列を、FontRenderなどのワイド文字列を受け取る関数に渡すときに使う。
         * @param utf8 UTF-8の文字列
         * @return ワイド文字列。空の文字列や、変換できない文字列は、空のワイド文字列になる。
         */
        std::wstring Utf8ToWide(const std::string& utf8);
    } // namespace util
} // namespace app
