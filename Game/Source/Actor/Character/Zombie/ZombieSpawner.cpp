/**
 * @file ZombieSpawner.cpp
 * @brief ゾンビを湧かせて、コアへ向かわせるクラス
 */
#include "stdafx.h"

#include "ZombieSpawner.h"

#include <cmath>
#include <utility>

#include "NormalZombie/NormalZombie.h"
#include "NormalZombie/NormalZombieController.h"
#include "RunningZombie/RunningZombie.h"
#include "RunningZombie/RunningZombieController.h"
#include "Source/Core/Param/ParamHolder.h"
#include "Source/Core/Util/RandomDevice.h"
#include "Source/MoveBehavior/ControllerSlot.h"
#include "ZombieBase.h"
#include "ZombieSpawnParameter.h"


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
             * @return 作ったゾンビ
             */
            template <typename TZombie, typename TController>
            std::unique_ptr<ZombieBase> CreateZombie(const Vector3& position, const ZombieSpawner::PositionProvider& provider)
            {
                auto zombie = std::make_unique<TZombie>();
                zombie->SetPosition(position);
                zombie->StartWrapper();

                auto controller = std::make_unique<TController>(zombie->GetParameter());
                controller->SetCorePositionProvider(provider);
                zombie->GetControllerSlot().Attach(std::move(controller));
                return zombie;
            }
        } // namespace


        ZombieSpawner::ZombieSpawner()
            : m_parameter(ParamHolder::Get().GetParameter<ZombieSpawnParameter>(EnParamID::ZombieSpawn))
            , m_corePositionProvider(nullptr)
            , m_zombies()
            , m_elapsed(0.0f)
            , m_spawnedCount(0)
        {
        }


        ZombieSpawner::~ZombieSpawner() = default;


        void ZombieSpawner::SetCorePositionProvider(const PositionProvider& provider)
        {
            m_corePositionProvider = provider;
        }


        void ZombieSpawner::Preload()
        {
            // 種類ごとに1体作って、すぐに捨てる。座標はどこでもよい(Update() は呼ばないので、動かない)
            const Vector3& position = GetGatePosition(m_parameter, EnZombieGate::West);
            for (int kind = 0; kind < static_cast<int>(EnZombieKind::Max); ++kind)
            {
                Create(static_cast<EnZombieKind>(kind), position);
            }
        }


        void ZombieSpawner::Update()
        {
            // 死んだゾンビを外す
            RemoveDeadZombies(m_zombies);

            // 湧かせる時間で、数にも空きがあれば、1体湧かせる
            m_elapsed = AdvanceSpawnTimer(m_elapsed, g_gameTime->GetFrameDeltaTime(), m_parameter.spawnInterval);
            if (CanSpawn(m_elapsed, m_parameter.spawnInterval, m_zombies.size(), m_parameter.maxAliveCount))
            {
                SpawnOne();
                m_elapsed = 0.0f;
            }

            // 湧いたばかりのゾンビも、描画の前に1回更新する
            for (const std::unique_ptr<ZombieBase>& zombie : m_zombies)
            {
                zombie->UpdateWrapper();
            }
        }


        void ZombieSpawner::Render(RenderContext& rc)
        {
            for (const std::unique_ptr<ZombieBase>& zombie : m_zombies)
            {
                zombie->RenderWrapper(rc);
            }
        }


        void ZombieSpawner::SpawnOne()
        {
            // ゲートを、4つからランダムに選ぶ
            const int gateIndex = util::RandomDevice::Random(0, static_cast<int>(EnZombieGate::Max) - 1);
            Vector3 position = GetGatePosition(m_parameter, static_cast<EnZombieGate>(gateIndex));

            // ゲートの中心から、円の中でずらす。(同じ点に湧くとゾンビ同士がぶつかって詰まる。半径の平方根をとって、円の中で一様にばらけさせる)
            const float angle = util::RandomDevice::Random(0.0f, Math::PI2);
            const float radius = m_parameter.gateJitterRadius * std::sqrt(util::RandomDevice::Random(0.0f, 1.0f));
            position.x += std::cos(angle) * radius;
            position.z += std::sin(angle) * radius;

            // 浮かせて湧かせる。高台のゲートでも、落ちて接地する
            position.y += m_parameter.dropHeight;

            m_zombies.push_back(Create(PickZombieKind(m_parameter.spawnOrder, m_spawnedCount), position));
            ++m_spawnedCount;
        }


        std::unique_ptr<ZombieBase> ZombieSpawner::Create(const EnZombieKind kind, const Vector3& position) const
        {
            switch (kind)
            {
            case EnZombieKind::Running:
                return CreateZombie<RunningZombie, RunningZombieController>(position, m_corePositionProvider);
            case EnZombieKind::Normal:
            default:
                return CreateZombie<NormalZombie, NormalZombieController>(position, m_corePositionProvider);
            }
        }
    } // namespace actor
} // namespace app
