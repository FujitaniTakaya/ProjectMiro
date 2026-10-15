/**
 * @file Miro.h
 * @brief 主人公のMiro
 */
#pragma once
#include <memory>

#include "Source/Actor/Character/CharacterBase.h"


namespace app
{
    /** 前方宣言 */
    class ControllerSlot;


    namespace actor
    {
        /** 前方宣言 */
        struct MiroParameter;
        class MiroStateMachine;


        /**
         * @brief 主人公のMiro
         * @details 次の2つを持つ。
         *          - MiroStateMachine : 入力から状態(待機・歩き・走り)を決める。入力を作る ControllerSlot もここが持つ
         *          - MiroParameter : ParamHolderから取り出した参照。ホットリロードで値が変わる(最大HPもここ)
         *          Start() でパッド(PlayerController)を付ける。GetControllerSlot() で付け替えられる。
         *          Update() の順は、ステートマシン(入力の作成 → ステートの更新) → モデルの更新 → HPの通知。
         *          HP(現在のHPはCharacterBase、最大HPはMiroParameter)は、InGameInfoHub に通知する。UIはそこから受け取る。
         *          ゾンビも、同じ形(ZombieStateMachine・ZombieParameter・AIController)で作る。
         * @note 初期座標は、Start() の前に SetPosition() で設定しておくこと。(キャラクターコントローラーが現在の座標に作られる)
         */
        class Miro : public CharacterBase
        {
        public:
            /**
             * @brief コントローラーの入れ物を取得
             * @details ステートマシンが持つ入れ物を返す。Attach() / Detach() でコントローラーを付け替えられる。
             * @return コントローラーの入れ物
             */
            ControllerSlot& GetControllerSlot();

            /**
             * @brief 最大HPを取得
             * @return MiroParameter の最大HP
             */
            virtual float GetMaxHp() const override;


        public:
            Miro();
            virtual ~Miro() override;


        protected:
            /** 初期化処理 */
            virtual void Start() override;
            /** 更新処理 */
            virtual void Update() override;
            /** 描画処理 */
            virtual void Render(RenderContext& rc) override;


        private:
            /** パラメーター(ParamHolderが持つもの。アプリが終わるまで使える) */
            const MiroParameter& m_parameter;
            /** ステートマシン */
            std::unique_ptr<MiroStateMachine> m_stateMachine;
        };
    } // namespace actor
} // namespace app
