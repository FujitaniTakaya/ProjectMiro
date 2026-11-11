/**
 * @file PathFollower.cpp
 * @brief 経路をたどる処理
 */
#include "stdafx.h"

#include "PathFollower.h"

#include <algorithm>
#include <cmath>


namespace app
{
    namespace nav
    {
        namespace
        {
            /** 向きを正規化してよい長さの二乗のしきい値(ゼロ除算防止) */
            constexpr float DIRECTION_NORMALIZE_SQ = 0.001f;


            /**
             * @brief 点から線分までの水平の距離を求める
             * @param pointX 点のX
             * @param pointZ 点のZ
             * @param from 線分の始まり
             * @param to 線分の終わり
             * @return 点から線分までの距離
             */
            float DistanceToSegment(const float pointX, const float pointZ, const NavVec3& from, const NavVec3& to)
            {
                const float segmentX = to.x - from.x;
                const float segmentZ = to.z - from.z;
                const float lengthSq = segmentX * segmentX + segmentZ * segmentZ;

                // 線分が点なら、その点までの距離
                float ratio = 0.0f;
                if (lengthSq > DIRECTION_NORMALIZE_SQ)
                {
                    ratio = std::clamp(((pointX - from.x) * segmentX + (pointZ - from.z) * segmentZ) / lengthSq, 0.0f, 1.0f);
                }

                const float closestX = from.x + segmentX * ratio;
                const float closestZ = from.z + segmentZ * ratio;
                return std::hypot(pointX - closestX, pointZ - closestZ);
            }
        } // namespace


        PathFollower::PathFollower()
            : m_path()
            , m_targetIndex(0)
            , m_hasStallReference(false)
            , m_stallReferenceX(0.0f)
            , m_stallReferenceZ(0.0f)
            , m_stallElapsed(0.0f)
        {
        }


        void PathFollower::SetPath(const std::vector<NavVec3>& path)
        {
            m_path = path;
            m_targetIndex = m_path.size() > 1 ? 1 : 0;
            m_hasStallReference = false;
            m_stallElapsed = 0.0f;
        }


        void PathFollower::Clear()
        {
            SetPath(std::vector<NavVec3>());
        }


        bool PathFollower::HasPath() const
        {
            return !m_path.empty();
        }


        size_t PathFollower::GetTargetIndex() const
        {
            return m_targetIndex;
        }


        const std::vector<NavVec3>& PathFollower::GetPath() const
        {
            return m_path;
        }


        EnFollowStatus PathFollower::Update(const float x, const float z, const float deltaTime, const PathFollowSettings& settings)
        {
            if (m_path.empty())
            {
                return EnFollowStatus::NoPath;
            }

            // 近づいたウェイポイントは済ませて、次へ進める
            bool advanced = false;
            while (m_targetIndex < m_path.size() && std::hypot(m_path[m_targetIndex].x - x, m_path[m_targetIndex].z - z) <= settings.reachRadius)
            {
                ++m_targetIndex;
                advanced = true;
            }
            if (m_targetIndex >= m_path.size())
            {
                return EnFollowStatus::Finished;
            }

            // 進んだら、進めなくなったかの基準を取り直す(この回は時間を数えない)
            bool isReferenceReset = false;
            if (advanced || !m_hasStallReference)
            {
                m_hasStallReference = true;
                m_stallReferenceX = x;
                m_stallReferenceZ = z;
                m_stallElapsed = 0.0f;
                isReferenceReset = true;
            }

            // 経路から外れた。直前のウェイポイントが無い(1点だけの経路)ときは、線が無いので調べない
            if (m_targetIndex > 0 && DistanceToSegment(x, z, m_path[m_targetIndex - 1], m_path[m_targetIndex]) > settings.maxDeviation)
            {
                return EnFollowStatus::NeedRepath;
            }

            // 進めなくなった: 基準の位置からあまり動かないまま、一定の時間が過ぎた
            if (isReferenceReset)
            {
                return EnFollowStatus::Following;
            }
            if (std::hypot(x - m_stallReferenceX, z - m_stallReferenceZ) >= settings.stallDistance)
            {
                m_stallReferenceX = x;
                m_stallReferenceZ = z;
                m_stallElapsed = 0.0f;
            }
            else
            {
                m_stallElapsed += deltaTime;
                if (m_stallElapsed >= settings.stallTime)
                {
                    m_stallElapsed = 0.0f;
                    return EnFollowStatus::NeedRepath;
                }
            }
            return EnFollowStatus::Following;
        }


        bool PathFollower::GetMoveDirection(const float x, const float z, float& outX, float& outZ) const
        {
            if (m_targetIndex >= m_path.size())
            {
                return false;
            }

            const float toX = m_path[m_targetIndex].x - x;
            const float toZ = m_path[m_targetIndex].z - z;
            const float lengthSq = toX * toX + toZ * toZ;
            if (lengthSq <= DIRECTION_NORMALIZE_SQ)
            {
                return false;
            }

            const float length = std::sqrt(lengthSq);
            outX = toX / length;
            outZ = toZ / length;
            return true;
        }
    } // namespace nav
} // namespace app
