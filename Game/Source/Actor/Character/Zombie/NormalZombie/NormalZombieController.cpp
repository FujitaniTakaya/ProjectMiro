/**
 * @file NormalZombieController.cpp
 * @brief 通常のゾンビ用のController
 */
#include "stdafx.h"

#include "NormalZombieController.h"

#include "Source/Actor/Character/Zombie/ZombieParameter.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /** 方向ベクトルを正規化してよい長さの二乗のしきい値(ゼロ除算防止。AIControllerと同じ値) */
            constexpr float DIRECTION_NORMALIZE_SQ = 0.001f;
        } // namespace


        NormalZombieController::NormalZombieController(const ZombieParameter& parameter)
            : m_parameter(parameter)
            , m_corePositionProvider(nullptr)
        {
        }


        void NormalZombieController::SetCorePositionProvider(const PositionProvider& provider)
        {
            m_corePositionProvider = provider;
        }


        CharacterInput NormalZombieController::Update(const ControllerContext& context)
        {
            CharacterInput input;

            // コアがなければ無入力
            Vector3 corePosition(Vector3::Zero);
            if (!m_corePositionProvider || !m_corePositionProvider(corePosition))
            {
                return input;
            }

            // 高さは無視して、XZ平面でコアへ向かう
            Vector3 toCore(corePosition.x - context.position.x, 0.0f, corePosition.z - context.position.z);
            const float distanceSq = toCore.LengthSq();

            // コアに着いていれば立ち止まる。真上・真下にいて向きが決まらないときも、動かない
            const float stopDistance = m_parameter.coreStopDistance;
            if (distanceSq <= stopDistance * stopDistance || distanceSq <= DIRECTION_NORMALIZE_SQ)
            {
                return input;
            }

            // ボタンは押さない。移動入力だけで、ゾンビは歩きになる
            toCore.Normalize();
            input.SetMove(toCore);
            return input;
        }
    } // namespace actor
} // namespace app
