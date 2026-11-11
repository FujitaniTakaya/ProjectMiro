/**
 * @file MiroStateMachine.h
 * @brief Miroのステートマシン
 */
#pragma once
#include "MiroMoveDirection.h"
#include "MiroWeaponHandler.h"
#include "Source/Actor/Character/CharacterStateMachine.h"


namespace app
{
    namespace actor
    {
        /** 前方宣言 */
        struct MiroParameter;


        /**
         * @brief Miroのステートマシン
         * @details キャラクター共通の待機ステートと、Miro専用の歩き・走り(8方向)・構え・構えを解く・リロードのステート(MiroIState.h)を登録して、入力から遷移先を決める。
         *          入力は、CharacterStateMachine が持つ ControllerSlot から Update() で作る。(CharacterStateMachine を参照)
         *          歩き・走りの速度は、MiroParameter の値を返す。
         *          向きは、移動方向ではなくカメラの方向(入力のLook)にそろえる。
         *          構え・構えを解く・リロードの間は、上半身(Upper レイヤー)が銃の動き、下半身(Base レイヤー)が歩き・待機になる。
         *          銃を扱っていない間は、Update() が上半身のレイヤーを止める。待機・歩き・走りのステートは、Base レイヤーの全身アニメーションだけを再生する。
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
             * @details 構え・構えを解く・リロードの間の速度。歩きより遅くする。
             *          下半身は歩きクリップを再生するので、歩きとの速度の比だけ遅く再生して、足が滑らないようにする。(PlayLowerBodyAnimation())
             * @return MiroParameter の構え中の移動速度(入力の倒し具合が最大のときの速さ)
             */
            float GetAimMoveSpeed() const;
            /**
             * @brief 構え中かどうか
             * @return 構えステートか、構えを解くステートなら true
             */
            bool IsAiming() const;

            /**
             * @brief 武器の処理を設定する
             * @details ステートマシンは武器を知らない。撃つ・リロードに要る処理は、持ち主が渡す。
             *          構えステートが、ZRを押した瞬間に fire を呼ぶ。リロードステートに入る時に reload を呼ぶ。
             *          canReload・isEmpty は、リロードステートに入るかの判定で、毎フレーム呼ぶ。
             * @note 渡した処理が参照するものは、このステートマシンが生きている間、有効であること。
             * @param handler 武器の処理
             */
            void SetWeaponHandler(MiroWeaponHandler handler);
            /**
             * @brief 1発撃つ
             * @return 撃てたら true。撃つ処理が設定されていない時と、撃てない時は false
             */
            bool TryFire() const;
            /**
             * @brief リロードできるか
             * @return 武器がリロードできる状態なら true。処理が設定されていない時は false
             */
            bool CanReload() const;
            /**
             * @brief マガジンが空か
             * @return マガジンが空なら true。処理が設定されていない時は false
             */
            bool IsMagazineEmpty() const;
            /**
             * @brief 武器のリロードを始める
             * @details 処理が設定されていない時は何もしない。
             */
            void StartReload() const;

            /**
             * @brief 下半身(Base レイヤー)のアニメーションを再生する
             * @details 構え・構えを解く・リロードのステートが、Enter() と毎フレームの Update() で呼ぶ。上半身(Upper レイヤー)は、そのステートが銃の動きを再生している。
             *          移動入力があれば、向いている方向から見た8方向の歩きクリップを、速度の比(aimMoveSpeed / walkSpeed)で遅くして再生する。
             *          構え中は歩きより遅く動くので、等速で再生すると足が地面を滑って見える。入力が無ければ、待機を通常の速さで再生する。
             *          同じクリップの再生は何もしないので、毎フレーム呼んでよい。方向が変わった時だけ、標準のブレンドで切り替わる。
             */
            void PlayLowerBodyAnimation();
            /**
             * @brief 向いている方向から見た、移動方向を求める
             * @details 向いている方向は、入力の Look。無ければ、今の体の向き。
             * @return 8方向のどれか
             */
            EnMiroMoveDirection CalcMoveDirection() const;
            /**
             * @brief 上半身アニメーションのブレンド時間を取得する
             * @return MiroParameter の上半身アニメーションのブレンド時間(秒)
             */
            float GetUpperBodyBlendTime() const;

            /**
             * @brief 更新
             * @details 基底の更新(入力の作成とステートの更新)の後に、銃を扱っていない間は上半身のレイヤーを止め、下半身の再生速度を元に戻す。
             */
            virtual void Update() override;


        protected:
            /**
             * @brief 向きを決める
             * @details 入力にLook(カメラの方向)があれば、移動方向と関係なくその方向を向く。(入力がなくても向く)
             *          なければ、共通の動き(移動入力がある間だけ、移動方向を向く)。
             *          構え中(構えを解く動きの間も含む)とリロード中は、その方向から画面の中央側(右)へ、MiroParameter の aimTurnDegrees だけ回す。
             *          肩越しのカメラは注視点が右にずれていて、カメラと平行に前を向くと銃が体の陰に入りやすいため。
             */
            virtual void UpdateRotation() override;

            /**
             * @brief ステートの変更先を取得する
             * @details 優先度の高い順に判定する。リロードに入る(または続ける)ならリロード、ZLを押していれば構え、
             *          ZLを離した直後なら構えを解く動き(終わるまで)、
             *          走りに切り替えられるなら走り、移動入力があれば歩き、どれにも当てはまらなければ待機。
             *          (忍び足の共通ステートはまだないので、歩きになる)
             * @return 変更先のステートポインタ
             */
            virtual IState* GetChangeState() override;

            /**
             * @brief 構えに切り替えられるかどうか
             * @return 構える(ZL。ActionLB2)を押していれば true
             */
            inline bool CanChangeAimState() const
            {
                return GetInput().IsPress(EnInputButton::ActionLB2);
            }
            /**
             * @brief 構えを解く動きに切り替えられるかどうか
             * @details 構えステートからZLを離した直後と、構えを解く動きが終わるまでの間。
             * @return 構えを解く動きに入る(または続ける)なら true
             */
            bool CanChangeAimEndState() const;

            /**
             * @brief リロードステートに切り替えられるかどうか
             * @details 次のどれかなら true。
             *          - リロードステートで、リロードのアニメーションが終わるまでの間(続ける)
             *          - 撃つ動きの最中ではなく、武器がリロードできる状態で、Xを押した瞬間か、マガジンが空になっている
             *          構える動きの間は、リロードに入れる。撃つ動きの最中は入れない。(撃つ動きが終わるまで待つ。最後の1発を撃った後は、その後で自動で入る)
             * @return リロードステートに入る(または続ける)なら true
             */
            bool CanChangeReloadState();

            /**
             * @brief 撃つ動きの最中かどうか
             * @return 構えステートで、撃つ動きの最中なら true
             */
            bool IsFiring();

            /**
             * @brief 銃が見えるように体を回す動きの最中かどうか
             * @return 構え中(構えを解く動きを含む)か、リロード中なら true
             */
            bool IsHandlingGun() const;


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
            /** 武器の処理。設定されていない時は、中身が空 */
            MiroWeaponHandler m_weaponHandler;
        };
    } // namespace actor
} // namespace app
