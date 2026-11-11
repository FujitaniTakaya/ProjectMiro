/**
 * @file PathFinder.cpp
 * @brief 格子の上の経路探索(A*)
 */
#include "stdafx.h"

#include "PathFinder.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <queue>
#include <utility>


namespace app
{
    namespace nav
    {
        namespace
        {
            /** 斜め移動の距離(セル1辺を1とする) */
            constexpr float SQRT2 = 1.41421356f;
            /** 高さの差を比べるときの許容(cm)。浮動小数点の誤差で境目が揺れないようにする */
            constexpr float STEP_EPSILON = 1.0e-3f;


            /**
             * @brief 縦か横に隣り合う2つのセルの間を、歩いて移れるか
             * @param grid 格子
             * @param fromX 移動元のX番号
             * @param fromZ 移動元のZ番号
             * @param toX 移動先のX番号
             * @param toZ 移動先のZ番号
             * @param maxStep 上り下りできる高さの差の最大
             * @return 移れるなら true
             */
            bool CanStepStraight(const NavGrid& grid, const int fromX, const int fromZ, const int toX, const int toZ, const float maxStep)
            {
                if (!grid.IsWalkable(fromX, fromZ) || !grid.IsWalkable(toX, toZ))
                {
                    return false;
                }
                return std::fabs(grid.GetCell(toX, toZ).height - grid.GetCell(fromX, fromZ).height) <= maxStep + STEP_EPSILON;
            }


            /**
             * @brief 隣り合う(斜めも含む)2つのセルの間を、歩いて移れるか
             * @details 斜めに移るときは、角を抜けないように、間の2つのセルを通る4つの縦横の移動がすべてできることを求める。
             */
            bool CanStep(const NavGrid& grid, const int fromX, const int fromZ, const int toX, const int toZ, const float maxStep)
            {
                if (fromX == toX || fromZ == toZ)
                {
                    return CanStepStraight(grid, fromX, fromZ, toX, toZ, maxStep);
                }

                return CanStepStraight(grid, fromX, fromZ, toX, fromZ, maxStep) && CanStepStraight(grid, toX, fromZ, toX, toZ, maxStep) &&
                       CanStepStraight(grid, fromX, fromZ, fromX, toZ, maxStep) && CanStepStraight(grid, fromX, toZ, toX, toZ, maxStep);
            }


            /**
             * @brief A*の待ち行列に入れる点
             */
            struct OpenNode
            {
                /** 推定の総コスト(これまでのコスト+終点までの推定) */
                float totalCost;
                /** セルの通し番号 */
                int index;
            };


            /**
             * @brief 総コストが小さい点を先に取り出すための比較
             */
            struct OpenNodeGreater
            {
                bool operator()(const OpenNode& lhs, const OpenNode& rhs) const
                {
                    return lhs.totalCost > rhs.totalCost;
                }
            };
        } // namespace


        std::optional<CellCoord> FindNearestWalkableCell(const NavGrid& grid, const float x, const float z, const int maxRadiusCells)
        {
            if (grid.IsEmpty())
            {
                return std::nullopt;
            }

            // 座標が格子の外でも、一番近い端のセルから探す
            const float cellSize = grid.GetCellSize();
            const int baseX = std::clamp(static_cast<int>(std::floor((x - grid.GetOriginX()) / cellSize)), 0, grid.GetWidth() - 1);
            const int baseZ = std::clamp(static_cast<int>(std::floor((z - grid.GetOriginZ()) / cellSize)), 0, grid.GetDepth() - 1);

            // 座標のセルが歩けるなら、それを使う(セルの境目では、隣の中心の方が近いことがあるため)
            const std::optional<CellCoord> inside = grid.WorldToCell(x, z);
            if (inside && grid.IsWalkable(inside->x, inside->z))
            {
                return inside;
            }

            // 座標からの距離が一番小さい歩けるセルを選ぶ
            std::optional<CellCoord> best;
            float bestDistanceSq = (std::numeric_limits<float>::max)();
            const int radius = std::max<int>(maxRadiusCells, 0);
            for (int offsetZ = -radius; offsetZ <= radius; ++offsetZ)
            {
                for (int offsetX = -radius; offsetX <= radius; ++offsetX)
                {
                    const int cellX = baseX + offsetX;
                    const int cellZ = baseZ + offsetZ;
                    if (!grid.IsWalkable(cellX, cellZ))
                    {
                        continue;
                    }

                    const float dx = grid.CellCenterX(cellX) - x;
                    const float dz = grid.CellCenterZ(cellZ) - z;
                    const float distanceSq = dx * dx + dz * dz;
                    if (distanceSq < bestDistanceSq)
                    {
                        bestDistanceSq = distanceSq;
                        best = CellCoord{ cellX, cellZ };
                    }
                }
            }
            return best;
        }


        bool HasLineOfSight(const NavGrid& grid, const int fromX, const int fromZ, const int toX, const int toZ, const float maxStep)
        {
            // 2つのセルの中心を結ぶ線が通るセルを、縦か横に1つずつたどる(斜めに角を抜けない)
            const int deltaX = std::abs(toX - fromX);
            const int deltaZ = std::abs(toZ - fromZ);
            const int stepX = toX > fromX ? 1 : -1;
            const int stepZ = toZ > fromZ ? 1 : -1;

            int cellX = fromX;
            int cellZ = fromZ;
            if (!grid.IsWalkable(cellX, cellZ))
            {
                return false;
            }

            int error = deltaX - deltaZ;
            for (int remaining = deltaX + deltaZ; remaining > 0; --remaining)
            {
                int nextX = cellX;
                int nextZ = cellZ;
                if (error > 0)
                {
                    nextX += stepX;
                    error -= 2 * deltaZ;
                }
                else
                {
                    nextZ += stepZ;
                    error += 2 * deltaX;
                }

                if (!CanStepStraight(grid, cellX, cellZ, nextX, nextZ, maxStep))
                {
                    return false;
                }
                cellX = nextX;
                cellZ = nextZ;
            }
            return true;
        }


        std::optional<std::vector<NavVec3>> FindPath(const NavGrid& grid, const NavVec3& start, const NavVec3& goal, const PathFindSettings& settings)
        {
            const std::optional<CellCoord> startCell = FindNearestWalkableCell(grid, start.x, start.z, settings.snapRadiusCells);
            const std::optional<CellCoord> goalCell = FindNearestWalkableCell(grid, goal.x, goal.z, settings.snapRadiusCells);
            if (!startCell || !goalCell)
            {
                return std::nullopt;
            }
            const int startX = startCell->x;
            const int startZ = startCell->z;
            const int goalX = goalCell->x;
            const int goalZ = goalCell->z;

            const int width = grid.GetWidth();
            const size_t cellCount = static_cast<size_t>(width) * static_cast<size_t>(grid.GetDepth());
            const float cellSize = grid.GetCellSize();
            const auto toIndex = [width](const int cellX, const int cellZ) { return cellZ * width + cellX; };

            // 終点までの推定コスト(斜めも動けるときの最短距離)
            const auto estimate = [&](const int cellX, const int cellZ) {
                const float dx = static_cast<float>(std::abs(cellX - goalX));
                const float dz = static_cast<float>(std::abs(cellZ - goalZ));
                return (std::max<float>(dx, dz) + (SQRT2 - 1.0f) * std::min<float>(dx, dz)) * cellSize;
            };

            std::vector<float> cost(cellCount, (std::numeric_limits<float>::max)());
            std::vector<int> parent(cellCount, -1);
            std::vector<uint8_t> closed(cellCount, 0);
            std::priority_queue<OpenNode, std::vector<OpenNode>, OpenNodeGreater> open;

            const int startIndex = toIndex(startX, startZ);
            const int goalIndex = toIndex(goalX, goalZ);
            cost[startIndex] = 0.0f;
            open.push({ estimate(startX, startZ), startIndex });

            while (!open.empty())
            {
                const int currentIndex = open.top().index;
                open.pop();
                if (closed[currentIndex] != 0)
                {
                    continue;
                }
                closed[currentIndex] = 1;
                if (currentIndex == goalIndex)
                {
                    break;
                }

                const int currentX = currentIndex % width;
                const int currentZ = currentIndex / width;
                for (int offsetZ = -1; offsetZ <= 1; ++offsetZ)
                {
                    for (int offsetX = -1; offsetX <= 1; ++offsetX)
                    {
                        if (offsetX == 0 && offsetZ == 0)
                        {
                            continue;
                        }

                        const int nextX = currentX + offsetX;
                        const int nextZ = currentZ + offsetZ;
                        if (!grid.InBounds(nextX, nextZ) || closed[toIndex(nextX, nextZ)] != 0 || !CanStep(grid, currentX, currentZ, nextX, nextZ, settings.maxStep))
                        {
                            continue;
                        }

                        const float stepCost = (offsetX != 0 && offsetZ != 0 ? SQRT2 : 1.0f) * cellSize;
                        const float nextCost = cost[currentIndex] + stepCost;
                        const int nextIndex = toIndex(nextX, nextZ);
                        if (nextCost < cost[nextIndex])
                        {
                            cost[nextIndex] = nextCost;
                            parent[nextIndex] = currentIndex;
                            open.push({ nextCost + estimate(nextX, nextZ), nextIndex });
                        }
                    }
                }
            }

            if (closed[goalIndex] == 0)
            {
                return std::nullopt;
            }

            // 終点から始点へたどって、始点から終点の順に並べ直す
            std::vector<std::pair<int, int>> chain;
            for (int index = goalIndex; index != -1; index = parent[index])
            {
                chain.emplace_back(index % width, index / width);
            }
            std::reverse(chain.begin(), chain.end());

            // 真っ直ぐ通れる範囲で間引いて、曲がり角だけを残す
            std::vector<NavVec3> path;
            const auto addWaypoint = [&](const std::pair<int, int>& cell) { path.push_back({ grid.CellCenterX(cell.first), grid.GetCell(cell.first, cell.second).height, grid.CellCenterZ(cell.second) }); };
            addWaypoint(chain.front());
            size_t anchor = 0;
            while (anchor + 1 < chain.size())
            {
                size_t farthest = chain.size() - 1;
                while (farthest > anchor + 1 && !HasLineOfSight(grid, chain[anchor].first, chain[anchor].second, chain[farthest].first, chain[farthest].second, settings.maxStep))
                {
                    --farthest;
                }
                addWaypoint(chain[farthest]);
                anchor = farthest;
            }
            return path;
        }
    } // namespace nav
} // namespace app
