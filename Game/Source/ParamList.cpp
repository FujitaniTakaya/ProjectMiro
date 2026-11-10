/**
 * @file ParamList.cpp
 * @brief 全パラメーターの登録一覧
 * @details Core(Source/Core/Param)の外に置いている。
 *          登録には各パラメーターの構造体(Actor・UIなど、上の層にある)が必要で、
 *          Coreに置くと、Coreが上の層をincludeすることになるため。
 *          宣言とIDは、Coreの ParamList.h にある。
 */
#include "stdafx.h"

#include "Source/Core/Param/ParamList.h"

#include "Source/Actor/Character/Miro/MiroParameter.h"
#include "Source/Actor/Character/Zombie/ZombieParameter.h"
#include "Source/Actor/Character/Zombie/ZombieSpawnParameter.h"
#include "Source/Actor/Stage/CoreParameter.h"
#include "Source/Actor/Weapon/WeaponParameter.h"
#include "Source/Camera/PlayerCameraParameter.h"
#include "Source/Core/Param/ParamHolder.h"
// NOTE: パラメーターの構造体と、jsonから読み込む関数のヘッダーは、ここでincludeする。


namespace app
{
    void RegisterAllParams(ParamHolder& holder)
    {
        holder.Register<actor::MiroParameter>(EnParamID::Miro, "Assets/parameter/actor/Miro.json", &actor::LoadMiroParameter, &actor::DrawMiroParameter);
        holder.Register<camera::PlayerCameraParameter>(EnParamID::PlayerCamera, "Assets/parameter/camera/PlayerCamera.json", &camera::LoadPlayerCameraParameter, &camera::DrawPlayerCameraParameter);
        holder.Register<actor::WeaponParameter>(EnParamID::Handgun, "Assets/parameter/actor/Handgun.json", &actor::LoadWeaponParameter, &actor::DrawWeaponParameter);
        holder.Register<actor::CoreParameter>(EnParamID::Core, "Assets/parameter/actor/Core.json", &actor::LoadCoreParameter, &actor::DrawCoreParameter);
        holder.Register<actor::ZombieParameter>(EnParamID::Zombie, "Assets/parameter/actor/Zombie.json", &actor::LoadZombieParameter, &actor::DrawZombieParameter);
        holder.Register<actor::ZombieSpawnParameter>(EnParamID::ZombieSpawn, "Assets/parameter/actor/ZombieSpawn.json", &actor::LoadZombieSpawnParameter, &actor::DrawZombieSpawnParameter);
        holder.Register<actor::ZombieParameter>(EnParamID::RunningZombie, "Assets/parameter/actor/RunningZombie.json", &actor::LoadZombieParameter, &actor::DrawZombieParameter);

        // 例: holder.Register<MiniMapParameter>(EnParamID::MiniMap, "Assets/parameter/ui/MiniMap.json", &LoadMiniMap);
        // 値をデバッグ画面(ParamDebugUI)で見たい場合は、4つ目の引数に表示関数を渡す。
        //     holder.Register<MiniMapParameter>(EnParamID::MiniMap, "Assets/parameter/ui/MiniMap.json", &LoadMiniMap, &DrawMiniMap);
    }
} // namespace app
