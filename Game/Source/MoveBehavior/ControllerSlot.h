/**
 * @file ControllerSlot.h
 * @brief Controllerを付け替えられるようにする入れ物
 * @details キャラクターが1つメンバとして持つ。キャラクターは毎フレーム Update() を呼び、GetInput() の入力だけを読む。
 *          例: m_controllers.Attach(std::make_unique<PlayerController>());
 *              ...
 *              ControllerContext context;
 *              context.position = GetPosition();
 *              context.forward = GetForward();
 *              const CharacterInput& input = m_controllers.Update(context);
 */
#pragma once
#include "IController.h"


namespace app
{
    /**
     * @brief Controllerを付け替えられるようにする入れ物
     */
    class ControllerSlot : public Noncopyable
    {
    public:
        ControllerSlot();
        ~ControllerSlot();


        /**
         * @brief Controllerを付ける
         * @param controller 付けるController。nullptr なら外すだけ
         * @details 既に付いているControllerがあれば、OnDetach() を呼んで破棄する。
         *          入力は無入力に戻り、新しいControllerの OnAttach() が呼ばれる。
         * @note Update() の最中(Controllerの中)から呼んではいけない
         */
        void Attach(std::unique_ptr<IController> controller);

        /**
         * @brief Controllerを外して、所有権を返す
         * @return 外したController。付いていなければ nullptr
         * @details OnDetach() を呼ぶ。返ったControllerは別のキャラクターの Attach() に渡して付け替えられる。
         * @note Update() の最中(Controllerの中)から呼んではいけない
         */
        std::unique_ptr<IController> Detach();

        /**
         * @brief Controllerを更新して、今フレームの入力を作る
         * @param context 操作対象の情報。deltaTime はここで埋める
         * @return 今フレームの入力。Controllerが付いていなければ無入力
         * @details Triggerは「今フレームで押している かつ 前フレームで押していない」で計算する。
         */
        const CharacterInput& Update(const ControllerContext& context);

        /**
         * @brief 今フレームの入力を取得する
         * @return Update() で作った入力
         */
        const CharacterInput& GetInput() const
        {
            return m_input;
        }

        /**
         * @brief Controllerが付いているかどうか
         * @return 付いていれば true
         */
        bool HasController() const
        {
            return m_controller != nullptr;
        }


    private:
        /** 付いているController */
        std::unique_ptr<IController> m_controller;
        /** 今フレームの入力 */
        CharacterInput m_input;
        /** Controllerの Update() を実行中かどうか(実行中の付け替えを防ぐ) */
        bool m_isUpdating;
    };
}
