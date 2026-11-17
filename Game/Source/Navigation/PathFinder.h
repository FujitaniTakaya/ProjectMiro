/**
 * @file PathFinder.h
 * @brief 格子の上の経路探索(A*)
 * @details NavGrid の上で、始点から終点までの経路を探す。
 *          8方向に動き、角を斜めに抜けることはしない。隣のセルとの高さの差が maxStep を超える所(崖)は越えない。
 *          見つけた経路は、真っ直ぐ通れる範囲で間引いて、曲がり角だけのウェイポイントにする。
 *          エンジンに依存しない。
 */
#pragma once
#include <optional>
#include <vector>

#include "NavGrid.h"
#include "NavTypes.h"


namespace app
{
    namespace nav
    {
        /**
         * @brief 経路探索の設定
         */
        struct PathFindSettings
        {
            /** 隣のセルへ上り下りできる高さの差の最大(cm) */
            float maxStep;
            /** 始点・終点が歩けないセルにあるとき、近くの歩けるセルを探す範囲(セル数) */
            int snapRadiusCells;

            /** コンストラクタ。初期値はこのステージ向けの目安 */
            PathFindSettings()
                : maxStep(35.0f)
                , snapRadiusCells(8)
            {
            }
        };


        /**
         * @brief 座標に一番近い、歩けるセルを探す
         * @details 座標のセルが歩けるならそのセル。歩けない(または格子の外)なら、周りに広げて探す。
         *          探す範囲は、座標に最も近いセルから maxRadiusCells セル以内。同じ距離なら先に見つけたものを使う。
         * @param grid 格子
         * @param x ワールドX
         * @param z ワールドZ
         * @param maxRadiusCells 探す範囲(セル数)。0なら座標のセルだけ
         * @return 見つけたセルの番号。見つからなければ std::nullopt
         */
        std::optional<CellCoord> FindNearestWalkableCell(const NavGrid& grid, float x, float z, int maxRadiusCells);


        /**
         * @brief 2つのセルの間を、真っ直ぐ歩いて通れるか
         * @details 2つのセルの中心を結ぶ線が通るセルをすべて調べる。すべて歩けて、続くセルとの高さの差が maxStep 以内なら通れる。
         * @param grid 格子
         * @param fromX 始点セルのX番号
         * @param fromZ 始点セルのZ番号
         * @param toX 終点セルのX番号
         * @param toZ 終点セルのZ番号
         * @param maxStep 隣のセルへ上り下りできる高さの差の最大
         * @return 通れるなら true
         */
        bool HasLineOfSight(const NavGrid& grid, int fromX, int fromZ, int toX, int toZ, float maxStep);


        /**
         * @brief 始点から終点までの経路を探す
         * @details 始点・終点は、歩けるセルに寄せてから探す。
         *          経路の最初の点は始点のセルの中心、最後の点は終点のセルの中心。高さはセルの地面の高さ。
         *          始点と終点が同じセルなら、終点のセルの中心だけの経路になる。
         * @param grid 格子
         * @param start 始点(ワールド座標)。高さは使わない
         * @param goal 終点(ワールド座標)。高さは使わない
         * @param settings 設定
         * @return 経路のウェイポイント。始点・終点を寄せられない・たどり着けないときは std::nullopt
         */
        std::optional<std::vector<NavVec3>> FindPath(const NavGrid& grid, const NavVec3& start, const NavVec3& goal, const PathFindSettings& settings);
    } // namespace nav
} // namespace app
