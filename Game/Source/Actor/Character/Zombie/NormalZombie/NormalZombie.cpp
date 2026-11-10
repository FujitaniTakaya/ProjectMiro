/**
 * @file NormalZombie.cpp
 * @brief 通常のゾンビ
 */
#include "stdafx.h"

#include "NormalZombie.h"

#include <cstddef>
#include <cstdint>
#include <iterator>

#include "Source/Actor/Character/Zombie/ZombieAnimationClipID.h"
#include "Source/Actor/Character/Zombie/ZombieParameter.h"
#include "Source/Actor/ModelData.h"
#include "Source/Core/Param/ParamHolder.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /** モデルのファイルパス。テクスチャは同じフォルダに置く */
            constexpr const char* MODEL_PATH = "Assets/modelData/character/zombie/Zombie.tkm";

            /**
             * アニメーションデータ
             * EnZombieAnimationClipID の順に並べること。先頭は EnCharacterAnimationClipID の順(待機)で、共通ステートが番号で再生する。
             */
            const AnimationData ANIMATION_DATA[] = {
                // 待機
                { "Assets/animData/character/zombie/Zombie_Idle.tka", true },
                // 歩き
                { "Assets/animData/character/zombie/Zombie_Walk.tka", true },
                // 走り
                { "Assets/animData/character/zombie/Zombie_Run.tka", true },
                // 死亡
                { "Assets/animData/character/zombie/Zombie_Death.tka", false },
                // 攻撃
                { "Assets/animData/character/zombie/Zombie_Attack.tka", false },
            };
            static_assert(std::size(ANIMATION_DATA) == static_cast<size_t>(EnZombieAnimationClipID::Max), "アニメーションデータの数をIDと合わせること");

            /** モデルデータ(Z-upのモデル) */
            const ModelData MODEL_DATA = {
                MODEL_PATH,
                ANIMATION_DATA,
                EnModelUpAxis::enModelUpAxisZ,
                static_cast<uint8_t>(std::size(ANIMATION_DATA))
            };
        } // namespace


        NormalZombie::NormalZombie()
            : ZombieBase(ParamHolder::Get().GetParameter<ZombieParameter>(EnParamID::Zombie), MODEL_DATA)
        {
        }


        NormalZombie::~NormalZombie() = default;
    } // namespace actor
} // namespace app
