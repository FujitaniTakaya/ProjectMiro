/**
 * @file IController.h
 * @brief キャラクターの入力を作るControllerのインターフェース
 * @details Controllerは「入力を作る側」。パッドを読む PlayerController も、周りを見て判断する AIController も、
 *          同じ CharacterInput を返す。キャラクターは ControllerSlot 越しにControllerを持ち、返ってきた入力だけを読む。
 *          そのため、Controllerを実行時に付け替えられる。(プレイヤーとAIの入れ替えなど)
 * @note ここでいうControllerは入力の元になるもの。k2EngineLowの物理のカプセル(CharacterController)とは別物。
 */
#pragma once
#include "CharacterInput.h"
#include "ControllerContext.h"


namespace app
{
    /**
     * @brief Controllerのインターフェース
     */
    class IController : public Noncopyable
    {
    public:
        virtual ~IController() = default;


        /**
         * @brief キャラクターに付けられたときに呼ばれる
         * @details 内部の状態を初期化する。同じControllerを別のキャラクターへ付け替えたときも呼ばれる。
         */
        virtual void OnAttach() {}

        /**
         * @brief キャラクターから外されるときに呼ばれる
         */
        virtual void OnDetach() {}

        /**
         * @brief 今フレームの入力を作る
         * @param context 操作対象の情報
         * @return 今フレームの入力。ボタンは押下状態だけ設定すればよい(Triggerは ControllerSlot が計算する)
         */
        virtual CharacterInput Update(const ControllerContext& context) = 0;
    };
}
