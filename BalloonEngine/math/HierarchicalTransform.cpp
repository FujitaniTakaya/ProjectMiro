/**
 * @file HierarchicalTransform.cpp
 * @brief 親子関係を持つ変換情報を保持するクラスの実装
 */
#include "BalloonEnginePreCompile.h"

#include "HierarchicalTransform.h"


namespace nsBalloonEngine
{
    // NOTE: m_worldMatrixは、Matrixのコンストラクタで単位行列になる。
    //       Matrix::Identityはstatic変数なので、初期化の順番に影響されないよう、使っていない。
    HierarchicalTransform::HierarchicalTransform()
        : m_localTransform()
        , m_worldTransform()
        , m_worldMatrix()
        , m_parent(nullptr)
        , m_children()
    {}


    HierarchicalTransform::~HierarchicalTransform()
    {
        // 親の子のリストに、破棄された自分が残らないようにする。
        if (m_parent)
        {
            m_parent->RemoveChild(this);
        }
        // 子の親に、破棄された自分が残らないようにする。
        RemoveAllChildren();
    }


    void HierarchicalTransform::UpdateTransform()
    {
        if (m_parent)
        {
            // ローカルの座標(親からのずれ)を行列にして、親のワールド行列と掛け合わせる。
            Matrix localTranslation;
            localTranslation.MakeTranslation(m_localTransform.m_position);

            Matrix worldTranslation;
            worldTranslation.Multiply(localTranslation, m_parent->m_worldMatrix);

            // 平行移動成分を、ワールドの座標にする。
            m_worldTransform.m_position.x = worldTranslation.m[3][0];
            m_worldTransform.m_position.y = worldTranslation.m[3][1];
            m_worldTransform.m_position.z = worldTranslation.m[3][2];

            // 拡大は、親の拡大と掛け合わせる。
            m_worldTransform.m_scale.x = m_localTransform.m_scale.x * m_parent->m_worldTransform.m_scale.x;
            m_worldTransform.m_scale.y = m_localTransform.m_scale.y * m_parent->m_worldTransform.m_scale.y;
            m_worldTransform.m_scale.z = m_localTransform.m_scale.z * m_parent->m_worldTransform.m_scale.z;

            // 回転は、親の回転と掛け合わせる。
            m_worldTransform.m_rotation = m_parent->m_worldTransform.m_rotation * m_localTransform.m_rotation;
        }
        else
        {
            // 親がいない場合は、ローカルの値をそのままコピーする。
            m_worldTransform.m_position = m_localTransform.m_position;
            m_worldTransform.m_scale = m_localTransform.m_scale;
            m_worldTransform.m_rotation = m_localTransform.m_rotation;
        }

        UpdateWorldMatrix();
    }


    void HierarchicalTransform::UpdateWorldMatrix()
    {
        // 拡大 → 回転 → 平行移動の順で掛け合わせる。
        Matrix scaling;
        scaling.MakeScaling(m_worldTransform.m_scale);

        Matrix rotation;
        rotation.MakeRotationFromQuaternion(m_worldTransform.m_rotation);

        Matrix translation;
        translation.MakeTranslation(m_worldTransform.m_position);

        Matrix scalingRotation;
        scalingRotation.Multiply(scaling, rotation);
        m_worldMatrix.Multiply(scalingRotation, translation);

        // 子のワールドの変換情報も、新しいワールド行列で更新する。
        for (HierarchicalTransform* child : m_children)
        {
            child->UpdateTransform();
        }
    }


    //=======================================================================
    // 親子関係
    //=======================================================================
    void HierarchicalTransform::SetParent(HierarchicalTransform* parent)
    {
        // すでに親がいる場合は、先に外す。
        RemoveParent();

        m_parent = parent;

        // 新しい親の、子のリストに自分を追加する。
        if (m_parent)
        {
            m_parent->AddChild(this);
        }
    }


    void HierarchicalTransform::RemoveParent()
    {
        // 親の、子のリストから自分を外す。(RemoveChild()が、自分の親も外す。)
        if (m_parent)
        {
            m_parent->RemoveChild(this);
            m_parent = nullptr;
        }
    }


    void HierarchicalTransform::AddChild(HierarchicalTransform* child)
    {
        if (HasChild(child))
        {
            return;
        }
        m_children.push_back(child);
    }


    void HierarchicalTransform::RemoveChild(HierarchicalTransform* child)
    {
        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it == m_children.end())
        {
            return;
        }

        // 子の親が自分の場合は、子の親を外す。(AddChild()だけで追加された子は、別の親を持っていることがある。)
        if ((*it)->m_parent == this)
        {
            (*it)->m_parent = nullptr;
        }
        m_children.erase(it);
    }


    void HierarchicalTransform::RemoveAllChildren()
    {
        // 子の親が自分の場合は、子の親を外してから、子のリストを空にする。
        for (HierarchicalTransform* child : m_children)
        {
            if (child && child->m_parent == this)
            {
                child->m_parent = nullptr;
            }
        }
        m_children.clear();
    }


    bool HierarchicalTransform::HasChild(HierarchicalTransform* child) const
    {
        return std::find(m_children.begin(), m_children.end(), child) != m_children.end();
    }
} // namespace nsBalloonEngine
