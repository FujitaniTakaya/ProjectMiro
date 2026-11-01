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
         * @details キャラクター共通の待機ステートと、Miro専用の歩き・走り(8方向)・構え・構えを解くステート(MiroIState.h)を登録して、入力から遷移先を決める。
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
            /**
             * @brief 構え中の移動速度を取得する
             * @details 構えながら動くアニメーションがないので、構えのポーズのまま滑るように動く。歩きより遅くする。
             * @return MiroParameter の構え中の移動速度(入力の倒し具合が最大のときの速さ)
             */
            float GetAimMoveSpeed() const;


        protected:
            /**
             * @brief 向きを決める
             * @details 入力にLook(カメラの方向)があれば、移動方向と関係なくその方向を向く。(入力がなくても向く)
             *          なければ、共通の動き(移動入力がある間だけ、移動方向を向く)。
             *          構え中(構えを解く動きの間も含む)は、その方向から画面の中央側(右)へ、MiroParameter の aimTurnDegrees だけ回す。
             *          肩越しのカメラは注視点が右にずれていて、カメラと平行に前を向くと銃が体の陰に入りやすいため。
             */
            virtual void UpdateRotation() override;

            /**
             * @brief ステートの変更先を取得する
             * @details 優先度の高い順に判定する。ZLを押していれば構え、ZLを離した直後なら構えを解く動き(終わるまで)、
             *          走りに切り替えられるなら走り、移動入力があれば歩き、どれにも当てはまらなければ待機。
             *          (忍び足の共通ステートはまだないので、歩きになる)
             * @return 変更先のステートポインタ
             */
            virtual IState* GetChangeState() override;

            /**
             * @brief 構えに切り替えられるかどうか
             * @return 構える(ZL)を押していれば true
             */
            inline bool CanChangeAimState() const
            {
                return GetInput().IsPress(EnInputButton::Aim);
            }
            /**
             * @brief 構えを解く動きに切り替えられるかどうか
             * @details 構えステートからZLを離した直後と、構えを解く動きが終わるまでの間。
             * @return 構えを解く動きに入る(または続ける)なら true
             */
            bool CanChangeAimEndState() const;

            /**
             * @brief 構え中かどうか
             * @return 構えステートか、構えを解くステートなら true
             */
            bool IsAiming() const;


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
