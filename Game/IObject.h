/**
 * @file IObject.h
 * @brief アプリ側のオブジェクトの基底クラス
 * @details NewGO/DeleteGOを使わずに、Applicationが自分で所有して更新・描画するためのインターフェース。
 */
#pragma once


namespace app
{
    /**
     * @brief アプリ側のオブジェクトの基底クラス
     */
    class IObject : public Noncopyable
    {
    public:
        IObject()
            : m_isActive(true)
            , m_isPause(false)
        {
        }

        virtual ~IObject() = default;


    protected:
        virtual void Start() = 0;
        virtual void Update() = 0;
        virtual void Render(RenderContext& renderContext) = 0;


        /** 下の関数を自分で呼んでください！ */
    public:
        void StartWrapper()
        {
            if (m_isActive)
            {
                Start();
            }
        }


        void UpdateWrapper()
        {
            if (m_isActive && !m_isPause)
            {
                Update();
            }
        }


        void RenderWrapper(RenderContext& renderContext)
        {
            if (m_isActive)
            {
                Render(renderContext);
            }
        }


        /**
         * @brief Activeフラグの設定
         * @param isActive Activeフラグ
         */
        void SetActive(const bool isActive)
        {
            m_isActive = isActive;
        }


        /**
         * @brief Pauseフラグの設定
         * @param isPause Pauseフラグ
         */
        void SetPause(const bool isPause)
        {
            m_isPause = isPause;
        }


    protected:
        /** Activeフラグ */
        bool m_isActive;
        /** Pauseフラグ */
        bool m_isPause;
    };
} // namespace app
