/**
 * @file ZombieManager.cpp
 * @brief ゾンビの生成・破棄・保持を行うクラス
 */
#include "stdafx.h"

#include "ZombieManager.h"

#include <optional>
#include <utility>

#include "Source/Actor/Character/Zombie/NormalZombie/NormalZombie.h"
#include "Source/Actor/Character/Zombie/NormalZombie/NormalZombieController.h"
#include "Source/Actor/Character/Zombie/RunningZombie/RunningZombie.h"
#include "Source/Actor/Character/Zombie/RunningZombie/RunningZombieController.h"
#include "Source/Actor/Character/Zombie/Spawn/ZombieSpawnParameter.h"
#include "Source/Core/Param/ParamHolder.h"
#include "Source/MoveBehavior/ControllerSlot.h"
#include "ZombieBase.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /**
             * @brief ゾンビを1体作って、コアへ向かうControllerを付ける
             * @details 座標は Start() の前に設定する。(キャラクターコントローラーが現在の座標に作られる)
             * @tparam TZombie ゾンビの型
             * @tparam TController 付けるControllerの型。コンストラクタにゾンビのパラメーターを取り、SetCorePositionProvider() を持つこと
             * @param position 湧かせる座標
             * @param provider コアの位置を教える関数
             * @param attackHandler 攻撃判定が重なった時の処理
             * @return 作ったゾンビ
             */
            template <typename TZombie, typename TController>
            std::unique_ptr<ZombieBase> CreateZombie(const Vector3& position, const ZombieManager::PositionProvider& provider, const ZombieAttackHandler& attackHandler)
            {
                auto zombie = std::make_unique<TZombie>();
                zombie->SetPosition(position);
                zombie->StartWrapper();
                zombie->SetAttackHandler(attackHandler);

                auto controller = std::make_unique<TController>(zombie->GetParameter());
                controller->SetCorePositionProvider(provider);
                zombie->GetControllerSlot().Attach(std::move(controller));
                return zombie;
            }
        } // namespace


        ZombieManager::ZombieManager()
            : m_parameter(ParamHolder::Get().GetParameter<ZombieSpawnParameter>(EnParamID::ZombieSpawn))
            , m_corePositionProvider(nullptr)
            , m_coreAttackHandler(nullptr)
            , m_spawner()
            , m_zombies()
        {
        }


        ZombieManager::~ZombieManager() = default;


        void ZombieManager::SetCorePositionProvider(const PositionProvider& provider)
        {
            m_corePositionProvider = provider;
        }


        void ZombieManager::SetCoreAttackHandler(const ZombieAttackHandler& handler)
        {
            m_coreAttackHandler = handler;
        }


        void ZombieManager::Preload()
        {
            // 種類ごとに1体作って、すぐに捨てる。座標はどこでもよい(Update() は呼ばないので、動かない)
            const Vector3& position = GetGatePosition(m_parameter, EnZombieGate::West);
            for (int kind = 0; kind < static_cast<int>(EnZombieKind::Max); ++kind)
            {
                // 戻り値を受けないので、この文の終わりで破棄される
                Create(static_cast<EnZombieKind>(kind), position);
            }
        }


        void ZombieManager::Update()
        {
            // 死んだゾンビを外す
            RemoveDeadZombies(m_zombies);

            // 湧かせる時間で、数にも空きがあれば、1体作る
            if (const std::optional<ZombieSpawnRequest> request = m_spawner.Update(m_zombies.size()))
            {
                m_zombies.push_back(Create(request->kind, request->position));
            }

            // 湧いたばかりのゾンビも、描画の前に1回更新する
            for (const std::unique_ptr<ZombieBase>& zombie : m_zombies)
            {
                zombie->UpdateWrapper();
            }
        }


        void ZombieManager::Render(RenderContext& rc)
        {
            for (const std::unique_ptr<ZombieBase>& zombie : m_zombies)
            {
                zombie->RenderWrapper(rc);
            }
        }


        std::unique_ptr<ZombieBase> ZombieManager::Create(const EnZombieKind kind, const Vector3& position) const
        {
            switch (kind)
            {
            case EnZombieKind::Running:
                return CreateZombie<RunningZombie, RunningZombieController>(position, m_corePositionProvider, m_coreAttackHandler);
            case EnZombieKind::Normal:
            default:
                return CreateZombie<NormalZombie, NormalZombieController>(position, m_corePositionProvider, m_coreAttackHandler);
            }
        }
    } // namespace actor
} // namespace app
