/**
 * @file StageField.cpp
 * @brief ステージのフィールド
 */
#include "stdafx.h"

#include "StageField.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /** モデルのファイルパス。テクスチャは同じフォルダに置く */
            constexpr const char* MODEL_PATH = "Assets/modelData/stage/field/stage.tkm";
            /** 影を受けるか */
            constexpr bool IS_RECEIVE_SHADOW = true;
            /** 影を落とすか */
            constexpr bool IS_CAST_SHADOW = false;
        } // namespace


        StageField::StageField() = default;


        StageField::~StageField() = default;


        void StageField::Start()
        {
            // アニメーションは無い。Z-upのモデル
            m_modelRender.Init(MODEL_PATH, nullptr, 0, EnModelUpAxis::enModelUpAxisZ, IS_RECEIVE_SHADOW, IS_CAST_SHADOW);

            // 動かさないので、ワールド行列はここで確定させる
            m_modelRender.SetTRS(m_transform);
            m_modelRender.Update();
        }


        void StageField::Update()
        {}


        void StageField::Render(RenderContext& rc)
        {
            m_modelRender.Draw(rc);
        }
    } // namespace actor
} // namespace app
