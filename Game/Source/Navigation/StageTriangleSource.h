/**
 * @file StageTriangleSource.h
 * @brief ステージのモデルから、経路探索用の三角形を取り出す
 * @details tkmファイルの頂点とインデックスを読んで、ワールド座標の三角形にする。物理エンジンは使わない。
 */
#pragma once
#include <vector>

#include "NavTypes.h"


namespace app
{
    namespace nav
    {
        /**
         * @brief tkmファイルから、ワールド座標の三角形を取り出す
         * @details 全メッシュの全三角形を取り出す。頂点にはワールド行列を掛ける。
         *          Z-upのモデルの向きの補正(Y-upへの回転)は、ワールド行列に含まれている前提。
         *          モデルを描画するモデルの GetWorldMatrix() をそのまま渡せば、描画と同じ位置・向きになる。
         * @param tkmPath tkmファイルのパス
         * @param modelToWorld モデルのワールド行列(Z-upの補正込み)
         * @param[out] outTriangles 三角形の追加先(先に空にしない。追加するだけ)
         * @return 読み込めたら true。読み込めなかったら false(outTrianglesは変更しない)
         */
        bool LoadStageTriangles(const char* tkmPath, const Matrix& modelToWorld, std::vector<NavTriangle>& outTriangles);
    } // namespace nav
} // namespace app
