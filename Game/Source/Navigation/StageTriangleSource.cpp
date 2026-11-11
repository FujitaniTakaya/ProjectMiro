/**
 * @file StageTriangleSource.cpp
 * @brief ステージのモデルから、経路探索用の三角形を取り出す
 */
#include "stdafx.h"

#include "StageTriangleSource.h"


namespace app
{
    namespace nav
    {
        namespace
        {
            /**
             * @brief 頂点にワールド行列を掛けて、経路探索の座標にする
             * @param position モデルの頂点座標
             * @param modelToWorld モデルのワールド行列
             * @return ワールド座標
             */
            NavVec3 ToWorld(const Vector3& position, const Matrix& modelToWorld)
            {
                Vector3 world(position);
                modelToWorld.Apply(world);
                return NavVec3{ world.x, world.y, world.z };
            }


            /**
             * @brief インデックスバッファの三角形を、すべて追加する
             * @tparam TIndexBuffer インデックスバッファの型(indices を持つ)
             * @param mesh メッシュ
             * @param indexBuffer インデックスバッファ(3つで1つの三角形)
             * @param modelToWorld モデルのワールド行列
             * @param[in,out] triangles 追加先
             */
            template <typename TIndexBuffer>
            void AppendTriangles(const TkmFile::SMesh& mesh, const TIndexBuffer& indexBuffer, const Matrix& modelToWorld, std::vector<NavTriangle>& triangles)
            {
                const size_t vertexCount = mesh.vertexBuffer.size();
                const size_t triangleCount = indexBuffer.indices.size() / 3;
                for (size_t i = 0; i < triangleCount; ++i)
                {
                    const size_t indexA = indexBuffer.indices[i * 3];
                    const size_t indexB = indexBuffer.indices[i * 3 + 1];
                    const size_t indexC = indexBuffer.indices[i * 3 + 2];
                    if (indexA >= vertexCount || indexB >= vertexCount || indexC >= vertexCount)
                    {
                        continue;
                    }
                    triangles.push_back(NavTriangle{
                        ToWorld(mesh.vertexBuffer[indexA].pos, modelToWorld),
                        ToWorld(mesh.vertexBuffer[indexB].pos, modelToWorld),
                        ToWorld(mesh.vertexBuffer[indexC].pos, modelToWorld),
                    });
                }
            }
        } // namespace


        bool LoadStageTriangles(const char* tkmPath, const Matrix& modelToWorld, std::vector<NavTriangle>& outTriangles)
        {
            // 形だけ欲しいので、最適化とテクスチャの読み込みはしない
            TkmFile tkmFile;
            if (!tkmFile.Load(tkmPath, false, false, true))
            {
                return false;
            }

            for (const TkmFile::SMesh& mesh : tkmFile.GetMeshParts())
            {
                for (const auto& indexBuffer : mesh.indexBuffer16Array)
                {
                    AppendTriangles(mesh, indexBuffer, modelToWorld, outTriangles);
                }
                for (const auto& indexBuffer : mesh.indexBuffer32Array)
                {
                    AppendTriangles(mesh, indexBuffer, modelToWorld, outTriangles);
                }
            }
            return true;
        }
    } // namespace nav
} // namespace app
