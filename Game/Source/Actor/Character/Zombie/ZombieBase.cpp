/**
 * @file ZombieBase.cpp
 * @brief ゾンビの基底クラス
 */
#include "stdafx.h"

#include "ZombieBase.h"

#include <algorithm>

#include "Source/MoveBehavior/ControllerSlot.h"
#include "ZombieParameter.h"
#include "ZombieStateMachine.h"


namespace app
{
    namespace actor
    {
        ZombieBase::ZombieBase(const ZombieParameter& parameter, const ModelData& modelData)
            : m_parameter(parameter)
            , m_modelData(modelData)
            , m_stateMachine(nullptr)
            , m_anger(0.0f)
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


        void ZombieBase::Start()
        {
            // モデル・アニメーションクリップのロードと、キャラクターコントローラーの初期化(現在のHPも最大HPで始まる)
            Init(m_modelData, m_parameter.radius, m_parameter.height);

            // 累積アンガー値は、初期アンガー値から始める
            m_anger = m_parameter.initialAnger;
        }


        void ZombieBase::Update()
        {
            // 入力の作成とステートの更新(入力はステートマシンが持つコントローラーから作る)
            m_stateMachine->Update();

            // モデルの更新
            CharacterBase::Update();
        }


        void ZombieBase::Render(RenderContext& rc)
        {
            CharacterBase::Render(rc);
        }
    } // namespace actor
} // namespace app
