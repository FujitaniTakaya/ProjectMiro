/**
 * @file Core.cpp
 * @brief ステージのコア
 */
#include "stdafx.h"

#include "Core.h"


namespace app
{
    namespace actor
    {
        namespace
        {
            /** モデルのファイルパス。テクスチャ(Gemme_BaseColor.DDS・Gemme_NormalMap.DDS・Gemme_SpecularMap.DDS)は同じフォルダに置く */
            constexpr const char* MODEL_PATH = "Assets/modelData/stage/core/Gemme.tkm";
            /** 影を受けるか */
            constexpr bool IS_RECEIVE_SHADOW = true;
            /** 影を落とすか */
            constexpr bool IS_CAST_SHADOW = true;
            /** 拡大率の初期値。モデルがメートル単位なので、cm単位のワールドに合わせて100倍にする */
            constexpr float MODEL_SCALE = 100.0f;
        } // namespace


        Core::Core()
        {
            m_transform.m_scale = Vector3(MODEL_SCALE, MODEL_SCALE, MODEL_SCALE);
        }


        Core::~Core() = default;


        void Core::Start()
        {
            // アニメーションは無い。Z-upのモデル
            m_modelRender.Init(MODEL_PATH, nullptr, 0, EnModelUpAxis::enModelUpAxisZ, IS_RECEIVE_SHADOW, IS_CAST_SHADOW);

            // 初回の描画前にワールド行列を確定させる
            m_modelRender.SetTRS(m_transform);
            m_modelRender.Update();
        }


        void Core::Update()
        {
            m_modelRender.SetTRS(m_transform);
            m_modelRender.Update();
        }


        void Core::Render(RenderContext& rc)
        {
            m_modelRender.Draw(rc);
        }
    } // namespace actor
} // namespace app
