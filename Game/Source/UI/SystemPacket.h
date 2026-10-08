/**
 * @file SystemPacket.h
 * @brief LayoutとMenuをまとめて扱うクラス
 */
#pragma once
#include "Source/UI/Layout.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief LayoutとMenuをまとめて扱うクラス
         * @details Layoutを作って、Layoutが所有するMenuを取り出せるようにする。
         * @tparam TMenu Menuの型(MenuBaseを継承していること)
         */
        template <typename TMenu>
        class SystemPacket : public Noncopyable
        {
        public:
            SystemPacket()
                : m_layout(nullptr)
                , m_menu(nullptr)
            {}

            // NOTE: MenuはLayoutが所有しているので、ここで破棄する必要は無い。
            ~SystemPacket() = default;


        public:
            /**
             * @brief 初期化
             * @details Layoutを作り直して、jsonからUIのパーツを作る。
             * @param jsonFilePath Layoutのjsonファイルのパス
             */
            void Initialize(const char* jsonFilePath)
            {
                m_menu = nullptr;
                m_layout = std::make_unique<Layout>();
                m_layout->Initialize<TMenu>(jsonFilePath);
                m_menu = m_layout->GetMenu<TMenu>();
            }

            /** 更新 */
            void Update()
            {
                if (m_layout)
                {
                    m_layout->Update();
                }
            }

            /**
             * @brief 描画
             * @param rc レンダーコンテキスト
             */
            void Render(RenderContext& rc)
            {
                if (m_layout)
                {
                    m_layout->Render(rc);
                }
            }


        public:
            /**
             * @brief Layoutを取得
             * @return Layout。初期化されていない場合はnullptr。
             */
            Layout* GetLayout() const
            {
                return m_layout.get();
            }

            /**
             * @brief Menuを取得
             * @return Menu。初期化されていない場合はnullptr。
             */
            TMenu* GetMenu() const
            {
                return m_menu;
            }


        private:
            /** Layout */
            std::unique_ptr<Layout> m_layout;
            /** Menu。(所有はLayout) */
            TMenu* m_menu;
        };


        /**
         * @brief SystemPacketのユニークポインタ
         */
        template <typename TMenu>
        using UIPacket = std::unique_ptr<SystemPacket<TMenu>>;


        /**
         * @brief SystemPacketを作って、初期化する
         * @param packet 作ったSystemPacketを受け取るユニークポインタ
         * @param path Layoutのjsonファイルのパス
         */
        template <typename TMenu>
        void InitUIPacket(UIPacket<TMenu>& packet, const char* path)
        {
            packet = std::make_unique<SystemPacket<TMenu>>();
            packet->Initialize(path);
        }
    } // namespace ui
} // namespace app
