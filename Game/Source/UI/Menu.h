/**
 * @file Menu.h
 * @brief UIのパーツを動かす処理の基底クラス
 * @details Layoutが、jsonからUIのパーツを作って、このクラスに登録する。
 *          MenuBaseを継承したクラスで、InitializeLogic()の中でUIのパーツを取り出して、ボタンを押した時の処理などを書く。
 */
#pragma once
#include "UIParts.h"

#include <unordered_map>


namespace app
{
    namespace ui
    {
        /**
         * @brief UIのパーツを動かす処理の基底クラス
         */
        class MenuBase : public Noncopyable
        {
        public:
            MenuBase()
                : m_canvas(nullptr)
                , m_uiMap()
            {}

            virtual ~MenuBase() = default;


        public:
            /** 更新 */
            virtual void Update()
            {
                if (m_canvas)
                {
                    m_canvas->Update();
                }
            }

            /**
             * @brief 描画
             * @param rc レンダーコンテキスト
             */
            virtual void Render(RenderContext& rc)
            {
                if (m_canvas)
                {
                    m_canvas->Render(rc);
                }
            }

            /**
             * @brief UIのパーツが作られた(作り直された)時の処理
             * @details Layoutが、jsonからUIのパーツを作る度に呼ぶ。
             *          ここで、GetUI()でUIのパーツを取り出して、ボタンを押した時の処理などを書く。
             *          UIのパーツは作り直されるので、取り出したポインタは、次に呼ばれた時に取り直すこと。
             */
            virtual void InitializeLogic() {}


        public:
            /**
             * @brief キャンバスを設定
             * @param canvas キャンバス。Menuが所有する。
             */
            void SetCanvas(std::unique_ptr<UICanvas> canvas)
            {
                m_canvas = std::move(canvas);
            }

            /**
             * @brief キャンバスを取得
             * @return キャンバス。設定されていない場合はnullptr。
             */
            UICanvas* GetCanvas() const
            {
                return m_canvas.get();
            }


        public:
            /**
             * @brief UIのパーツを登録
             * @details すでに同じキーが登録されている場合は、上書きする。
             * @param key キー(UIの名前のハッシュ値)
             * @param ui UIのパーツ。所有はキャンバス。
             */
            void RegisterUI(const uint32_t key, UIBase* ui)
            {
                m_uiMap[key] = ui;
            }

            /**
             * @brief UIのパーツの登録を解除
             * @param key キー(UIの名前のハッシュ値)
             */
            void UnregisterUI(const uint32_t key)
            {
                m_uiMap.erase(key);
            }

            /**
             * @brief UIのパーツを取得
             * @tparam T 取得するUIのパーツの型
             * @param key キー(UIの名前のハッシュ値)
             * @return UIのパーツ。登録されていない場合や、型が違う場合はnullptr。
             */
            template <typename T>
            T* GetUI(const uint32_t key) const
            {
                const auto it = m_uiMap.find(key);
                return (it != m_uiMap.end()) ? dynamic_cast<T*>(it->second) : nullptr;
            }

            /**
             * @brief UIのパーツが登録されているかどうか
             * @param key キー(UIの名前のハッシュ値)
             * @return 登録されていればtrue
             */
            bool HasUI(const uint32_t key) const
            {
                return m_uiMap.count(key) > 0;
            }

            /** UIのパーツの登録を、全て解除 */
            void Clear()
            {
                m_uiMap.clear();
            }


        protected:
            /** キャンバス */
            std::unique_ptr<UICanvas> m_canvas;
            /** 登録されているUIのパーツ。(所有はキャンバス) */
            std::unordered_map<uint32_t, UIBase*> m_uiMap;
        };
    } // namespace ui
} // namespace app
