/**
 * @file CharacterBase.cpp
 * @brief キャラクターの基底クラス
 */
#include "stdafx.h"

#include "CharacterBase.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /** 影を受けるか */
            constexpr bool IS_RECEIVE_SHADOW = true;
            /** 影を落とすか */
            constexpr bool IS_CAST_SHADOW = true;
        } // namespace


        CharacterBase::CharacterBase()
            : m_animationClips(nullptr)
        {}


        CharacterBase::~CharacterBase() = default;


        void CharacterBase::Update()
        {
            // モデルの行列を更新
            // アニメーション進行とボーン行列の計算を含む
            m_modelRender.SetTRS(m_transform);
            m_modelRender.Update();
        }


        void CharacterBase::Render(RenderContext& rc)
        {
            m_modelRender.Draw(rc);
        }


        void CharacterBase::Init(const ModelData& data, const float radius, const float height)
        {
            // アニメーションクリップを同期ロード
            // NOTE: クリップが0個のときは配列を作らずnullptrを渡す。
            //       make_unique<AnimationClip[]>(0)は非nullを返し、ModelRender側でアニメーションありとして扱われてしまう。
            if (data.clipNum > 0)
            {
                m_animationClips = std::make_unique<AnimationClip[]>(data.clipNum);
                for (uint8_t i = 0; i < data.clipNum; ++i)
                {
                    m_animationClips[i].Load(data.animationData[i].fileName);
                    m_animationClips[i].SetLoopFlag(data.animationData[i].isLoop);
                }
            }

            // モデルを初期化
            m_modelRender.Init(
                data.fileName,
                m_animationClips.get(),
                data.clipNum,
                data.upAxis,
                IS_RECEIVE_SHADOW,
                IS_CAST_SHADOW
            );

            // 初回の描画前にワールド行列を確定させる
            m_modelRender.SetTRS(m_transform);
            m_modelRender.Update();

            // キャラクターコントローラーを初期化
            m_characterController.Init(radius, height, m_transform.m_position);
        }
    }
}
