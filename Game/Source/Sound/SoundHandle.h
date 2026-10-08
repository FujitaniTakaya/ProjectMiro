/**
 * @file SoundHandle.h
 * @brief サウンド再生ハンドルの型定義(軽量ヘッダ)
 */
#pragma once
#include <cstddef>
#include <cstdint>


namespace app
{
    /** サウンド再生ハンドル */
    using SoundHandle = uint32_t;
    /** ハンドル無効値 */
    static constexpr SoundHandle INVALID_SOUND_HANDLE = 0xffffffff;



    /**
     * @brief サウンドの種類
     * @details 鳴らす音ごとの項目。SOUND_LISTに、このIDとサウンドファイルのパスを1行で登録する。
     *          NOTE: 今あるBGM / SE / Voiceは、音を足すまでの仮の項目(グループ名ではない)。音を足したら置き換えること。
     */
    enum class EnSoundID : uint8_t
    {
        /** BGM */
        BGM = 0,
        /** SE */
        SE,
        /** Voice */
        Voice,
        Max,
        None = Max
    };



    /**
     * @brief サウンドの情報
     */
    struct SoundInformation
    {
        /** @brief サウンドのID */
        EnSoundID id;
        /** @brief サウンドファイルのパス */
        const char* filePath;


        /** @brief コンストラクタ */
        constexpr SoundInformation(const EnSoundID ID, const char* path)
            : id(ID)
            , filePath(path)
        {}
    };



    inline constexpr SoundInformation SOUND_LIST[] = {
        SoundInformation(EnSoundID::BGM, ""),
        SoundInformation(EnSoundID::SE, ""),
        SoundInformation(EnSoundID::Voice, "")
    };



    /**
     * @brief 表の中に、同じidの行がないかを調べる(コンパイル時)
     * @details 同じ番号は登録済みだと何もされないので、重複した行は黙って無視されてしまう。それを防ぐためのもの。
     * @param list 表
     * @return 重複がなければtrue
     */
    template <size_t N>
    constexpr bool IsUniqueSoundID(const SoundInformation (&list)[N])
    {
        for (size_t i = 0; i < N; i++)
        {
            for (size_t j = i + 1; j < N; j++)
            {
                if (list[i].id == list[j].id)
                {
                    return false;
                }
            }
        }
        return true;
    }

    static_assert(IsUniqueSoundID(SOUND_LIST), "SOUND_LIST に、同じidの行がある。");

} // namespace app
