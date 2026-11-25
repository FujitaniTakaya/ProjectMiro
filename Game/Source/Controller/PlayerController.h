/**
 * @file PlayerController.h
 * @brief パッド(キーボード)の入力から、キャラクターへの入力を作るController
 * @details 向きたい方向(Look)には、既定では、スティックを倒していなくても、常にカメラの水平方向の前を設定する。
 *          EnLookMode::Never を選ぶと、構えている間も設定しない。(カメラを自由に回しても、キャラクターの向きが変わらず、構えはその場で行う)
 */
#pragma once
#include <cstdint>

#include "Source/MoveBehavior/IController.h"


namespace app
{
    /**
     * @brief パッドの入力から、キャラクターへの入力を作るController
     */
    class PlayerController : public IController
    {
    public:
        /** 向きたい方向(Look)を、いつ設定するか */
        enum class EnLookMode : uint8_t
        {
            Always, //!< 常にカメラの水平方向の前。インゲームの、カメラを向いたまま動く操作
            Never   //!< 設定しない。キャラクターは、動く時は移動方向を向き、止まっている時は今の向きのまま(構えている間も)
        };


    public:
        /**
         * @brief コンストラクタ
         * @note パッドが繋がっていなくても、キーボードの入力は全てのパッド番号に入る。
         *       実際に繋がっているパッド番号を指定すること。
         * @param padIndex 読み取るパッドの番号(0〜3)
         * @param lookMode 向きたい方向を、いつ設定するか。既定は常に(今までの動き)
         */
        explicit PlayerController(const int padIndex = 0, const EnLookMode lookMode = EnLookMode::Always);


        /**
         * @brief 今フレームの入力を作る
         * @param context 使わない(パッドとカメラから決める)
         * @return 今フレームの入力。パッド番号が範囲外なら無入力
         */
        CharacterInput Update(const ControllerContext& context) override;


    private:
        /** 読み取るパッドの番号 */
        int m_padIndex;
        /** 向きたい方向を、いつ設定するか */
        EnLookMode m_lookMode;
    };
} // namespace app
