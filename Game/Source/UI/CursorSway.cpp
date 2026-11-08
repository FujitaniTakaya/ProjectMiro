/**
 * @file CursorSway.cpp
 * @brief 選択カーソルの矢印を左右に揺らす
 */
#include "stdafx.h"

#include "CursorSway.h"

#include <cstdint>
#include <memory>
#include <utility>

#include "Source/Core/Util/CRC32.h"
#include "Source/UI/Animation/UIAnimationFactory.h"


namespace app
{
    namespace ui
    {
        namespace
        {
            /** UIアニメーションの定義のjson */
            constexpr const char* ANIMATION_JSON_PATH = "Assets/parameter/ui/Animation.json";
            /** 矢印を揺らすアニメーションの定義の名前(ハッシュ値)。UIに登録するキーにもなる。 */
            constexpr uint32_t SWAY_KEY = Hash32("cursorSway");
        } // namespace


        void AttachCursorSway(UIBase* cursor)
        {
            if (!cursor)
            {
                return;
            }

            // NOTE: すでに読み込んでいる場合は、何もしない。
            UIAnimationParameter::Get().Load(ANIMATION_JSON_PATH);

            std::unique_ptr<UIFloatAnimation> animation = UIAnimationFactory::Create<UIFloatAnimation>(SWAY_KEY);
            if (!animation)
            {
                return;
            }

            // NOTE: UITranslateAnimationは座標を丸ごと書いて、Menuが書くy座標を潰すので、x座標だけを書く関数を設定する。
            //       アニメーションは矢印自身が持つので、矢印へのポインタは、アニメーションが動く間は有効。
            const float centerX = cursor->m_transform.m_localTransform.m_position.x;
            animation->SetFunc([cursor, centerX](const float& offset) {
                cursor->m_transform.m_localTransform.m_position.x = centerX + offset;
            });
            animation->PlayAnimation();

            cursor->AddAnimation(SWAY_KEY, std::move(animation));
        }
    } // namespace ui
} // namespace app
