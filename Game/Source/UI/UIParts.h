/**
 * @file UIParts.h
 * @brief UIのパーツ群
 * @details UIBaseを継承したパーツを、UICanvasに登録して使う。
 *          パーツの座標・拡大・回転は、m_transform.m_localTransformに設定する。
 *          描画には、親(UICanvas)を考慮したm_transform.m_worldTransformが使われる。
 *          パーツにはUIAnimationを登録できる。登録したアニメーションは、パーツのUpdate()で更新される。
 */
#pragma once
#include "UISprite.h"
#include "Animation/UIAnimation.h"

#include <algorithm>
#include <memory>
#include <vector>


namespace app
{
    namespace ui
    {
        /**
         * @brief UIの基底クラス
         */
        class UIBase : public Noncopyable
        {
        public:
            UIBase();
            virtual ~UIBase();


        public:
            /** 更新 */
            virtual void Update() = 0;

            /**
             * @brief 描画
             * @param rc レンダーコンテキスト
             */
            virtual void Render(RenderContext& rc) = 0;


        public:
            /**
             * @brief 描画するかどうか
             * @return 描画するならtrue
             */
            bool IsDraw() const
            {
                return m_isDraw;
            }

            /**
             * @brief 描画するかどうかを設定
             * @param isDraw 描画するかどうか
             */
            void SetIsDraw(const bool isDraw)
            {
                m_isDraw = isDraw;
            }

            /**
             * @brief キーを設定
             * @param key キー(UIの名前のハッシュ値)
             */
            void SetKey(const uint32_t key)
            {
                m_key = key;
            }

            /**
             * @brief キーを取得
             * @return キー(UIの名前のハッシュ値)
             */
            uint32_t GetKey() const
            {
                return m_key;
            }


        public:
            /**
             * @brief 登録されている全てのアニメーションを更新
             * @details 再生中のアニメーションだけが、値を反映する。UIのUpdate()の先頭(トランスフォームの更新の前)で呼ぶ。
             *          更新中は、AddAnimation() RemoveAnimation()を呼ばないこと。
             */
            void UpdateAnimation()
            {
                for (AnimationEntry& entry : m_animations)
                {
                    entry.m_animation->Update();
                }
            }

            /** 登録されている全てのアニメーションを再生 */
            void PlayAnimation()
            {
                for (AnimationEntry& entry : m_animations)
                {
                    entry.m_animation->PlayAnimation();
                }
            }

            /** 登録されている全てのアニメーションを停止 */
            void StopAnimation()
            {
                for (AnimationEntry& entry : m_animations)
                {
                    entry.m_animation->StopAnimation();
                }
            }

            /**
             * @brief 再生中のアニメーションがあるかどうか
             * @return 1つでも再生中ならtrue
             */
            bool IsPlayAnimation() const
            {
                for (const AnimationEntry& entry : m_animations)
                {
                    if (entry.m_animation->IsPlayAnimation())
                    {
                        return true;
                    }
                }
                return false;
            }

            /**
             * @brief アニメーションを登録
             * @details すでに同じキーのアニメーションがある場合は、置き換える。アニメーションはUIが所有する。
             * @param key キー
             * @param animation アニメーション
             */
            void AddAnimation(const uint32_t key, std::unique_ptr<UIAnimationBase> animation);

            /**
             * @brief アニメーションの登録を解除して、破棄する
             * @details キーのアニメーションが無い場合は、何もしない。
             * @param key キー
             */
            void RemoveAnimation(const uint32_t key);

            /**
             * @brief アニメーションを探す
             * @param key キー
             * @return 見つかったアニメーション。無い場合はnullptr。
             */
            UIAnimationBase* FindAnimation(const uint32_t key) const
            {
                for (const AnimationEntry& entry : m_animations)
                {
                    if (entry.m_key == key)
                    {
                        return entry.m_animation.get();
                    }
                }
                return nullptr;
            }

            /**
             * @brief アニメーションを探す
             * @tparam T 取得するアニメーションの型
             * @param key キー
             * @return 見つかったアニメーション。無い場合や、型が違う場合はnullptr。
             */
            template <typename T>
            T* FindAnimation(const uint32_t key) const
            {
                return dynamic_cast<T*>(FindAnimation(key));
            }

            /**
             * @brief 登録されている全てのアニメーションに、関数を呼ぶ
             * @tparam F UIAnimationBase*を受け取る関数
             * @param func 関数
             */
            template <typename F>
            void ForEachAnimation(F&& func)
            {
                for (AnimationEntry& entry : m_animations)
                {
                    func(entry.m_animation.get());
                }
            }


        public:
            /** トランスフォーム。m_localTransformに値を設定する。 */
            HierarchicalTransform m_transform;
            /** 色 */
            Vector4 m_color;
            /** 基点。(0, 0)が左下、(1, 1)が右上。 */
            Vector2 m_pivot;
            /** 描画するかどうか */
            bool m_isDraw;


        private:
            /**
             * @brief 登録されたアニメーション
             */
            struct AnimationEntry
            {
                /**
                 * @brief コンストラクタ
                 * @param key キー
                 * @param animation アニメーション
                 */
                AnimationEntry(const uint32_t key, std::unique_ptr<UIAnimationBase> animation)
                    : m_key(key)
                    , m_animation(std::move(animation))
                {
                }

                /** キー */
                uint32_t m_key;
                /** アニメーション */
                std::unique_ptr<UIAnimationBase> m_animation;
            };


        private:
            /** キー(UIの名前のハッシュ値) */
            uint32_t m_key;
            /** 登録されたアニメーション。1つのUIに登録される数は少ないので、配列を線形に探す。 */
            std::vector<AnimationEntry> m_animations;
        };




        /**
         * @brief 画像1枚を描画するUIの基底クラス
         */
        class UIImage : public UIBase
        {
        public:
            UIImage();
            ~UIImage() override;


        public:
            /** 更新 */
            void Update() override;

            /**
             * @brief 描画
             * @param rc レンダーコンテキスト
             */
            void Render(RenderContext& rc) override;


        public:
            /**
             * @brief 基点を設定
             * @param pivot 基点
             */
            void SetPivot(const Vector2& pivot)
            {
                m_pivot = pivot;
            }


        protected:
            /**
             * @brief 画像を読み込んで、トランスフォームと色を設定する
             * @param assetName 画像(DDS)のファイルパス
             * @param width 幅
             * @param height 高さ
             * @param position 座標
             * @param scale 拡大率
             * @param rotation 回転
             * @param color 色
             * @param pivot 基点
             */
            void InitializeImage(
                const char* assetName,
                const float width,
                const float height,
                const Vector3& position,
                const Vector3& scale,
                const Quaternion& rotation,
                const Vector4& color,
                const Vector2& pivot
            );


        protected:
            /** 画像 */
            UISprite m_sprite;
        };




        /**
         * @brief アイコン
         */
        class UIIcon : public UIImage
        {
        public:
            UIIcon();
            ~UIIcon() override;


        public:
            /**
             * @brief 初期化
             * @param assetName 画像(DDS)のファイルパス
             * @param width 幅
             * @param height 高さ
             * @param position 座標
             * @param scale 拡大率
             * @param rotation 回転
             * @param color 色
             */
            void Initialize(
                const char* assetName,
                const float width,
                const float height,
                const Vector3& position,
                const Vector3& scale,
                const Quaternion& rotation,
                const Vector4& color
            );

            /**
             * @brief 初期化時の画像のサイズ(拡大率を掛ける前)を取得
             * @return 幅(x)と高さ(y)
             */
            const Vector2& GetSize() const
            {
                return m_size;
            }


        private:
            /** 初期化時の画像のサイズ(拡大率を掛ける前) */
            Vector2 m_size;
        };




        /**
         * @brief ボタン
         */
        class UIButton : public UIImage
        {
        public:
            UIButton();
            ~UIButton() override;


        public:
            /**
             * @brief 初期化
             * @param assetName 画像(DDS)のファイルパス
             * @param width 幅
             * @param height 高さ
             * @param position 座標
             * @param scale 拡大率
             * @param rotation 回転
             * @param color 色
             */
            void Initialize(
                const char* assetName,
                const float width,
                const float height,
                const Vector3& position,
                const Vector3& scale,
                const Quaternion& rotation,
                const Vector4& color
            );
        };




        /**
         * @brief ゲージ
         * @details 基点を指定できる画像。基点を端にして、拡大率を変えるとゲージとして使える。
         */
        class UIGauge : public UIImage
        {
        public:
            UIGauge();
            ~UIGauge() override;


        public:
            /**
             * @brief 初期化
             * @param assetName 画像(DDS)のファイルパス
             * @param width 幅
             * @param height 高さ
             * @param position 座標
             * @param scale 拡大率
             * @param rotation 回転
             * @param color 色
             * @param pivot 基点
             */
            void Initialize(
                const char* assetName,
                const float width,
                const float height,
                const Vector3& position,
                const Vector3& scale,
                const Quaternion& rotation,
                const Vector4& color,
                const Vector2& pivot
            );
        };




        /**
         * @brief 何も描画しないUI
         * @details トランスフォームだけを持つ。親として使う。
         */
        class UIDummy : public UIBase
        {
        public:
            UIDummy();
            ~UIDummy() override;


        public:
            /** 更新 */
            void Update() override;

            /**
             * @brief 描画
             * @details 何も描画しない。
             * @param rc レンダーコンテキスト
             */
            void Render(RenderContext& rc) override;
        };




        /**
         * @brief UIをまとめるキャンバス
         * @details CreateUI()で作ったUIは、キャンバスが所有する。
         *          UIのトランスフォームの親は、キャンバスのトランスフォームになる。
         *          キャンバスのm_transform.m_localTransformを動かすと、全てのUIが追従する。
         */
        class UICanvas : public UIBase
        {
        public:
            UICanvas();
            ~UICanvas() override;


        public:
            /** 更新。キャンバスのトランスフォームを更新した後に、全てのUIを更新する。 */
            void Update() override;

            /**
             * @brief 描画。全てのUIを、作った順に描画する。
             * @param rc レンダーコンテキスト
             */
            void Render(RenderContext& rc) override;


        public:
            /**
             * @brief UIを作る
             * @tparam T 作るUIの型(UIBaseを継承していること)
             * @param key キー(UIの名前のハッシュ値)
             * @return 作ったUI。キャンバスが所有する。
             */
            template <typename T>
            T* CreateUI(const uint32_t key)
            {
                auto ui = std::make_unique<T>();
                ui->SetKey(key);
                ui->m_transform.SetParent(&m_transform);

                T* result = ui.get();
                m_uiList.push_back(std::move(ui));
                return result;
            }

            /**
             * @brief UIを破棄する
             * @details キーのUIが無い場合は、何もしない。
             * @param key キー(UIの名前のハッシュ値)
             */
            void RemoveUI(const uint32_t key);

            /**
             * @brief UIを探す
             * @tparam T 取得するUIの型
             * @param key キー(UIの名前のハッシュ値)
             * @return 見つかったUI。無い場合や、型が違う場合はnullptr。
             */
            template <typename T>
            T* FindUI(const uint32_t key) const
            {
                const auto it = std::find_if(
                    m_uiList.begin(),
                    m_uiList.end(),
                    [key](const std::unique_ptr<UIBase>& ui) { return ui->GetKey() == key; }
                );
                return (it != m_uiList.end()) ? dynamic_cast<T*>(it->get()) : nullptr;
            }


        private:
            /** キャンバスが所有するUI */
            std::vector<std::unique_ptr<UIBase>> m_uiList;
        };
    } // namespace ui
} // namespace app
