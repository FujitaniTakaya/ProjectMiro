/**
 * @file Miro.cpp
 * @brief 主人公のMiro
 */
#include "stdafx.h"

#include "Miro.h"

#include <cstdint>
#include <iterator>

#include "MiroParameter.h"
#include "MiroStateMachine.h"
#include "Source/Actor/Character/CharacterAnimationClipID.h"
#include "Source/Controller/PlayerController.h"
#include "Source/Core/Param/ParamHolder.h"
#include "Source/MoveBehavior/ControllerSlot.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /**
             * アニメーションデータ
             * 先頭は EnCharacterAnimationClipID の順(待機・歩き・走り)に並べること。共通ステートが番号で再生する。
             */
            const AnimationData ANIMATION_DATA[] = {
                { "Assets/animData/idle.tka", true },
                { "Assets/animData/walk.tka", true },
                { "Assets/animData/run.tka", true },
            };
            static_assert(std::size(ANIMATION_DATA) == CHARACTER_ANIMATION_CLIP_COUNT, "共通のアニメーションクリップの数と合わせること");

            /** モデルデータ */
            const ModelData MODEL_DATA = {
                "Assets/modelData/unityChan.tkm",
                ANIMATION_DATA,
                EnModelUpAxis::enModelUpAxisY,
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


        void Miro::Start()
        {
            // モデル・アニメーションクリップのロードと、キャラクターコントローラーの初期化
            Init(MODEL_DATA, m_parameter.radius, m_parameter.height);

            // パッドで操作する
            GetControllerSlot().Attach(std::make_unique<PlayerController>());
        }


        void Miro::Update()
        {
            // 入力の作成とステートの更新(入力はステートマシンが持つコントローラーから作る)
            m_stateMachine->Update();

            // モデルの更新
            CharacterBase::Update();
        }


        void Miro::Render(RenderContext& rc)
        {
            CharacterBase::Render(rc);
        }
    } // namespace actor
} // namespace app
