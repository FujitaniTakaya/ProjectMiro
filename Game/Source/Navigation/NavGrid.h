/**
 * @file NavGrid.h
 * @brief 経路探索用の格子
 * @details XZ平面を正方形のセルに区切り、セルごとに「歩けるか」と「地面の高さ」を持つ。
 *          高さは1セルに1つだけ(橋のような重なった床は扱わない)。
 *          エンジンに依存しない。
 */
#pragma once
#include <cstddef>
#include <vector>


namespace app
{
    namespace nav
    {
        /**
         * @brief 経路探索用の格子
         */
        class NavGrid
        {
        public:
            /**
             * @brief セル
             */
            struct Cell
            {
                /** 歩けるか */
                bool walkable;
                /** 地面の高さ(Y)。歩けないセルでは意味を持たない */
                float height;

                /** コンストラクタ。歩けない・高さ0のセルを作る */
                Cell()
                    : walkable(false)
                    , height(0.0f)
                {
                }

                /**
                 * @brief コンストラクタ
                 * @param isWalkable 歩けるか
                 * @param cellHeight 地面の高さ
                 */
                Cell(const bool isWalkable, const float cellHeight)
                    : walkable(isWalkable)
                    , height(cellHeight)
                {
                }
            };


        public:
            /**
             * @brief コンストラクタ。空の格子を作る
             */
            NavGrid();

            /**
             * @brief コンストラクタ。すべて「歩けない」セルの格子を作る
             * @param originX 格子の最小X(セル(0,0)の端)
             * @param originZ 格子の最小Z(セル(0,0)の端)
             * @param cellSize セル1辺の長さ。0以下のときは1にする
             * @param width X方向のセル数。負のときは0にする
             * @param depth Z方向のセル数。負のときは0にする
             */
            NavGrid(float originX, float originZ, float cellSize, int width, int depth);

            /**
             * @brief セルが1つも無いか
             * @return 空なら true
             */
            bool IsEmpty() const;

            /**
             * @brief X方向のセル数を取得する
             * @return X方向のセル数
             */
            int GetWidth() const;

            /**
             * @brief Z方向のセル数を取得する
             * @return Z方向のセル数
             */
            int GetDepth() const;

            /**
             * @brief セル1辺の長さを取得する
             * @return セル1辺の長さ
             */
            float GetCellSize() const;

            /**
             * @brief 格子の最小Xを取得する
             * @return 格子の最小X
             */
            float GetOriginX() const;

            /**
             * @brief 格子の最小Zを取得する
             * @return 格子の最小Z
             */
            float GetOriginZ() const;

            /**
             * @brief セルの番号が格子の中か
             * @param cellX セルのX番号
             * @param cellZ セルのZ番号
             * @return 中なら true
             */
            bool InBounds(int cellX, int cellZ) const;

            /**
             * @brief セルを取得する
             * @param cellX セルのX番号
             * @param cellZ セルのZ番号
             * @return セル。範囲外のときは「歩けない」セル
             */
            const Cell& GetCell(int cellX, int cellZ) const;

            /**
             * @brief セルを設定する
             * @param cellX セルのX番号
             * @param cellZ セルのZ番号
             * @param cell 設定するセル。範囲外のときは何もしない
             */
            void SetCell(int cellX, int cellZ, const Cell& cell);

            /**
             * @brief セルが歩けるか
             * @param cellX セルのX番号
             * @param cellZ セルのZ番号
             * @return 歩けるなら true。範囲外は false
             */
            bool IsWalkable(int cellX, int cellZ) const;

            /**
             * @brief ワールドのXZ座標を含むセルの番号を求める
             * @param x ワールドX
             * @param z ワールドZ
             * @param[out] outCellX セルのX番号
             * @param[out] outCellZ セルのZ番号
             * @return 格子の中なら true。外なら false(出力は変更しない)
             */
            bool WorldToCell(float x, float z, int& outCellX, int& outCellZ) const;

            /**
             * @brief セルの中心のワールドXを求める
             * @param cellX セルのX番号
             * @return セルの中心のワールドX
             */
            float CellCenterX(int cellX) const;

            /**
             * @brief セルの中心のワールドZを求める
             * @param cellZ セルのZ番号
             * @return セルの中心のワールドZ
             */
            float CellCenterZ(int cellZ) const;


        private:
            /** 格子の最小X */
            float m_originX;
            /** 格子の最小Z */
            float m_originZ;
            /** セル1辺の長さ */
            float m_cellSize;
            /** X方向のセル数 */
            int m_width;
            /** Z方向のセル数 */
            int m_depth;
            /** セル(Zの行ごとにXが並ぶ) */
            std::vector<Cell> m_cells;
        };
    } // namespace nav
} // namespace app
