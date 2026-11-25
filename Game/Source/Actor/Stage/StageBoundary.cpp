/**
 * @file StageBoundary.cpp
 * @brief ステージの外周の見えない壁
 */
#include "stdafx.h"

#include "StageBoundary.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /** 壁のモデルのファイルパス */
            constexpr const char* MODEL_PATH = "Assets/modelData/stage/field/stage_wall.tkm";
            /** 影を受けるか(描画しないので受けない) */
            constexpr bool IS_RECEIVE_SHADOW = false;
            /** 影を落とすか(描画しないので落とさない) */
            constexpr bool IS_CAST_SHADOW = false;
        } // namespace


        StageBoundary::StageBoundary()
            : m_modelRender()
            , m_physicsStaticObject()
        {}


        StageBoundary::~StageBoundary() = default;


        void StageBoundary::Build()
        {
            // アニメーションは無い。Z-upのモデル
            m_modelRender.Init(MODEL_PATH, nullptr, 0, EnModelUpAxis::enModelUpAxisZ, IS_RECEIVE_SHADOW, IS_CAST_SHADOW);

            // StageField と同じく、動かさないので、ワールド行列はここで確定させる
            m_modelRender.SetTRS(Transform());
            m_modelRender.Update();

            m_physicsStaticObject.CreateFromModel(m_modelRender.GetModel(), m_modelRender.GetModel().GetWorldMatrix());
        }
    } // namespace actor
} // namespace app
