/**
 * @file NavTypes.h
 * @brief 経路探索で使う基本の型
 * @details エンジンに依存しない。単体でテストできるように、Vector3 ではなく専用の型を使う。
 *          座標はワールド座標(Y-up、単位はcm)。水平面はXZ。
 */
#pragma once


namespace app
{
    namespace nav
    {
        /**
         * @brief 水平(XZ平面)の2次元ベクトル
         */
        struct NavVec2
        {
            /** X成分 */
            float x;
            /** Z成分 */
            float z;
        };


        /**
         * @brief 格子のセルの番号
         */
        struct CellCoord
        {
            /** セルのX番号 */
            int x;
            /** セルのZ番号 */
            int z;
        };


        /**
         * @brief 3次元の座標
         */
        struct NavVec3
        {
            /** X座標 */
            float x;
            /** Y座標(高さ) */
            float y;
            /** Z座標 */
            float z;
        };


        /**
         * @brief 三角形
         * @details 頂点は、表から見て反時計回り((b-a)×(c-a)が表向き)に並べる。
         */
        struct NavTriangle
        {
            /** 頂点A */
            NavVec3 a;
            /** 頂点B */
            NavVec3 b;
            /** 頂点C */
            NavVec3 c;
        };
    } // namespace nav
} // namespace app
