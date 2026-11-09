/**
 * @file ZombieParameter.h
 * @brief Zombieのパラメーター
 * @details ParamHolderに EnParamID::Zombie で登録する。jsonは Assets/parameter/actor/Zombie.json。
 *          例: const ZombieParameter& param = ParamHolder::Get().GetParameter<ZombieParameter>(EnParamID::Zombie);
 */
#pragma once
#include "Source/Core/Json/JsonView.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief Zombieのパラメーター
         * @details 基本パラメーター(HP・移動速度・攻撃力)と、アンガーパラメーターを持つ。
         *          ゾンビの種類を増やすときは、同じ構造体で、EnParamID とjsonを足す。
         * @note walkSpeed / runSpeed は速度そのものではなく、プレイヤーの走り速度(MiroParameter::runSpeed)に対する割合。
         *       radius と height は Init() で一度だけ使うので、ホットリロードしても次に起動するまで反映されない。
         *       maxHp はホットリロードで変わる。現在のHPは丸められないので、最大HPを下げると、現在のHPが最大HPを超えることがある。
         */
        struct ZombieParameter
        {
            ZombieParameter();


            // [PARAM_GEN begin members] 自動生成(tools/param_gen.py)。手で編集しない
            /** 最大HP。個体の耐久力で、ダメージを受けきると死亡する */
            float maxHp;
            /** 通常時の移動速度。プレイヤーの走り速度に対する割合 */
            float walkSpeed;
            /** ロックオン時の移動速度。プレイヤーの走り速度に対する割合 */
            float runSpeed;
            /** プレイヤーに与える固定ダメージ */
            float attackPower;
            /** 最初から保持しているアンガー値(種類ごとの固定値) */
            float initialAnger;
            /** あがりやすさ。被弾ダメージをアンガー値へ加算するときの倍率 */
            float angerGainRate;
            /** こらえる閾値(発火ライン)。累積アンガー値がこの値に達すると、本格ロックオンへ移行する */
            float angerThreshold;
            /** ばらまくアンガー値。本格ロックオンに達したとき、周囲のゾンビへ伝播させる値 */
            float angerSpreadValue;
            /** 巻き込む範囲。アンガー値の伝播が届く距離(半径) */
            float angerSpreadRadius;
            /** キャラクターコントローラーの半径 */
            float radius;
            /** キャラクターコントローラーの高さ */
            float height;
            // [PARAM_GEN end members]
        };




        /*************************************************************/


        /**
         * @brief jsonからZombieのパラメーターを読み込む
         * @details 無いキー・型が違うキーは、パラメーターの初期値のままになる。
         * @param root jsonのルート
         * @param[out] param 読み込み先。デフォルト構築された状態で渡される
         */
        void LoadZombieParameter(const JsonView& root, ZombieParameter& param);

        /**
         * @brief Zombieのパラメーターを、デバッグ画面(ParamDebugUI)に表示する
         * @param param 表示するパラメーター
         */
        void DrawZombieParameter(const ZombieParameter& param);
    } // namespace actor
} // namespace app
