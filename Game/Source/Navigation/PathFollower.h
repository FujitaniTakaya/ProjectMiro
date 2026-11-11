/**
 * @file PathFollower.h
 * @brief 経路をたどる処理
 * @details FindPath() が作ったウェイポイントの列を順にたどるために、いま向かう点を持ち、近づいたら次の点に進める。
 *          経路から外れた・進めなくなったときは、探し直しが必要だと知らせる。探し直すのは使う側。
 *          水平(XZ)の位置だけを見る。エンジンに依存しない。
 */
#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

#include "NavTypes.h"


namespace app
{
    namespace nav
    {
        /**
         * @brief 経路をたどる状況
         */
        enum class EnFollowStatus : uint8_t
        {
            /** 経路が無い */
            NoPath,
            /** 経路をたどっている */
            Following,
            /** 最後のウェイポイントに着いた */
            Finished,
            /** 経路から外れた、または進めなくなった。探し直すこと */
            NeedRepath,

            Max
        };


        /**
         * @brief 経路をたどるときの設定
         */
        struct PathFollowSettings
        {
            /** ウェイポイントに着いたとみなす水平の距離(cm) */
            float reachRadius;
            /** 経路(直前のウェイポイントから、いま向かう点までの線)から外れたとみなす水平の距離(cm) */
            float maxDeviation;
            /** この時間(秒)のあいだ stallDistance 以上動けなければ、進めなくなったとみなす */
            float stallTime;
            /** 進めなくなったかを調べるときに、動いたとみなす水平の距離(cm) */
            float stallDistance;

            /** コンストラクタ。初期値はこのステージ向けの目安 */
            PathFollowSettings()
                : reachRadius(60.0f)
                , maxDeviation(200.0f)
                , stallTime(1.5f)
                , stallDistance(20.0f)
            {
            }
        };


        /**
         * @brief 経路をたどる処理
         */
        class PathFollower
        {
        public:
            /**
             * @brief コンストラクタ。経路を持たない状態で作る
             */
            PathFollower();

            /**
             * @brief 経路を設定する
             * @details 最初のウェイポイントは始点なので、2つ以上あれば2つ目から向かう。たどり状況は最初からやり直す。
             * @param path ウェイポイントの列(FindPath() の結果)。空なら経路なし
             */
            void SetPath(const std::vector<NavVec3>& path);

            /**
             * @brief 経路を捨てる
             */
            void Clear();

            /**
             * @brief 経路を持っているか
             * @return 持っていれば true
             */
            bool HasPath() const;

            /**
             * @brief いま向かっているウェイポイントの番号を取得する
             * @return ウェイポイントの番号。経路が終わっていれば経路の長さ
             */
            size_t GetTargetIndex() const;

            /**
             * @brief 経路を取得する
             * @return ウェイポイントの列
             */
            const std::vector<NavVec3>& GetPath() const;

            /**
             * @brief たどる状況を進める
             * @details 近づいたウェイポイントを済ませて、次の点に進める。続けて、経路から外れていないか・進めなくなっていないかを調べる。
             * @param x 現在のワールドX
             * @param z 現在のワールドZ
             * @param deltaTime 前回からの時間(秒)
             * @param settings 設定
             * @return たどる状況
             */
            EnFollowStatus Update(float x, float z, float deltaTime, const PathFollowSettings& settings);

            /**
             * @brief いま向かっているウェイポイントへの、水平方向の向きを求める
             * @param x 現在のワールドX
             * @param z 現在のワールドZ
             * @param[out] outX 向き(長さ1)のX成分
             * @param[out] outZ 向き(長さ1)のZ成分
             * @return 求められたら true。経路が無い・終わっている・ウェイポイントの真上や真下にいて向きが決まらないときは false(出力は変更しない)
             */
            bool GetMoveDirection(float x, float z, float& outX, float& outZ) const;


        private:
            /** ウェイポイントの列 */
            std::vector<NavVec3> m_path;
            /** いま向かっているウェイポイントの番号 */
            size_t m_targetIndex;
            /** 進めなくなったかを調べる基準の位置があるか */
            bool m_hasStallReference;
            /** 進めなくなったかを調べる基準の位置X */
            float m_stallReferenceX;
            /** 進めなくなったかを調べる基準の位置Z */
            float m_stallReferenceZ;
            /** 基準の位置から動いていない時間(秒) */
            float m_stallElapsed;
        };
    } // namespace nav
} // namespace app
