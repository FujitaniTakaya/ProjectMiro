/**
 * @file GameResult.cpp
 * @brief インゲームの結果をリザルトシーンへ渡す記録
 */
#include "stdafx.h"

#include "GameResult.h"


namespace app
{
    GameResult::GameResult()
        : m_data()
    {}


    GameResult::~GameResult() = default;


    GameResult& GameResult::Get()
    {
        static GameResult instance;
        return instance;
    }


    void GameResult::Set(const GameResultData& data)
    {
        m_data = data;
    }


    const GameResultData& GameResult::Data() const
    {
        return m_data;
    }
} // namespace app
