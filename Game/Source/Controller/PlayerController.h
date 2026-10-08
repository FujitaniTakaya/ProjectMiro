/**
 * @file PlayerController.h
 * @brief パッド(キーボード)の入力から、キャラクターへの入力を作るController
 * @details ProjectBeastのDaddyPenguinControllerの、入力の部分を元にしている。
 *          左スティックをカメラの向きに合わせた移動入力にして、倒し具合で忍び足と走りを切り替える。
 *            左スティック : 移動(カメラ基準)。9割まで倒すと忍び足、それ以上は走り
 *            Bボタン      : 押している間は忍び足
 *            Aボタン      : Jump
 *            Xボタン      : Action1
 *            Yボタン      : Action2
 *          パッドが繋がっていないときは、GamePadのキーボード入力(WASD / J,K,I,L)がそのまま使われる。
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
