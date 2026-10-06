/**
 * @file EffectHandle.h
 * @brief エフェクト再生ハンドルの型定義、エフェクトの種類と情報(軽量ヘッダ)
 */
#pragma once
#include <cstdint>
#include <iterator>


namespace app
{
    /** エフェクト再生ハンドル */
    using EffectHandle = uint32_t;
    /** ハンドル無効値 */
    static constexpr EffectHandle INVALID_EFFECT_HANDLE = 0xffffffff;



    /**
     * @brief エフェクトの種類
     * @details EFFECT_LISTと同じ並びにすること。値がそのままEffectEngine::ResistEffect()の登録番号になる。
     */
    enum class EnEffectKind : uint8_t
    {
        /** プレースホルダー。エフェクトを追加したら置き換えること。 */
        Sample = 0,
        Max,
        None = Max
    };



    /**
     * @brief エフェクトの情報
     */
    struct EffectInformation
    {
        /** @brief エフェクトファイル(.efk)のパス。空の場合は未登録として扱う。 */
        const char16_t* assetPath;


        /** @brief コンストラクタ */
        constexpr explicit EffectInformation(const char16_t* path)
            : assetPath(path)
        {}
    };



    inline constexpr EffectInformation EFFECT_LIST[] = {
        EffectInformation(u"")
    };

    static_assert(
        std::size(EFFECT_LIST) == static_cast<size_t>(EnEffectKind::Max),
        "EFFECT_LISTの数がEnEffectKindと合っていない。");

} // namespace app
