/**
 * @file BulletManager.cpp
 * @brief 弾の生成・更新・描画・破棄を行うクラス
 */
#include "stdafx.h"

#include "BulletManager.h"

#include <algorithm>
#include <utility>

#include "Bullet.h"


namespace app
{
    namespace actor
    {
        BulletManager::BulletManager()
            : m_bullets()
            , m_hitHandler(nullptr)
        {}


        BulletManager::~BulletManager()
        {
            // NOTE: 弾はエンジンの資源を持つ。プロセスの終了時には、すでに Clear() されていること。
            K2_ASSERT(m_bullets.empty(), "BulletManager::Clear() を呼ばずに終了しました。\n");
        }


        BulletManager& BulletManager::Get()
        {
            static BulletManager instance;
            return instance;
        }


        void BulletManager::Shot(const Quaternion& rotation, const Vector3& position, const float speed, const btCollisionObject* ignore)
        {
            auto bullet = std::make_unique<Bullet>();
            bullet->Init(rotation, position, speed);
            bullet->SetIgnoreObject(ignore);
            bullet->StartWrapper();
            m_bullets.push_back(std::move(bullet));
        }


        void BulletManager::SetHitHandler(const BulletHitHandler& handler)
        {
            m_hitHandler = handler;
        }


        void BulletManager::NotifyHit(const BulletHit& hit)
        {
            if (m_hitHandler)
            {
                m_hitHandler(hit);
            }
        }


        void BulletManager::Update()
        {
            for (const std::unique_ptr<Bullet>& bullet : m_bullets)
            {
                bullet->UpdateWrapper();
                // 当たった弾は、この後に外す。当たった時の処理は、外す前の1回だけ呼ぶ
                if (bullet->HasHit())
                {
                    NotifyHit(bullet->GetHit());
                }
            }

            // 当たった弾・寿命が尽きた弾を外す
            m_bullets.erase(
                std::remove_if(m_bullets.begin(), m_bullets.end(), [](const std::unique_ptr<Bullet>& bullet) { return bullet->IsFinished(); }),
                m_bullets.end()
            );
        }


        void BulletManager::Render(RenderContext& rc)
        {
            for (const std::unique_ptr<Bullet>& bullet : m_bullets)
            {
                bullet->RenderWrapper(rc);
            }
        }


        void BulletManager::Clear()
        {
            m_bullets.clear();
            // 当たった時の処理は、シーンのポインタを持つので、シーンの終了と一緒に外す
            m_hitHandler = nullptr;
        }
    } // namespace actor
} // namespace app
