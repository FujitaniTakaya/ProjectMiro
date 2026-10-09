/**
 * @file HotReloadHandle.h
 * @brief ホットリロードの登録ハンドルの型定義(軽量ヘッダ)
 */
#pragma once
#include <cstdint>


namespace app
{
    /** ホットリロードの登録ハンドル */
    using HotReloadHandle = uint32_t;
    /** ハンドル無効値 */
    static constexpr HotReloadHandle INVALID_HOT_RELOAD_HANDLE = 0xffffffff;

} // namespace app
