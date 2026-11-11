/**
 * @file NavGridBuilder.h
 * @brief 三角形から経路探索用の格子を作る
 * @details ステージの三角形(ワールド座標)を、XZ平面のセルに写して、セルごとの歩ける・歩けないと地面の高さを決める。
 *          エンジンに依存しない。
 * @note このステージ専用の割り切りがある。1セルに1つの高さしか持たないので、頭上が空いている床が複数あるセルでは、一番低い床を使う
 *       (高台の下の床は、高台の底面で頭上が塞がれて除かれ、アーチの上面は、その下の地面より高いので使われない)。
 */
#pragma once
#include <vector>

#include "NavGrid.h"
#include "NavTypes.h"


namespace app
{
    namespace nav
    {
        /**
         * @brief 格子を作るときの設定
         */
        struct NavBuildSettings
        {
            /** セル1辺の長さ(cm) */
            float cellSize;
            /** 歩ける斜面の最大の傾き(度)。これより急な面は床にならない */
            float maxSlopeDegrees;
            /** 隣のセルへ上り下りできる高さの差の最大(cm)。これを超える段差(崖)は越えられない */
            float maxStep;
            /** 歩く者の半径(cm)。歩けない所・崖からこの距離以内のセルは歩けないことにする */
            float agentRadius;
            /** 歩く者の高さ(cm)。頭上にこの高さの空きが無い床は歩けない */
            float agentHeight;

            /** コンストラクタ。初期値はこのステージ向けの目安 */
            NavBuildSettings()
                : cellSize(25.0f)
                , maxSlopeDegrees(45.0f)
                , maxStep(35.0f)
                , agentRadius(25.0f)
                , agentHeight(110.0f)
            {
            }
        };


        /**
         * @brief 三角形から経路探索用の格子を作る
         * @details 格子の範囲は、三角形のXZの範囲にそろえる。
         *          1. 各セルの中心を通る真上からの線で、三角形の面の高さを集める。
         *             傾きが maxSlopeDegrees 以下の上向きの面を床、下向きの面を天井とする。
         *          2. 頭上に agentHeight 以上の空きがある床のうち、一番低いものをセルの床にする。無ければそのセルは歩けない。
         *          3. 隣のセルとの高さの差が maxStep を超える所(崖)を危険とする。
         *          4. 歩けない・危険なセルから agentRadius 以内(格子の外も含む)のセルを、歩けないことにする。
         * @param triangles ステージの三角形(ワールド座標)
         * @param settings 設定
         * @return 作った格子。三角形が空なら空の格子
         */
        NavGrid BuildNavGrid(const std::vector<NavTriangle>& triangles, const NavBuildSettings& settings);
    } // namespace nav
} // namespace app
