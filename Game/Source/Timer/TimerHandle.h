/**
 * @file TimerHandle.h
 * @brief タイマーのハンドルの型定義、時間切れになったあとの動作(軽量ヘッダ)
 */
#pragma once
#include <cstdint>


namespace app
{
    /** タイマーのハンドル */
    using TimerHandle = uint32_t;
    /** ハンドル無効値 */
    static constexpr TimerHandle INVALID_TIMER_HANDLE = 0xffffffff;



    /**
     * @brief 時間切れになったあとの動作
     * @details タイマーを作るときに選ぶ。タイマーごとの名前ではなく、あとの扱いの2択。
     */
    enum class EnTimeUpAction : uint8_t
    {
        /** マネージャーに登録されたまま残る。IsTimeUp()で読める。StartTimer()で頭から数え直せる。DestroyTimer()で解除する。 */
        Hold = 0,
        /** 時間切れになると、マネージャーのリストから破棄される。 */
        Release,
    };

} // namespace app
