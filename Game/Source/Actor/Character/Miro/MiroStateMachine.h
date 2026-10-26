/**
 * @file MiroStateMachine.h
 * @brief Miroのステートマシン
 */
#pragma once
#include "Source/Actor/Character/CharacterStateMachine.h"


namespace app
{
    namespace actor
    {
        /** 前方宣言 */
        struct MiroParameter;


        /**
         * @brief Miroのステートマシン
         * @details キャラクター共通の待機ステートと、Miro専用の歩き・走りステート(8方向。MiroIState.h)を登録して、入力から遷移先を決める。
         *          入力は、CharacterStateMachine が持つ ControllerSlot から Update() で作る。(CharacterStateMachine を参照)
         *          歩き・走りの速度は、MiroParameter の値を返す。
         *          向きは、移動方向ではなくカメラの方向(入力のLook)にそろえる。
         *          Miro専用のステートが要るようになったら、ここで追加する。
         * @note MiroParameter は ParamHolder が持つものの参照なので、ホットリロードで速度が変わる。
         */
        class MiroStateMachine : public CharacterStateMachine
        {
        public:
            /**
             * @brief 歩きの移動速度を取得する
             * @return MiroParameter の歩きの移動速度
             */
            virtual float GetWalkSpeed() const override;
            /**
             * @brief 走りの移動速度を取得する
             * @return MiroParameter の走りの移動速度
             */
            virtual float GetRunSpeed() const override;


        protected:
            /**
             * @brief 向きを決める
             * @details 入力にLook(カメラの方向)があれば、移動方向と関係なくその方向を向く。(入力がなくても向く)
             *          なければ、共通の動き(移動入力がある間だけ、移動方向を向く)。
             */
            virtual void UpdateRotation() override;

            /**
             * @brief ステートの変更先を取得する
             * @details 優先度の高い順に判定する。走りに切り替えられるなら走り、移動入力があれば歩き、どれにも当てはまらなければ待機。
             *          (忍び足の共通ステートはまだないので、歩きになる)
             * @return 変更先のステートポインタ
             */
            virtual IState* GetChangeState() override;


        public:
            /**
             * @brief コンストラクタ
             * @param ownerCharacter 持ち主のキャラクター
             * @param parameter Miroのパラメーター(ParamHolderが持つもの。アプリが終わるまで使える)
             */
            MiroStateMachine(CharacterBase* ownerCharacter, const MiroParameter& parameter);
            virtual ~MiroStateMachine() override = default;


        private:
            /** パラメーター */
            const MiroParameter& m_parameter;
        };
    } // namespace actor
} // namespace app
