/**
 * @file StateMachineBase.cpp
 * @brief アクターのステートマシンの基底クラス群
 */
#include "stdafx.h"

#include "StateMachineBase.h"


namespace app
{
    namespace
    {
        /** ステート用に最初に確保しておく数 */
        constexpr size_t RESERVE_STATE_COUNT = 16;
    } // namespace


    StateMachineBase::StateMachineBase()
        : m_states()
        , m_currentState(nullptr)
    {
        m_states.reserve(RESERVE_STATE_COUNT);
    }


    void StateMachineBase::Update()
    {
        // ステートを変更する
        ChangeState();

        // 現在のステートを更新する
        if (m_currentState)
        {
            m_currentState->Update();
        }
    }


    void StateMachineBase::ChangeState()
    {
        // 変更先のステートを取得する
        IState* nextState = GetChangeState();

        // ステートが変更されている場合
        if (nextState && m_currentState != nextState)
        {
            if (m_currentState)
            {
                m_currentState->Exit();
            }
            m_currentState = nextState;
            m_currentState->Enter();
        }
    }


    void StateMachineBase::ReEnterCurrentState()
    {
        // 現在のステートの Enter() を再度呼び出してアニメーションを再適用する
        if (m_currentState)
        {
            m_currentState->Enter();
        }
    }


    bool StateMachineBase::IsEqualCurrentState(const uint32_t stateID) const
    {
        // 現在のステートが持つIDと比べるだけでよい(登録されていないIDは一致しない)
        return m_currentState && m_currentState->m_stateID == stateID;
    }


    IState* StateMachineBase::FindState(const uint32_t stateID)
    {
        for (const auto& entry : m_states)
        {
            if (entry.id == stateID)
            {
                return entry.state.get();
            }
        }
        // IDが外れ値の場合nullptrを返す
        return nullptr;
    }
} // namespace app
