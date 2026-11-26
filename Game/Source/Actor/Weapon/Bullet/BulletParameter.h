/**
 * @file BulletParameter.h
 * @brief Bulletのパラメーター
 * @details ParamHolderに EnParamID::Bullet で登録する。jsonは Assets/parameter/actor/Bullet.json。
 *          例: const BulletParameter& param = ParamHolder::Get().GetParameter<BulletParameter>(EnParamID::Bullet);
 */
#pragma once
#include "Source/Core/Json/JsonView.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief Bulletのパラメーター
         */
        struct BulletParameter
        {
            BulletParameter();


            // [PARAM_GEN begin members] 自動生成(tools/param_gen.py)。手で編集しない
            /** 弾が消えるまでの時間(秒)。発射してからの経過時間 */
            float lifeTime;
            /** 弾の発射スピード(cm/秒)。InGameScene で撃つ時に使う */
            float shotSpeed;
            /** 弾がゾンビに当たった時のダメージ */
            float damage;
            /** 弾の当たり判定の球の半径(cm)。大きいと、縁をかすっても当たる */
            float hitRadius;
            /** 頭に当たった時のダメージ倍率。damage にこの値を掛けて、ゾンビに与える */
            float headshotMultiplier;
            /** 弾のモデルの拡大率(一様)。1.0で、tkmの大きさのまま */
            float modelScale;
            /** 弾のモデルの向きの補正。[rotX, rotY, rotZ](度)で、飛ぶ向きに対してモデルを回す。モデルが横を向いて見える時に、飛ぶ向きへ合わせる */
            Vector3 modelRotation;
            // [PARAM_GEN end members]
        };




        /*************************************************************/


        /**
         * @brief jsonからBulletのパラメーターを読み込む
         * @details 無いキー・型が違うキーは、パラメーターの初期値のままになる。
         * @param root jsonのルート
         * @param[out] param 読み込み先。デフォルト構築された状態で渡される
         */
        void LoadBulletParameter(const JsonView& root, BulletParameter& param);

        /**
         * @brief Bulletのパラメーターを、デバッグ画面(ParamDebugUI)に表示する
         * @param param 表示するパラメーター
         */
        void DrawBulletParameter(const BulletParameter& param);
    } // namespace actor
} // namespace app
