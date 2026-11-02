/**
 * @file PlayerController.h
 * @brief パッド(キーボード)の入力から、キャラクターへの入力を作るController
 * @details 向きたい方向(Look)には、スティックを倒していなくても、常にカメラの水平方向の前を設定する。
 */
#pragma once
#include "Source/MoveBehavior/IController.h"


namespace app
{
    /**
     * @brief パッドの入力から、キャラクターへの入力を作るController
     */
    class PlayerController : public IController
    {
    public:
        /**
         * @brief コンストラクタ
         * @note パッドが繋がっていなくても、キーボードの入力は全てのパッド番号に入る。
         *       実際に繋がっているパッド番号を指定すること。
         * @param padIndex 読み取るパッドの番号(0〜3)
         */
        explicit PlayerController(const int padIndex = 0);


        /**
         * @brief 今フレームの入力を作る
         * @param context 使わない(パッドとカメラから決める)
         * @return 今フレームの入力。パッド番号が範囲外なら無入力
         */
        CharacterInput Update(const ControllerContext& context) override;


    private:
        /** 読み取るパッドの番号 */
        int m_padIndex;
    };
} // namespace app
