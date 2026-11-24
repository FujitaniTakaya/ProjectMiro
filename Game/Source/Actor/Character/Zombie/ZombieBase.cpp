/**
 * @file ZombieBase.cpp
 * @brief ゾンビの基底クラス
 */
#include "stdafx.h"

#include "ZombieBase.h"

#include <algorithm>
#include <cwchar>

#include "Source/MoveBehavior/ControllerSlot.h"
#include "ZombieParameter.h"
#include "ZombieStateMachine.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /** 攻撃判定を付けるボーンの名前(右手。攻撃アニメで振り下ろされるのが右手) */
            constexpr const wchar_t* ATTACK_BONE_NAME = L"hand_r";
            /** 上半身レイヤー(怯み)が効くボーンの根の名前。ここから先(背骨・両腕・首・頭)が怯みで動く。腰と脚は歩き・走りのまま */
            constexpr const wchar_t* UPPER_BODY_ROOT_BONE_NAME = L"spine_01";
            /** 攻撃アニメのイベント名: 攻撃判定を出す(振り下ろしの始まり) */
            constexpr const wchar_t* ATTACK_BEGIN_EVENT = L"AttackBegin";
            /** 攻撃アニメのイベント名: 攻撃判定を消す(振り下ろしの終わり) */
            constexpr const wchar_t* ATTACK_END_EVENT = L"AttackEnd";


            /**
             * @brief ボーンのワールド座標を取得する
             * @param bone ボーン
             * @return ワールド座標
             */
            Vector3 GetBonePosition(const Bone& bone)
            {
                const Matrix& world = bone.GetWorldMatrix();
                return Vector3(world.m[3][0], world.m[3][1], world.m[3][2]);
            }
        } // namespace


        ZombieBase::ZombieBase(const ZombieParameter& parameter, const ModelData& modelData)
            : m_parameter(parameter)
            , m_modelData(modelData)
            , m_stateMachine(nullptr)
            , m_anger(0.0f)
            , m_isLockedOn(false)
            , m_hasLockOnEvent(false)
            , m_attackHitBox()
            , m_attackHandler(nullptr)
            , m_attackBone(nullptr)
            , m_isAttackActive(false)
            , m_hasAttackHit(false)
            , m_isDead(false)
        {
            m_stateMachine = std::make_unique<ZombieStateMachine>(this, m_parameter);
        }


        ZombieBase::~ZombieBase() = default;


        ControllerSlot& ZombieBase::GetControllerSlot()
        {
            return m_stateMachine->GetControllerSlot();
        }


        float ZombieBase::GetMaxHp() const
        {
            return m_parameter.maxHp;
        }


        void ZombieBase::SetAnger(const float anger)
        {
            m_anger = std::max<float>(anger, 0.0f);
        }


        void ZombieBase::AddAnger(const float amount)
        {
            if (amount <= 0.0f)
            {
                return;
            }
            SetAnger(m_anger + amount);
            // 初めてしきい値に達した時だけ、ロックオンにして、伝播の通知を予約する
            if (!m_isLockedOn && m_anger >= m_parameter.angerThreshold)
            {
                m_isLockedOn = true;
                m_hasLockOnEvent = true;
            }
        }


        void ZombieBase::TakeShotDamage(const float damage)
        {
            TakeDamage(damage);
            AddAnger(damage * m_parameter.angerGainRate);

            // 倒れていなければ、上半身だけ怯ませる。(殺した弾は、死亡アニメーションを上書きしないよう怯ませない)
            if (!IsDead())
            {
                m_stateMachine->RequestFlinch();
            }
        }


        bool ZombieBase::ConsumeLockOnEvent()
        {
            const bool hasEvent = m_hasLockOnEvent;
            m_hasLockOnEvent = false;
            return hasEvent;
        }


        void ZombieBase::SetAttackHandler(const ZombieAttackHandler& handler)
        {
            m_attackHandler = handler;
        }


        bool ZombieBase::IsBody(const btCollisionObject& object)
        {
            return &object == m_characterController.GetRigidBody()->GetBody();
        }


        bool ZombieBase::IsDeathFinished() const
        {
            // m_isDead を条件に含めるのは、Init() 前(HPが初期値の0)に、死亡扱いされないため
            return m_isDead && m_stateMachine->IsDeathFinished();
        }


        void ZombieBase::Start()
        {
            // モデル・アニメーションクリップのロードと、キャラクターコントローラーの初期化(現在のHPも最大HPで始まる)
            Init(m_modelData, m_parameter.radius, m_parameter.height);

            // 攻撃判定を付ける右手のボーンと、攻撃アニメのイベントの受け取り。(this はこのゾンビが生きている間、有効)
            m_attackBone = m_modelRender.FindBone(ATTACK_BONE_NAME);

            // 怯みを重ねる上半身の範囲
            m_modelRender.SetLayerMask(EnAnimationLayer::Upper, UPPER_BODY_ROOT_BONE_NAME);
            m_modelRender.AddAnimationEventListener([this]([[maybe_unused]] const wchar_t* clipName, const wchar_t* eventName) { OnAnimationEvent(eventName); });

            // 累積アンガー値は、初期アンガー値から始める。(初期値がしきい値以上なら、最初からロックオン)
            m_anger = 0.0f;
            m_isLockedOn = false;
            m_hasLockOnEvent = false;
            AddAnger(m_parameter.initialAnger);
        }


        void ZombieBase::Update()
        {
            // 死亡した瞬間に、1度だけ後始末する。(ステートマシンの更新より前なので、死亡ステートの1フレーム目から体は無い)
            if (!m_isDead && IsDead())
            {
                OnDead();
            }

            // 入力の作成とステートの更新(入力はステートマシンが持つコントローラーから作る)
            m_stateMachine->Update();

            // モデルの更新(アニメーションのイベントもここで届く。右手のワールド座標もここで確定する)
            CharacterBase::Update();

            // 確定した右手の位置に攻撃判定を合わせて、重なった物を渡す
            UpdateAttackHitBox();
        }


        void ZombieBase::Render(RenderContext& rc)
        {
            CharacterBase::Render(rc);
        }


        void ZombieBase::OnDead()
        {
            m_isDead = true;

            m_attackHitBox.Release();
            m_isAttackActive = false;

            // デストラクタの RemoveRigidBoby() で二重に外れるが、bullet 側では無害
            m_characterController.RemoveRigidBoby();
        }


        void ZombieBase::OnAnimationEvent(const wchar_t* eventName)
        {
            // 死亡した後に届いたイベントで、攻撃判定を出さない
            if (m_isDead)
            {
                return;
            }

            if (std::wcscmp(eventName, ATTACK_BEGIN_EVENT) == 0)
            {
                // 右手のボーンが無ければ、攻撃判定は出ない
                if (m_attackBone == nullptr)
                {
                    return;
                }

                // 右手に箱を出す。位置はこの後の UpdateAttackHitBox() で、更新された右手に合わせる
                const float size = m_parameter.attackBoxSize;
                m_attackHitBox.CreateBox(GetBonePosition(*m_attackBone), Quaternion::Identity, Vector3(size, size, size));
                m_isAttackActive = true;
                m_hasAttackHit = false;
            }
            else if (std::wcscmp(eventName, ATTACK_END_EVENT) == 0)
            {
                m_attackHitBox.Release();
                m_isAttackActive = false;
            }
        }


        void ZombieBase::UpdateAttackHitBox()
        {
            if (!m_isAttackActive)
            {
                return;
            }

            m_attackHitBox.SetPosition(GetBonePosition(*m_attackBone));

            // 今の振りでもう当たった、または処理が無ければ、渡さない
            if (m_hasAttackHit || !m_attackHandler)
            {
                return;
            }
            PhysicsWorld::GetInstance()->ContactTest(&m_attackHitBox.GetbtCollisionObject(), [this](const btCollisionObject& contact) {
                // 1つの振りで、ダメージは1回だけ。同じフレームに複数の判定が重なっても、最初の当たりだけを数える
                if (!m_hasAttackHit && m_attackHandler(contact, m_parameter.attackPower))
                {
                    m_hasAttackHit = true;
                }
            });
        }
    } // namespace actor
} // namespace app
