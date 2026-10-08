/**
 * @file HierarchicalTransform.h
 * @brief 親子関係を持つ変換情報を保持するクラスの宣言
 */
#pragma once
#include "math/Transform.h"


namespace nsBalloonEngine
{
    /**
     * @brief 親子関係を持つ変換情報を保持するクラス
     * @details m_localTransformに値を設定してUpdateTransform()を呼ぶと、
     *          親の変換情報を掛け合わせたm_worldTransformが求まる。
     *          親を設定する時はSetParent()、外す時はRemoveParent()を使う。
     *          親・子のどちらが先に破棄されても、破棄される側が相手のリストから外れる。
     *          コピーすると親子の参照が壊れるので、コピーは禁止している。
     * @note    親子が循環するような親は設定しないこと。(UpdateTransform()が終わらなくなる。)
     */
    class HierarchicalTransform : public Noncopyable
    {
    public:
        HierarchicalTransform();
        ~HierarchicalTransform();


    public:
        /**
         * @brief 変換情報を更新
         * @details 親がいる場合は、親のワールドの変換情報とm_localTransformを掛け合わせて、m_worldTransformを更新する。
         *          親がいない場合は、m_localTransformをそのままm_worldTransformにコピーする。
         *          更新したら、子のUpdateTransform()も呼ぶ。
         */
        void UpdateTransform();


        //=======================================================================
        // 親子関係
        //=======================================================================
    public:
        /**
         * @brief 親を設定
         * @details すでに親がいる場合は、先に外してから設定する。
         *          nullptrを渡すと、親を外す。
         * @param parent 親
         */
        void SetParent(HierarchicalTransform* parent);

        /**
         * @brief 親を外す
         * @details 親がいない場合は何もしない。
         */
        void RemoveParent();

        /**
         * @brief 親がいるかどうか
         * @return 親がいればtrue
         */
        bool HasParent() const
        {
            return m_parent != nullptr;
        }

        /**
         * @brief 子を追加
         * @details 子のリストに追加するだけで、子の親は設定しない。
         *          すでに追加されている子は、追加しない。
         * @note    通常は、子の側でSetParent()を使うこと。
         * @param child 子
         */
        void AddChild(HierarchicalTransform* child);

        /**
         * @brief 子を外す
         * @details 子のリストから外し、子の親も外す。
         *          リストにいない子は、何もしない。
         * @note    通常は、子の側でRemoveParent()を使うこと。
         * @param child 子
         */
        void RemoveChild(HierarchicalTransform* child);

        /**
         * @brief 全ての子を外す
         * @details 全ての子の親も外す。
         */
        void RemoveAllChildren();

        /**
         * @brief 指定した子がいるかどうか
         * @param child 子
         * @return 子のリストにいればtrue
         */
        bool HasChild(HierarchicalTransform* child) const;


    public:
        /** 親を考慮しない、自身の変換情報 */
        Transform m_localTransform;
        /** 親の変換情報を考慮した、ワールドの変換情報。UpdateTransform()で更新される。 */
        Transform m_worldTransform;


    private:
        /**
         * @brief ワールド行列を更新
         * @details m_worldTransformからワールド行列を作り、子のUpdateTransform()も呼ぶ。
         *          UpdateTransform()から呼ばれるので、直接呼ぶ必要はない。
         */
        void UpdateWorldMatrix();


    private:
        /** 親を考慮したワールド行列 */
        Matrix m_worldMatrix;
        /** 親 */
        HierarchicalTransform* m_parent;
        /** 子 */
        std::vector<HierarchicalTransform*> m_children;
    };
} // namespace nsBalloonEngine
