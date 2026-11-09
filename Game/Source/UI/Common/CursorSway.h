/**
 * @file CursorSway.h
 * @brief 選択カーソルの矢印を左右に揺らす
 * @details 揺れ幅と速さは、UIアニメーションの定義 "cursorSway" (Assets/parameter/ui/Animation.json) で決まる。
 *          矢印のx座標が、揺れの中心になる。Menuが矢印のy座標を書き換えても、揺れには影響しない。
 *          例: AttachCursorSway(m_cursor);   // MenuBase::InitializeLogic() の中で、矢印を取り出した後に呼ぶ
 */
#pragma once
#include "Source/UI/Foundation/UIParts.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief 矢印に、左右に揺れるアニメーションを付けて、再生する
         * @details 付けた時のx座標を中心にして、x座標だけを動かす。
         *          矢印が作り直された時(Layoutがjsonの更新でパーツを作り直した時)は、アニメーションも一緒に破棄されるので、
         *          MenuBase::InitializeLogic() の度に呼び直すこと。
         *          定義が無い場合は、揺らさない。
         * @param cursor 矢印のUI。nullptrの場合は何もしない。
         */
        void AttachCursorSway(UIBase* cursor);
    } // namespace ui
} // namespace app
