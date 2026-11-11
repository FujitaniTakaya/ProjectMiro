/**
 * @file NavigationService.h
 * @brief 経路探索のサービス
 * @details ステージのモデルから経路探索用の格子を1回作って持ち、ゲートからコアまでなどの経路を探して返す。
 *          格子の作り方・経路の探し方の値は、パラメーター(NavigationParameter)から読む。
 * @note このステージ専用。格子は1つだけ持つ。
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
         * @brief 経路探索のサービス
         * @details 起動時の処理は無く、初めて呼ばれたときに作られるので、関数内staticの Get() で取得する。
         */
        class NavigationService : public Noncopyable
        {
        public:
            /**
             * @brief インスタンスを取得する
             * @details 初めて呼んだときに作られ、プロセスの終了時に破棄される。生成・破棄の呼び出しは要らない。
             * @return インスタンス
             */
            static NavigationService& Get();


            /**
             * @brief ステージのモデルから格子を作る
             * @details 作り直すこともできる(前の格子は捨てる)。格子の作り方の値(セルの大きさなど)は、このときのパラメーターを使う。
             * @param tkmPath ステージのtkmファイルのパス
             * @param modelToWorld ステージのワールド行列(Z-upの補正込み)
             * @return 作れたら true。モデルを読めない・三角形が無いときは false(格子は空になる)
             */
            bool Build(const char* tkmPath, const Matrix& modelToWorld);

            /**
             * @brief 格子を捨てる
             */
            void Clear();

            /**
             * @brief 格子ができているか
             * @return できていれば true
             */
            bool IsReady() const;

            /**
             * @brief 始点から終点までの経路を探す
             * @details 探し方の値(段差の上限など)は、パラメーターを使う。高さは使わず、水平(XZ)の位置で探す。
             * @param start 始点(ワールド座標)
             * @param goal 終点(ワールド座標)
             * @param[out] outPath 経路のウェイポイント。見つからなかったときは空
             * @return 見つかったら true。格子ができていない・たどり着けないときは false
             */
            bool FindPath(const Vector3& start, const Vector3& goal, std::vector<NavVec3>& outPath) const;

            /**
             * @brief 格子を取得する
             * @return 格子。できていなければ空
             */
            const NavGrid& GetGrid() const;


        private:
            /** コンストラクタ。Get() からだけ呼ばれる */
            NavigationService();


        private:
            /** 経路探索用の格子 */
            NavGrid m_grid;
        };
    } // namespace nav
} // namespace app
