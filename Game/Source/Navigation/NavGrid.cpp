/**
 * @file NavGrid.cpp
 * @brief 経路探索用の格子
 */
#include "stdafx.h"

#include "NavGrid.h"

#include <algorithm>
#include <cmath>


namespace app
{
    namespace nav
    {
        NavGrid::NavGrid()
            : m_originX(0.0f)
            , m_originZ(0.0f)
            , m_cellSize(1.0f)
            , m_width(0)
            , m_depth(0)
            , m_cells()
        {
        }


        NavGrid::NavGrid(const float originX, const float originZ, const float cellSize, const int width, const int depth)
            : m_originX(originX)
            , m_originZ(originZ)
            , m_cellSize(cellSize > 0.0f ? cellSize : 1.0f)
            , m_width(std::max<int>(width, 0))
            , m_depth(std::max<int>(depth, 0))
            , m_cells(static_cast<size_t>(std::max<int>(width, 0)) * static_cast<size_t>(std::max<int>(depth, 0)))
        {
        }


        bool NavGrid::IsEmpty() const
        {
            return m_cells.empty();
        }


        int NavGrid::GetWidth() const
        {
            return m_width;
        }


        int NavGrid::GetDepth() const
        {
            return m_depth;
        }


        float NavGrid::GetCellSize() const
        {
            return m_cellSize;
        }


        float NavGrid::GetOriginX() const
        {
            return m_originX;
        }


        float NavGrid::GetOriginZ() const
        {
            return m_originZ;
        }


        bool NavGrid::InBounds(const int cellX, const int cellZ) const
        {
            return cellX >= 0 && cellX < m_width && cellZ >= 0 && cellZ < m_depth;
        }


        const NavGrid::Cell& NavGrid::GetCell(const int cellX, const int cellZ) const
        {
            static const Cell OUT_OF_BOUNDS_CELL;
            if (!InBounds(cellX, cellZ))
            {
                return OUT_OF_BOUNDS_CELL;
            }
            return m_cells[static_cast<size_t>(cellZ) * static_cast<size_t>(m_width) + static_cast<size_t>(cellX)];
        }


        void NavGrid::SetCell(const int cellX, const int cellZ, const Cell& cell)
        {
            if (!InBounds(cellX, cellZ))
            {
                return;
            }
            m_cells[static_cast<size_t>(cellZ) * static_cast<size_t>(m_width) + static_cast<size_t>(cellX)] = cell;
        }


        bool NavGrid::IsWalkable(const int cellX, const int cellZ) const
        {
            return GetCell(cellX, cellZ).walkable;
        }


        bool NavGrid::WorldToCell(const float x, const float z, int& outCellX, int& outCellZ) const
        {
            const int cellX = static_cast<int>(std::floor((x - m_originX) / m_cellSize));
            const int cellZ = static_cast<int>(std::floor((z - m_originZ) / m_cellSize));
            if (!InBounds(cellX, cellZ))
            {
                return false;
            }
            outCellX = cellX;
            outCellZ = cellZ;
            return true;
        }


        float NavGrid::CellCenterX(const int cellX) const
        {
            return m_originX + (static_cast<float>(cellX) + 0.5f) * m_cellSize;
        }


        float NavGrid::CellCenterZ(const int cellZ) const
        {
            return m_originZ + (static_cast<float>(cellZ) + 0.5f) * m_cellSize;
        }
    } // namespace nav
} // namespace app
