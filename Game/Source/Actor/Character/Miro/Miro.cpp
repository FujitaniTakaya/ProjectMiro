/**
 * @file Miro.cpp
 * @brief 主人公のMiro
 */
#include "stdafx.h"

#include "Miro.h"

#include <cstddef>
#include <cstdint>
#include <iterator>

#include "MiroAnimationClipID.h"
#include "MiroParameter.h"
#include "MiroStateMachine.h"
#include "Source/Actor/ModelData.h"
#include "Source/Controller/PlayerController.h"
#include "Source/Core/Param/ParamHolder.h"
#include "Source/Manager/InGameInfoHub.h"
#include "Source/MoveBehavior/ControllerSlot.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /** モデルのファイルパス。テクスチャは同じフォルダに置く */
            constexpr const char* MODEL_PATH = "Assets/modelData/character/miro/miro.tkm";

            /**
             * アニメーションデータ
             * EnMiroAnimationClipID の順に並べること。先頭は EnCharacterAnimationClipID の順(待機)で、共通ステートが番号で再生する。
             */
            const AnimationData ANIMATION_DATA[] = {
                // 共通
                { "Assets/animData/character/miro/Miro_Idle.tka", true },
                // 歩き
                { "Assets/animData/character/miro/walk/Miro_Walk_Fwd.tka", true },
                { "Assets/animData/character/miro/walk/Miro_Walk_Fwd_Left.tka", true },
                { "Assets/animData/character/miro/walk/Miro_Walk_Fwd_Right.tka", true },
                { "Assets/animData/character/miro/walk/Miro_Walk_Left.tka", true },
                { "Assets/animData/character/miro/walk/Miro_Walk_Right.tka", true },
                { "Assets/animData/character/miro/walk/Miro_Walk_Bwd.tka", true },
                { "Assets/animData/character/miro/walk/Miro_Walk_Bwd_Left.tka", true },
                { "Assets/animData/character/miro/walk/Miro_Walk_Bwd_Right.tka", true },
                // 走り
                { "Assets/animData/character/miro/dash/Miro_Run_Fwd.tka", true },
                { "Assets/animData/character/miro/dash/Miro_Run_Fwd_Left.tka", true },
                { "Assets/animData/character/miro/dash/Miro_Run_Fwd_Right.tka", true },
                { "Assets/animData/character/miro/dash/Miro_Run_Left.tka", true },
                { "Assets/animData/character/miro/dash/Miro_Run_Right.tka", true },
                { "Assets/animData/character/miro/dash/Miro_Run_Bwd.tka", true },
                { "Assets/animData/character/miro/dash/Miro_Run_Bwd_Left.tka", true },
                { "Assets/animData/character/miro/dash/Miro_Run_Bwd_Right.tka", true },
                // ハンドガン
                { "Assets/animData/character/miro/pistol/Miro_GunShot01.tka", false },
                { "Assets/animData/character/miro/pistol/Miro_GunShot02.tka", false },
                { "Assets/animData/character/miro/pistol/Miro_GunShot03.tka", false },
                { "Assets/animData/character/miro/pistol/Miro_Pistol_DryFire.tka", false },
                { "Assets/animData/character/miro/pistol/Miro_Pistol_Equip.tka", false },
                { "Assets/animData/character/miro/pistol/Miro_Pistol_Reload.tka", false },
            };
            static_assert(std::size(ANIMATION_DATA) == static_cast<size_t>(EnMiroAnimationClipID::Max), "アニメーションデータの数をIDと合わせること");

            /** モデルデータ(Z-upのモデル) */
            const ModelData MODEL_DATA = {
                MODEL_PATH,
                ANIMATION_DATA,
                EnModelUpAxis::enModelUpAxisZ,
                static_cast<uint8_t>(std::size(ANIMATION_DATA))
            };
        } // namespace


        Miro::Miro()
            : m_parameter(ParamHolder::Get().GetParameter<MiroParameter>(EnParamID::Miro))
            , m_stateMachine(nullptr)
        {
            m_stateMachine = std::make_unique<MiroStateMachine>(this, m_parameter);
        }


        Miro::~Miro() = default;


        ControllerSlot& Miro::GetControllerSlot()
        {
            return m_stateMachine->GetControllerSlot();
        }


        float Miro::GetMaxHp() const
        {
            return m_parameter.maxHp;
        }


        void Miro::Start()
        {
            // モデル・アニメーションクリップのロードと、キャラクターコントローラーの初期化(現在のHPも最大HPで始まる)
            Init(MODEL_DATA, m_parameter.radius, m_parameter.height);

            // 最初のHPを通知
            InGameInfoHub::Get().NotifyPlayerHp(GetHp(), GetMaxHp());

            // パッドで操作する
            GetControllerSlot().Attach(std::make_unique<PlayerController>());
        }


        void Miro::Update()
        {
            // 入力の作成とステートの更新(入力はステートマシンが持つコントローラーから作る)
            m_stateMachine->Update();

            // モデルの更新
            CharacterBase::Update();

            // HPの通知(値が変わった時だけ、UIに伝わる。ホットリロードで最大HPが変わった時も含む)
            InGameInfoHub::Get().NotifyPlayerHp(GetHp(), GetMaxHp());
        }


        void Miro::Render(RenderContext& rc)
        {
            CharacterBase::Render(rc);
        }
    } // namespace actor
} // namespace app
