/**
 * @file NavGridBuilder.cpp
 * @brief 三角形から経路探索用の格子を作る
 */
#include "stdafx.h"

#include "NavGridBuilder.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>


namespace app
{
    namespace nav
    {
        namespace
        {
            /** 度からラジアンへの変換係数 */
            constexpr float DEGREES_TO_RADIANS = 3.14159265358979f / 180.0f;
            /** 面の法線のYがこの値より小さい(下向きに十分傾いている)面を天井とみなす境目 */
            constexpr float CEILING_NORMAL_Y = -1.0e-3f;
            /** 三角形のXZ平面での面積(の2倍)がこれ以下なら、真上から見て面が無いものとして飛ばす */
            constexpr float MIN_PROJECTED_AREA = 1.0e-6f;
            /** 三角形の中に点があるかを調べるときの許容(辺の上のセルを取りこぼさないため) */
            constexpr float INSIDE_TOLERANCE = 1.0e-5f;
            /** 床と天井の高さが同じとみなす許容(cm) */
            constexpr float HEIGHT_EPSILON = 0.5f;
            /** 隣のセルの高さの差を比べるときの許容(cm)。浮動小数点の誤差で境目が揺れないようにする */
            constexpr float STEP_EPSILON = 1.0e-3f;


            /**
             * @brief セルごとの、真上からの線に当たった床と天井の高さ
             */
            struct CellSurfaces
            {
                /** 床の高さ */
                std::vector<float> floors;
                /** 天井の高さ */
                std::vector<float> ceilings;
            };


            /**
             * @brief 三角形を1枚、セルに写して床・天井の高さを集める
             * @param triangle 三角形
             * @param minNormalY 床とみなす面の法線Yの最小値(傾きの上限から求めた値)
             * @param grid 範囲を決めるための格子(セルの中身は使わない)
             * @param[in,out] surfaces セルごとの床・天井の高さ(Zの行ごとにXが並ぶ)
             */
            void RasterizeTriangle(const NavTriangle& triangle, const float minNormalY, const NavGrid& grid, std::vector<CellSurfaces>& surfaces)
            {
                const NavVec3& a = triangle.a;
                const float ux = triangle.b.x - a.x;
                const float uy = triangle.b.y - a.y;
                const float uz = triangle.b.z - a.z;
                const float vx = triangle.c.x - a.x;
                const float vy = triangle.c.y - a.y;
                const float vz = triangle.c.z - a.z;

                // 法線(外積)。Yの成分だけで、床か天井かを決める
                const float normalX = uy * vz - uz * vy;
                const float normalY = uz * vx - ux * vz;
                const float normalZ = ux * vy - uy * vx;
                const float length = std::sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ);
                if (length <= MIN_PROJECTED_AREA)
                {
                    return;
                }
                const float unitNormalY = normalY / length;
                const bool isFloor = unitNormalY >= minNormalY;
                const bool isCeiling = unitNormalY < CEILING_NORMAL_Y;
                if (!isFloor && !isCeiling)
                {
                    return;
                }

                // 真上から見た面積(の2倍)。ほぼ0なら垂直な面なので飛ばす
                const float area = ux * vz - uz * vx;
                if (std::fabs(area) <= MIN_PROJECTED_AREA)
                {
                    return;
                }

                // 三角形が重なるセルの範囲
                const float cellSize = grid.GetCellSize();
                const float minX = std::min<float>({ a.x, triangle.b.x, triangle.c.x });
                const float maxX = std::max<float>({ a.x, triangle.b.x, triangle.c.x });
                const float minZ = std::min<float>({ a.z, triangle.b.z, triangle.c.z });
                const float maxZ = std::max<float>({ a.z, triangle.b.z, triangle.c.z });
                const int cellX0 = std::max<int>(0, static_cast<int>(std::floor((minX - grid.GetOriginX()) / cellSize)));
                const int cellX1 = std::min<int>(grid.GetWidth() - 1, static_cast<int>(std::floor((maxX - grid.GetOriginX()) / cellSize)));
                const int cellZ0 = std::max<int>(0, static_cast<int>(std::floor((minZ - grid.GetOriginZ()) / cellSize)));
                const int cellZ1 = std::min<int>(grid.GetDepth() - 1, static_cast<int>(std::floor((maxZ - grid.GetOriginZ()) / cellSize)));

                for (int cellZ = cellZ0; cellZ <= cellZ1; ++cellZ)
                {
                    const float pointZ = grid.CellCenterZ(cellZ);
                    for (int cellX = cellX0; cellX <= cellX1; ++cellX)
                    {
                        const float pointX = grid.CellCenterX(cellX);

                        // P - A = s*u + t*v を解く(XZ平面)
                        const float dx = pointX - a.x;
                        const float dz = pointZ - a.z;
                        const float s = (dx * vz - vx * dz) / area;
                        const float t = (ux * dz - uz * dx) / area;
                        if (s < -INSIDE_TOLERANCE || t < -INSIDE_TOLERANCE || s + t > 1.0f + INSIDE_TOLERANCE)
                        {
                            continue;
                        }

                        const float height = a.y + s * uy + t * vy;
                        CellSurfaces& cell = surfaces[static_cast<size_t>(cellZ) * static_cast<size_t>(grid.GetWidth()) + static_cast<size_t>(cellX)];
                        if (isFloor)
                        {
                            cell.floors.push_back(height);
                        }
                        else
                        {
                            cell.ceilings.push_back(height);
                        }
                    }
                }
            }


            /**
             * @brief セルの床を決める
             * @details 頭上に agentHeight 以上の空きがある床のうち、一番低いものを選ぶ。
             * @param surfaces セルの床・天井の高さ
             * @param agentHeight 歩く者の高さ
             * @param[out] outHeight 選んだ床の高さ
             * @return 床があれば true
             */
            bool PickFloor(const CellSurfaces& surfaces, const float agentHeight, float& outHeight)
            {
                bool found = false;
                float lowest = (std::numeric_limits<float>::max)();
                for (const float floorHeight : surfaces.floors)
                {
                    if (found && floorHeight >= lowest)
                    {
                        continue;
                    }

                    // 床の上にある天井のうち、一番近いものまでの空き
                    float clearance = (std::numeric_limits<float>::max)();
                    for (const float ceilingHeight : surfaces.ceilings)
                    {
                        if (ceilingHeight >= floorHeight - HEIGHT_EPSILON)
                        {
                            clearance = std::min<float>(clearance, std::max<float>(ceilingHeight - floorHeight, 0.0f));
                        }
                    }

                    if (clearance >= agentHeight)
                    {
                        lowest = floorHeight;
                        found = true;
                    }
                }

                if (found)
                {
                    outHeight = lowest;
                }
                return found;
            }
        } // namespace


        NavGrid BuildNavGrid(const std::vector<NavTriangle>& triangles, const NavBuildSettings& settings)
        {
            if (triangles.empty())
            {
                return NavGrid();
            }

            // 格子の範囲は、三角形のXZの範囲
            float minX = (std::numeric_limits<float>::max)();
            float maxX = std::numeric_limits<float>::lowest();
            float minZ = (std::numeric_limits<float>::max)();
            float maxZ = std::numeric_limits<float>::lowest();
            for (const NavTriangle& triangle : triangles)
            {
                for (const NavVec3* vertex : { &triangle.a, &triangle.b, &triangle.c })
                {
                    minX = std::min<float>(minX, vertex->x);
                    maxX = std::max<float>(maxX, vertex->x);
                    minZ = std::min<float>(minZ, vertex->z);
                    maxZ = std::max<float>(maxZ, vertex->z);
                }
            }

            const float cellSize = std::max<float>(settings.cellSize, 1.0e-3f);
            const int width = std::max<int>(1, static_cast<int>(std::ceil((maxX - minX) / cellSize)));
            const int depth = std::max<int>(1, static_cast<int>(std::ceil((maxZ - minZ) / cellSize)));
            NavGrid grid(minX, minZ, cellSize, width, depth);

            // 1. 床・天井の高さを集める
            const float minNormalY = std::cos(settings.maxSlopeDegrees * DEGREES_TO_RADIANS);
            std::vector<CellSurfaces> surfaces(static_cast<size_t>(width) * static_cast<size_t>(depth));
            for (const NavTriangle& triangle : triangles)
            {
                RasterizeTriangle(triangle, minNormalY, grid, surfaces);
            }

            // 2. セルの床を決める
            std::vector<uint8_t> hasFloor(surfaces.size(), 0);
            for (int cellZ = 0; cellZ < depth; ++cellZ)
            {
                for (int cellX = 0; cellX < width; ++cellX)
                {
                    float height = 0.0f;
                    const size_t index = static_cast<size_t>(cellZ) * static_cast<size_t>(width) + static_cast<size_t>(cellX);
                    if (PickFloor(surfaces[index], settings.agentHeight, height))
                    {
                        hasFloor[index] = 1;
                        grid.SetCell(cellX, cellZ, NavGrid::Cell(true, height));
                    }
                }
            }

            // 3. 崖(隣のセルとの高さの差が大きい所)を危険にする。段差の両側を危険とする
            std::vector<uint8_t> isBad(surfaces.size(), 0);
            for (int cellZ = 0; cellZ < depth; ++cellZ)
            {
                for (int cellX = 0; cellX < width; ++cellX)
                {
                    const size_t index = static_cast<size_t>(cellZ) * static_cast<size_t>(width) + static_cast<size_t>(cellX);
                    if (hasFloor[index] == 0)
                    {
                        isBad[index] = 1;
                        continue;
                    }

                    for (int offsetZ = -1; offsetZ <= 1; ++offsetZ)
                    {
                        for (int offsetX = -1; offsetX <= 1; ++offsetX)
                        {
                            const int neighborX = cellX + offsetX;
                            const int neighborZ = cellZ + offsetZ;
                            if ((offsetX == 0 && offsetZ == 0) || !grid.InBounds(neighborX, neighborZ) || !grid.IsWalkable(neighborX, neighborZ))
                            {
                                continue;
                            }
                            const float heightDifference = std::fabs(grid.GetCell(neighborX, neighborZ).height - grid.GetCell(cellX, cellZ).height);
                            if (heightDifference > settings.maxStep + STEP_EPSILON)
                            {
                                isBad[index] = 1;
                            }
                        }
                    }
                }
            }

            // 4. 歩けない・危険なセルから agentRadius 以内(格子の外を含む)を歩けないことにする
            const int radiusCells = static_cast<int>(std::ceil(settings.agentRadius / cellSize));
            std::vector<std::pair<int, int>> discOffsets;
            for (int offsetZ = -radiusCells; offsetZ <= radiusCells; ++offsetZ)
            {
                for (int offsetX = -radiusCells; offsetX <= radiusCells; ++offsetX)
                {
                    const float distance = std::sqrt(static_cast<float>(offsetX * offsetX + offsetZ * offsetZ)) * cellSize;
                    if (distance <= settings.agentRadius + STEP_EPSILON)
                    {
                        discOffsets.emplace_back(offsetX, offsetZ);
                    }
                }
            }

            NavGrid result(minX, minZ, cellSize, width, depth);
            for (int cellZ = 0; cellZ < depth; ++cellZ)
            {
                for (int cellX = 0; cellX < width; ++cellX)
                {
                    const size_t index = static_cast<size_t>(cellZ) * static_cast<size_t>(width) + static_cast<size_t>(cellX);
                    if (hasFloor[index] == 0)
                    {
                        continue;
                    }

                    bool blocked = false;
                    for (const auto& offset : discOffsets)
                    {
                        const int neighborX = cellX + offset.first;
                        const int neighborZ = cellZ + offset.second;
                        if (!grid.InBounds(neighborX, neighborZ) || isBad[static_cast<size_t>(neighborZ) * static_cast<size_t>(width) + static_cast<size_t>(neighborX)] != 0)
                        {
                            blocked = true;
                            break;
                        }
                    }
                    result.SetCell(cellX, cellZ, NavGrid::Cell(!blocked, grid.GetCell(cellX, cellZ).height));
                }
            }
            return result;
        }
    } // namespace nav
} // namespace app
