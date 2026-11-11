/**
 * @file ZombieSpawnParameter.h
 * @brief ZombieSpawnのパラメーター
 * @details ParamHolderに EnParamID::ZombieSpawn で登録する。jsonは Assets/parameter/actor/ZombieSpawn.json。
 *          例: const ZombieSpawnParameter& param = ParamHolder::Get().GetParameter<ZombieSpawnParameter>(EnParamID::ZombieSpawn);
 */
#pragma once
#include <cstdint>
#include <string>

#include "Source/Core/Json/JsonView.h"


namespace app
{
    namespace actor
    {
        /**
         * @brief ゾンビが出てくるゲート
         * @details ステージの4つのゲート(アーチ)が、そのままスポーン地点になる。
         *          番号(0〜Max-1)でも引ける。ランダムに選ぶときは、Max 未満の乱数を static_cast<EnZombieGate>() して使う。
         */
        enum class EnZombieGate : uint8_t
        {
            /** 北(ワールドの -Z 側。北の高台の上) */
            North,
            /** 南(ワールドの +Z 側。南の高台の上) */
            South,
            /** 西(ワールドの -X 側。低地) */
            West,
            /** 東(ワールドの +X 側。低地) */
            East,

            Max
        };


        /**
         * @brief ZombieSpawnのパラメーター
         * @details ゾンビのスポーン地点(4つのゲート)の座標を持つ。座標は、ゲートの足元の中心のワールド座標(cm)。
         *          値はステージのモデル(stage.tkm)のゲートの位置から求めたもの。モデルを書き出し直したら、jsonも直すこと。
         *          ゾンビを湧かせる設定(順番・間隔・同時数の上限・浮かせる高さ・ずらす半径)も持つ。使うのは ZombieSpawner。
         * @note ゲートはアーチで、通り抜ける開口部は幅500・高さ約420(ゾンビのカプセルは半径25・高さ110)。
         *       北と南のゲートは高台の上(Y=300)にある。出現させるときは、少し浮かせてから落とすこと。
         */
        struct ZombieSpawnParameter
        {
            ZombieSpawnParameter();


            // [PARAM_GEN begin members] 自動生成(tools/param_gen.py)。手で編集しない
            /** 北のゲート(スポーン地点)の足元のワールド座標。Yはゲートが乗っている地面の高さ */
            Vector3 gateNorth;
            /** 南のゲート(スポーン地点)の足元のワールド座標。Yはゲートが乗っている地面の高さ */
            Vector3 gateSouth;
            /** 西のゲート(スポーン地点)の足元のワールド座標。Yはゲートが乗っている地面の高さ */
            Vector3 gateWest;
            /** 東のゲート(スポーン地点)の足元のワールド座標。Yはゲートが乗っている地面の高さ */
            Vector3 gateEast;
            /** 湧く順番。N=通常のゾンビ(歩き)、R=走り続けるゾンビ。左から順に使い、最後まで来たら先頭に戻る。例: NNNR = 3体歩き→1体走りの繰り返し。空や知らない文字はNとして扱う */
            std::string spawnOrder;
            /** 1体湧かせてから、次を湧かせるまでの時間(秒) */
            float spawnInterval;
            /** 同時に生きていられるゾンビの数の上限。上限の間は湧かない */
            int maxAliveCount;
            /** ゲートの足元の座標から、どれだけ浮かせて湧かせるか(cm)。高台のゲートでも落ちて接地する */
            float dropHeight;
            /** ゲートの中心から、湧く位置をXZ平面でずらす半径(cm)。ゲートの開口(幅500)に収まる値にすること */
            float gateJitterRadius;
            // [PARAM_GEN end members]
        };




        /*************************************************************/


        /**
         * @brief ゲートの座標を取得
         * @param param ZombieSpawnのパラメーター
         * @param gate ゲート。Max は渡さないこと
         * @return ゲートの足元の中心のワールド座標
         */
        const Vector3& GetGatePosition(const ZombieSpawnParameter& param, const EnZombieGate gate);

        /**
         * @brief jsonからZombieSpawnのパラメーターを読み込む
         * @details 無いキー・型が違うキーは、パラメーターの初期値のままになる。
         * @param root jsonのルート
         * @param[out] param 読み込み先。デフォルト構築された状態で渡される
         */
        void LoadZombieSpawnParameter(const JsonView& root, ZombieSpawnParameter& param);

        /**
         * @brief ZombieSpawnのパラメーターを、デバッグ画面(ParamDebugUI)に表示する
         * @param param 表示するパラメーター
         */
        void DrawZombieSpawnParameter(const ZombieSpawnParameter& param);
    } // namespace actor
} // namespace app
