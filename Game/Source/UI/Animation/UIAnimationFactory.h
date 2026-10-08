/**
 * @file UIAnimationFactory.h
 * @brief jsonの定義(UIAnimationParameter)から、UIアニメーションを作る
 * @details 事前に、UIAnimationParameter::Get().Load()で、定義のjsonを読み込んでおくこと。
 *          例: UIAnimationFactory::Attach<UIColorAnimation>(ui, Hash32("fadeIn"));
 *              ui->FindAnimation(Hash32("fadeIn"))->PlayAnimation();
 */
#pragma once
#include <cstdint>
#include <memory>
#include <utility>

#include "Source/UI/UIParts.h"
#include "UIAnimation.h"
#include "UIAnimationParameter.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief jsonの定義から、UIアニメーションを作るクラス
         */
        class UIAnimationFactory
        {
        public:
            /**
             * @brief アニメーションを作る
             * @details 作ったアニメーションは、jsonの定義に従う。jsonが更新されたら、次のPlayAnimation()で定義を取り直す。
             *          SetParameter()で値を設定すると、定義に従わなくなる。
             * @tparam T 作るアニメーションの型(UIValueAnimationを継承していること)
             * @param key 定義のキー(jsonの"key"をHash32した値)
             * @return 作ったアニメーション。定義が無い場合はnullptr。
             */
            template <typename T>
            static std::unique_ptr<T> Create(const uint32_t key)
            {
                const UIAnimationDef* def = UIAnimationParameter::Get().Find(key);
                if (!def)
                {
                    K2_LOG("UIアニメーションの定義がありません。key=%u\n", key);
                    return nullptr;
                }

                auto animation = std::make_unique<T>();
                animation->SetDefinition(*def);
                return animation;
            }

            /**
             * @brief アニメーションを作って、UIに登録する
             * @details UIにすでに同じキーのアニメーションがある場合は、置き換える。
             *          登録したキーで、UIBase::FindAnimation()から取り出せる。
             * @tparam T 作るアニメーションの型
             * @param target 登録するUI
             * @param key 定義のキー(jsonの"key"をHash32した値)。UIに登録するキーにもなる。
             * @return 登録できたらtrue。UIが無い場合や、定義が無い場合はfalse。
             */
            template <typename T>
            static bool Attach(UIBase* target, const uint32_t key)
            {
                if (!target)
                {
                    return false;
                }

                std::unique_ptr<T> animation = Create<T>(key);
                if (!animation)
                {
                    return false;
                }

                target->AddAnimation(key, std::move(animation));
                return true;
            }
        };
    } // namespace ui
} // namespace app
