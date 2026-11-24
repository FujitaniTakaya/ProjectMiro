/**
 * @file BulletSweep.cpp
 * @brief 弾の当たり判定(球のスイープ)と、レティクルへ向かう発射角度の計算
 */
#include "stdafx.h"

#include "BulletSweep.h"

#include <cfloat>
#include <cmath>


namespace app
{
    namespace actor
    {
        namespace
        {
            /** 狙い点が銃口からこれより近い時は、向きが不安定なので狙い点を使わない(cm) */
            constexpr float MIN_AIM_DISTANCE = 50.0f;
            /** 向きがほぼ同じ・ほぼ逆と見なす内積の境目。Quaternion::SetRotation(from, to) の 0.998 より、ずっときつくしてある */
            constexpr float PARALLEL_DOT = 0.999999f;


            /**
             * @brief 球のスイープの結果を受けるコールバック
             * @details 最も近い当たりを1つだけ持つ。当たる相手は SweepSphere() の説明の通り。
             */
            struct BulletSweepCallback : public btCollisionWorld::ClosestConvexResultCallback
            {
                BulletSweepCallback(const btVector3& from, const btVector3& to, const btCollisionObject* ignore)
                    : ClosestConvexResultCallback(from, to)
                    , m_ignore(ignore)
                {}


                bool needsCollision(btBroadphaseProxy* proxy) const override
                {
                    const auto* object = static_cast<const btCollisionObject*>(proxy->m_clientObject);
                    if (object == m_ignore)
                    {
                        return false;
                    }
                    // ゴーストは、ユーザー属性を付けたもの(コアの箱)にだけ当たる。ゾンビの攻撃判定の箱などは素通りする
                    if (object->getInternalType() == btCollisionObject::CO_GHOST_OBJECT && object->getUserIndex() != enCollisionAttr_User)
                    {
                        return false;
                    }
                    return ClosestConvexResultCallback::needsCollision(proxy);
                }


                /** 無視する物理オブジェクト */
                const btCollisionObject* m_ignore;
            };
        } // namespace


        bool SweepSphere(const Vector3& from, const Vector3& to, const float radius, const btCollisionObject* ignore, BulletHit& hit)
        {
            // 動かない球は、スイープできない
            const Vector3 move = to - from;
            if (move.Length() < FLT_EPSILON)
            {
                return false;
            }

            btSphereShape shape(radius);
            btTransform start;
            start.setIdentity();
            start.setOrigin(btVector3(from.x, from.y, from.z));
            btTransform end;
            end.setIdentity();
            end.setOrigin(btVector3(to.x, to.y, to.z));

            BulletSweepCallback callback(start.getOrigin(), end.getOrigin(), ignore);
            PhysicsWorld::GetInstance()->ConvexSweepTest(&shape, start, end, callback);
            if (!callback.hasHit())
            {
                return false;
            }

            hit.object = callback.m_hitCollisionObject;
            hit.position = from + move * callback.m_closestHitFraction;
            hit.point = Vector3(callback.m_hitPointWorld.x(), callback.m_hitPointWorld.y(), callback.m_hitPointWorld.z());
            hit.normal = Vector3(callback.m_hitNormalWorld.x(), callback.m_hitNormalWorld.y(), callback.m_hitNormalWorld.z());
            return true;
        }


        Quaternion MakeRotationFromTo(const Vector3& from, const Vector3& to)
        {
            Vector3 fromDirection = from;
            fromDirection.Normalize();
            Vector3 toDirection = to;
            toDirection.Normalize();

            const float dot = Dot(fromDirection, toDirection);
            if (dot > PARALLEL_DOT)
            {
                // ほぼ同じ向き
                return Quaternion::Identity;
            }

            Vector3 axis;
            if (dot < -PARALLEL_DOT)
            {
                // ほぼ逆向き。from に垂直な軸なら、どれで半回転してもよい
                axis = Cross(fromDirection, Vector3::AxisX);
                if (axis.Length() < 0.1f)
                {
                    axis = Cross(fromDirection, Vector3::AxisY);
                }
            }
            else
            {
                axis = Cross(fromDirection, toDirection);
            }
            axis.Normalize();

            Quaternion rotation;
            rotation.SetRotation(axis, std::acos(dot));
            return rotation;
        }


        bool SweepAimRay(
            const Vector3& cameraPosition,
            const Vector3& cameraForward,
            const Vector3& muzzle,
            const float range,
            const float radius,
            const btCollisionObject* ignore,
            BulletHit& hit,
            Vector3* endPoint
        )
        {
            Vector3 forward = cameraForward;
            forward.Normalize();

            // 視線の始点は、銃口と同じ奥行きの面に置く。(カメラが後ろにいても、銃口より手前の物には当たらない)
            const float depth = Dot(muzzle - cameraPosition, forward);
            const Vector3 start = cameraPosition + forward * (depth > 0.0f ? depth : 0.0f);
            const Vector3 end = start + forward * range;
            if (endPoint != nullptr)
            {
                *endPoint = end;
            }

            return SweepSphere(start, end, radius, ignore, hit);
        }


        Quaternion CalcAimRotation(
            const Vector3& cameraPosition,
            const Vector3& cameraForward,
            const Vector3& muzzle,
            const float range,
            const float radius,
            const btCollisionObject* ignore,
            const Quaternion& fallback
        )
        {
            BulletHit hit;
            Vector3 end;
            const Vector3 aimPoint = SweepAimRay(cameraPosition, cameraForward, muzzle, range, radius, ignore, hit, &end) ? hit.position : end;

            Vector3 direction = aimPoint - muzzle;
            if (direction.Length() < MIN_AIM_DISTANCE)
            {
                return fallback;
            }
            direction.Normalize();

            // Bullet は、発射角度で Vector3::Front を回した向きに進む
            return MakeRotationFromTo(Vector3::Front, direction);
        }
    } // namespace actor
} // namespace app
