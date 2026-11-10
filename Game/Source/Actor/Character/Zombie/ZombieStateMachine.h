/**
 * @file ZombieStateMachine.h
 * @brief ゾンビのステートマシン
 */
#pragma once
#include "Source/Actor/Character/CharacterStateMachine.h"


namespace app
{
    namespace actor
    {
        /** 前方宣言 */
        struct MiroParameter;
        struct ZombieParameter;


        /**
         * @brief ゾンビのステートマシン
         * @details 待機・歩き・走り・攻撃(ZombieIState.h のゾンビ専用のステート)を登録して、入力から遷移先を決める。
         *          待機は、入るときに慣性を消すので、入力がなくなる(コアに着く)と、滑らずにその場で止まる。
         *          攻撃は、ActionA(攻撃ボタン)を押している間に入る。動きが終わるまでは、ボタンを離しても続ける。
         *          入力は、CharacterStateMachine が持つ ControllerSlot から Update() で作る。(CharacterStateMachine を参照)
         *          歩き・走りの速度は、プレイヤーの走り速度に ZombieParameter の割合をかけた値を返す。
         *          ロックオン専用のステートができるまでは、歩き=通常時、走り=ロックオン時として扱う。(走りは、入力のダッシュで切り替わる)
         *          ゾンビ専用のステート(ロックオン・死亡など)が要るようになったら、ここで追加する。
         * @note ZombieParameter・MiroParameter は ParamHolder が持つものの参照なので、ホットリロードで速度が変わる。
         */
        class ZombieStateMachine : public CharacterStateMachine
        {
        public:
            /**
             * @brief 歩きの移動速度を取得する
             * @return プレイヤーの走り速度 × ZombieParameter の歩きの割合
             */
            virtual float GetWalkSpeed() const override;
            /**
             * @brief 走りの移動速度を取得する
             * @return プレイヤーの走り速度 × ZombieParameter の走りの割合
             */
            virtual float GetRunSpeed() const override;


        protected:
            /**
             * @brief 攻撃に入る(続ける)かどうか
             * @return 攻撃の動きの途中、または ActionA(攻撃ボタン)を押していれば true
             */
            bool CanChangeAttackState() const;

            /**
             * @brief ステートの変更先を取得する
             * @details 優先度の高い順に判定する。攻撃に入る(続ける)なら攻撃、走りに切り替えられるなら走り、移動入力があれば歩き、どれにも当てはまらなければ待機。
             * @return 変更先のステートポインタ
             */
            virtual IState* GetChangeState() override;


        public:
            /**
             * @brief コンストラクタ
             * @param ownerCharacter 持ち主のキャラクター
             * @param parameter Zombieのパラメーター(ParamHolderが持つもの。アプリが終わるまで使える)
             */
            ZombieStateMachine(CharacterBase* ownerCharacter, const ZombieParameter& parameter);
            virtual ~ZombieStateMachine() override = default;


        private:
            /** パラメーター */
            const ZombieParameter& m_parameter;
            /** プレイヤー(Miro)のパラメーター。速度の基準 */
            const MiroParameter& m_playerParameter;
        };
    } // namespace actor
} // namespace app
