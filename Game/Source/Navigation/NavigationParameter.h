/**
 * @file NavigationParameter.h
 * @brief Navigationのパラメーター
 * @details ParamHolderに EnParamID::Navigation で登録する。jsonは Assets/parameter/navigation/Navigation.json。
 *          例: const NavigationParameter& param = ParamHolder::Get().GetParameter<NavigationParameter>(EnParamID::Navigation);
 */
#pragma once
#include "Source/Core/Json/JsonView.h"


namespace app
{
    namespace nav
    {
        /**
         * @brief Navigationのパラメーター
         */
        struct NavigationParameter
        {
            NavigationParameter();


            // [PARAM_GEN begin members] 自動生成(tools/param_gen.py)。手で編集しない
            /** 格子のセル1辺の長さ(cm)。小さいほど細かいが、重くなる。格子を作るとき(NavigationService::Build)に1回だけ使う */
            float cellSize;
            /** 歩ける斜面の最大の傾き(度)。これより急な面は床にならない。格子を作るときだけ使う */
            float maxSlopeDegrees;
            /** 隣のセルへ上り下りできる高さの差の最大(cm)。これを超える段差(崖)は越えない。格子を作るときと、経路を探すときに使う */
            float maxStep;
            /** 歩く者の半径(cm)。歩けない所・崖からこの距離以内のセルは通らない。格子を作るときだけ使う */
            float agentRadius;
            /** 歩く者の高さ(cm)。頭上にこの高さの空きが無い床は歩けない。格子を作るときだけ使う */
            float agentHeight;
            /** 始点・終点が歩けないセルにあるとき、近くの歩けるセルを探す範囲(セル数) */
            int snapRadiusCells;
            /** ウェイポイントに着いたとみなす水平の距離(cm) */
            float reachRadius;
            /** 経路(直前のウェイポイントから、いま向かう点までの線)から外れたとみなす水平の距離(cm)。外れたら探し直す */
            float maxDeviation;
            /** この時間(秒)のあいだ stallDistance 以上動けなければ、進めなくなったとみなして探し直す */
            float stallTime;
            /** 進めなくなったかを調べるときに、動いたとみなす水平の距離(cm) */
            float stallDistance;
            /** 探し直すのを試す間隔の最小(秒)。探し直しに失敗したときに、毎フレーム探さないため */
            float repathInterval;
            // [PARAM_GEN end members]
        };




        /*************************************************************/


        /**
         * @brief jsonからNavigationのパラメーターを読み込む
         * @details 無いキー・型が違うキーは、パラメーターの初期値のままになる。
         * @param root jsonのルート
         * @param[out] param 読み込み先。デフォルト構築された状態で渡される
         */
        void LoadNavigationParameter(const JsonView& root, NavigationParameter& param);

        /**
         * @brief Navigationのパラメーターを、デバッグ画面(ParamDebugUI)に表示する
         * @param param 表示するパラメーター
         */
        void DrawNavigationParameter(const NavigationParameter& param);
    } // namespace nav
} // namespace app
