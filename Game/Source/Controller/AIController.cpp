/**
 * @file AIController.cpp
 * @brief 周りを見て判断し、キャラクターへの入力を作るController(AI)
 */
#include "stdafx.h"
#include "AIController.h"
#include "Source/MoveBehavior/StateMachineBase.h"
#include "Source/Util/RandomDevice.h"


namespace app
{
    namespace
    {
        /** 方向ベクトルを正規化してよい長さの二乗のしきい値(ゼロ除算防止。BeastのCHASE_DIR_NORMALIZE_SQ) */
        constexpr float DIRECTION_NORMALIZE_SQ = 0.001f;
        /** 巡回点がないときの、徘徊の目的地までの距離の最小の割合(wanderRadiusに対する割合) */
        constexpr float WANDER_MIN_RADIUS_RATE = 0.5f;


        /** y成分を0にした値を返す */
        Vector3 ToFlat(const Vector3& v)
        {
            return Vector3(v.x, 0.0f, v.z);
        }

        /** XZ平面での距離の二乗を返す */
        float FlatDistanceSq(const Vector3& a, const Vector3& b)
        {
            return ToFlat(a - b).LengthSq();
        }
    }


    /*************************************************************/
    // 各状態
    //   状態は、次の状態のIDを m_nextStateID に書くだけ。切り替えはステートマシンが次のフレームの頭で行う。
    //   入力は m_output に設定する。(状態が切り替わるフレームも、その状態の入力を出し続ける)
    /*************************************************************/


    /**
     * @brief 待機。その場で止まって、時間が経つと徘徊へ
     */
    class AIController::IdleState : public IState
    {
        appState(Idle)

    public:
        explicit IdleState(AIController& ai)
            : m_ai(ai)
            , m_timeLeft(0.0f)
        {
        }

        void Enter() override;
        void Update() override;
        void Exit() override {}

    private:
        /** 持ち主 */
        AIController& m_ai;
        /** 待機の残り時間(秒) */
        float m_timeLeft;
    };


    /**
     * @brief 徘徊。目的地へ歩き、目標が見えたら追跡へ
     */
    class AIController::WanderingState : public IState
    {
        appState(Wandering)

    public:
        explicit WanderingState(AIController& ai)
            : m_ai(ai)
            , m_destination(Vector3::Zero)
            , m_stuckBasePosition(Vector3::Zero)
            , m_stuckTimer(0.0f)
        {
        }

        void Enter() override;
        void Update() override;
        void Exit() override {}

    private:
        /** 持ち主 */
        AIController& m_ai;
        /** 目的地 */
        Vector3 m_destination;
        /** 停滞の判定の基準にする位置 */
        Vector3 m_stuckBasePosition;
        /** 停滞している時間(秒) */
        float m_stuckTimer;
    };


    /**
     * @brief 追跡。目標を走って追い、近づいたら止まって攻撃する
     */
    class AIController::ChaseState : public IState
    {
        appState(Chase)

    public:
        explicit ChaseState(AIController& ai)
            : m_ai(ai)
            , m_attackTimer(0.0f)
        {
        }

        void Enter() override;
        void Update() override;
        void Exit() override {}

    private:
        /** 持ち主 */
        AIController& m_ai;
        /** 前回の攻撃からの経過時間(秒)。attackInterval に達すると次の攻撃ができる */
        float m_attackTimer;
    };


    /**
     * @brief 帰巣。ホームへ歩いて戻る
     */
    class AIController::ReturnHomeState : public IState
    {
        appState(ReturnHome)

    public:
        explicit ReturnHomeState(AIController& ai)
            : m_ai(ai)
        {
        }

        void Enter() override;
        void Update() override;
        void Exit() override {}

    private:
        /** 持ち主 */
        AIController& m_ai;
    };


    /**
     * @brief AIの状態を切り替えるステートマシン
     */
    class AIController::StateMachine : public StateMachineBase
    {
    public:
        explicit StateMachine(AIController& ai)
            : m_ai(ai)
        {
            AddState<IdleState>(ai);
            AddState<WanderingState>(ai);
            AddState<ChaseState>(ai);
            AddState<ReturnHomeState>(ai);
        }


    protected:
        IState* GetChangeState() override
        {
            return FindState(m_ai.m_nextStateID);
        }


    private:
        /** 持ち主 */
        AIController& m_ai;
    };


    /*************************************************************/


    void AIController::IdleState::Enter()
    {
        m_ai.m_state = EnAIState::Idle;
        // 目標は諦める
        m_ai.m_hasTarget = false;
        // 待機する時間を決める(Beastは毎フレーム決め直していたので、入ったときに1回だけ決める)
        m_timeLeft = util::RandomDevice::Random(0.0f, m_ai.m_config.idleTimeMax);
    }


    void AIController::IdleState::Update()
    {
        // 止まって待つ(入力はなし)
        m_timeLeft -= m_ai.m_context.deltaTime;

        if (m_ai.IsFarFromHome())
        {
            m_ai.m_nextStateID = ReturnHomeState::ID();
        }
        else if (m_timeLeft <= 0.0f)
        {
            m_ai.m_nextStateID = WanderingState::ID();
        }
    }


    void AIController::WanderingState::Enter()
    {
        m_ai.m_state = EnAIState::Wandering;
        m_destination = m_ai.ChooseWanderDestination();
        // 停滞の判定をやり直す
        m_stuckTimer = 0.0f;
        m_stuckBasePosition = m_ai.m_context.position;
    }


    void AIController::WanderingState::Update()
    {
        const ControllerContext& context = m_ai.m_context;
        const AIControllerConfig& config = m_ai.m_config;

        // 目的地へ歩く
        m_ai.MoveToward(m_destination);

        // 停滞の検出: ほとんど動いていなければ時間を進め、動いていればやり直す
        if (FlatDistanceSq(context.position, m_stuckBasePosition) < config.stuckMoveThreshold * config.stuckMoveThreshold)
        {
            m_stuckTimer += context.deltaTime;
        }
        else
        {
            m_stuckTimer = 0.0f;
            m_stuckBasePosition = context.position;
        }

        if (m_ai.TryFindTarget())
        {
            m_ai.m_nextStateID = ChaseState::ID();
        }
        else if (m_ai.IsFarFromHome())
        {
            m_ai.m_nextStateID = ReturnHomeState::ID();
        }
        else if (FlatDistanceSq(context.position, m_destination) <= config.arriveDistWander * config.arriveDistWander)
        {
            // 目的地に着いた
            m_ai.m_nextStateID = IdleState::ID();
        }
        else if (m_stuckTimer >= config.stuckTimeLimit)
        {
            // 動けないので、今の目的地は諦める
            m_ai.m_nextStateID = IdleState::ID();
        }
    }


    void AIController::ChaseState::Enter()
    {
        m_ai.m_state = EnAIState::Chase;
        // 追いついたらすぐ攻撃できるようにする
        m_attackTimer = m_ai.m_config.attackInterval;
    }


    void AIController::ChaseState::Update()
    {
        const ControllerContext& context = m_ai.m_context;
        const AIControllerConfig& config = m_ai.m_config;

        // 目標の位置を追う。目標がいなくなっていたら、最後に見た位置へ向かう
        Vector3 livePosition(Vector3::Zero);
        if (m_ai.m_targetProvider && m_ai.m_targetProvider(livePosition))
        {
            m_ai.m_hasTarget = true;
            m_ai.m_targetPosition = livePosition;
            m_ai.m_lastKnownTargetPosition = livePosition;
        }
        else
        {
            m_ai.m_hasTarget = false;
        }

        m_ai.m_chaseTimeLeft -= context.deltaTime;
        m_attackTimer = (std::min)(m_attackTimer + context.deltaTime, config.attackInterval);

        const Vector3& destination = m_ai.m_hasTarget ? m_ai.m_targetPosition : m_ai.m_lastKnownTargetPosition;
        const float distanceSq = FlatDistanceSq(context.position, destination);

        if (m_ai.m_hasTarget && distanceSq <= config.attackDistance * config.attackDistance)
        {
            // 攻撃: 止まって、間隔ごとにAction1を1フレームだけ押す
            if (m_attackTimer >= config.attackInterval)
            {
                m_ai.m_output.SetPress(EnInputButton::Action1);
                m_attackTimer = 0.0f;
            }
        }
        else
        {
            // 走って追う
            m_ai.MoveToward(destination);
            m_ai.m_output.SetPress(EnInputButton::Dash);
        }

        if (m_ai.m_chaseTimeLeft <= 0.0f)
        {
            // 追い続けて疲れたので、諦めてホームへ戻る
            m_ai.m_nextStateID = ReturnHomeState::ID();
        }
        else if (m_ai.m_hasTarget && distanceSq > config.lostChaseDistance * config.lostChaseDistance)
        {
            // 離されすぎたので、諦めてホームへ戻る
            m_ai.m_nextStateID = ReturnHomeState::ID();
        }
        else if (!m_ai.m_hasTarget && distanceSq <= config.arriveLastKnownDist * config.arriveLastKnownDist)
        {
            // 見失った位置に着いたが、目標はいなかった
            m_ai.m_nextStateID = IdleState::ID();
        }
    }


    void AIController::ReturnHomeState::Enter()
    {
        m_ai.m_state = EnAIState::ReturnHome;
        // 目標は諦める
        m_ai.m_hasTarget = false;
    }


    void AIController::ReturnHomeState::Update()
    {
        const AIControllerConfig& config = m_ai.m_config;

        m_ai.MoveToward(m_ai.m_homePosition);

        if (FlatDistanceSq(m_ai.m_context.position, m_ai.m_homePosition) < config.arriveDistReturnHome * config.arriveDistReturnHome)
        {
            // ホームに着いた。休んで、追跡できる時間を回復する
            m_ai.m_chaseTimeLeft = config.maxChaseSeconds;
            m_ai.m_nextStateID = IdleState::ID();
        }
    }


    /*************************************************************/


    AIController::AIController(const AIControllerConfig& config)
        : m_config(config)
        , m_targetProvider(nullptr)
        , m_homePosition(Vector3::Zero)
        , m_isHomeFixed(false)
        , m_patrolPoints()
        , m_patrolIndex(0)
        , m_context()
        , m_output()
        , m_state(EnAIState::Idle)
        , m_nextStateID(IdleState::ID())
        , m_isFirstUpdate(true)
        , m_hasTarget(false)
        , m_targetPosition(Vector3::Zero)
        , m_lastKnownTargetPosition(Vector3::Zero)
        , m_chaseTimeLeft(config.maxChaseSeconds)
        , m_machine(std::make_unique<StateMachine>(*this))
    {
    }


    AIController::~AIController() = default;


    void AIController::SetTargetProvider(const TargetProvider& provider)
    {
        m_targetProvider = provider;
    }


    void AIController::SetHomePosition(const Vector3& position)
    {
        m_homePosition = position;
        m_isHomeFixed = true;
    }


    void AIController::AddPatrolPoint(const Vector3& position)
    {
        m_patrolPoints.push_back(position);
    }


    void AIController::OnAttach()
    {
        // 別のキャラクターへ付け替えられたときのために、状態を最初に戻す
        m_state = EnAIState::Idle;
        m_nextStateID = IdleState::ID();
        m_isFirstUpdate = true;
        m_hasTarget = false;
        m_chaseTimeLeft = m_config.maxChaseSeconds;
        m_machine = std::make_unique<StateMachine>(*this);
    }


    CharacterInput AIController::Update(const ControllerContext& context)
    {
        m_context = context;
        m_output = CharacterInput();

        // ホームが指定されていなければ、最初に付けられた位置をホームにする
        if (m_isFirstUpdate)
        {
            m_isFirstUpdate = false;
            if (!m_isHomeFixed)
            {
                m_homePosition = context.position;
            }
        }

        m_machine->Update();

        return m_output;
    }


    bool AIController::IsFarFromHome() const
    {
        return FlatDistanceSq(m_context.position, m_homePosition) > m_config.maxDistFromHome * m_config.maxDistFromHome;
    }


    bool AIController::TryFindTarget()
    {
        Vector3 targetPosition(Vector3::Zero);
        if (!m_targetProvider || !m_targetProvider(targetPosition))
        {
            return false;
        }

        // 距離
        Vector3 toTarget = ToFlat(targetPosition - m_context.position);
        if (toTarget.LengthSq() > m_config.sightDistance * m_config.sightDistance)
        {
            return false;
        }

        // 視野(体の正面を基準にする)。向きが分からなければ見えない
        Vector3 forward = ToFlat(m_context.forward);
        if (forward.LengthSq() <= FLT_EPSILON)
        {
            return false;
        }
        forward.Normalize();
        // 真上にいるときは方向が決まらないので、視野の中として扱う
        if (toTarget.LengthSq() > FLT_EPSILON)
        {
            toTarget.Normalize();
            if (forward.Dot(toTarget) < cosf(Math::DegToRad(m_config.sightHalfAngleDeg)))
            {
                return false;
            }
        }

        // 遮るものがないか
        if (!HasLineOfSight(targetPosition))
        {
            return false;
        }

        m_hasTarget = true;
        m_targetPosition = targetPosition;
        m_lastKnownTargetPosition = targetPosition;
        return true;
    }


    bool AIController::HasLineOfSight(const Vector3& targetPosition) const
    {
        // 物理ワールドがなければ、遮るものはないとみなす
        PhysicsWorld* physicsWorld = PhysicsWorld::GetInstance();
        if (physicsWorld == nullptr)
        {
            return true;
        }

        // 足元だと地面に当たってしまうので、目の高さに持ち上げる
        const Vector3 eyeOffset(0.0f, m_config.eyeHeight, 0.0f);
        Vector3 start = m_context.position + eyeOffset;
        Vector3 end = targetPosition + eyeOffset;

        // 両端を縮めて、自分と目標のカプセルに当たらないようにする
        Vector3 direction = end - start;
        const float length = direction.Length();
        if (length <= m_config.rayEndMargin * 2.0f)
        {
            // 近すぎて、間に遮るものは入らない
            return true;
        }
        direction = direction * (1.0f / length);
        start = start + direction * m_config.rayEndMargin;
        end = end - direction * m_config.rayEndMargin;

        // 何かに当たれば見えない
        Vector3 hitPosition(Vector3::Zero);
        return !physicsWorld->RayTest(start, end, hitPosition);
    }


    Vector3 AIController::ChooseWanderDestination()
    {
        // 巡回点があれば、順に回る
        if (!m_patrolPoints.empty())
        {
            const Vector3 destination = m_patrolPoints[m_patrolIndex];
            m_patrolIndex = (m_patrolIndex + 1) % m_patrolPoints.size();
            return destination;
        }

        // なければ、ホームの周りのランダムな点
        const float angle = util::RandomDevice::Random(0.0f, Math::PI2);
        const float radius = m_config.wanderRadius * util::RandomDevice::Random(WANDER_MIN_RADIUS_RATE, 1.0f);
        return m_homePosition + Vector3(cosf(angle) * radius, 0.0f, sinf(angle) * radius);
    }


    void AIController::MoveToward(const Vector3& destination)
    {
        Vector3 direction = ToFlat(destination - m_context.position);
        if (direction.LengthSq() > DIRECTION_NORMALIZE_SQ)
        {
            direction.Normalize();
            m_output.SetMove(direction);
        }
    }
}
