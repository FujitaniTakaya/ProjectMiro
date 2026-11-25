/**
 * @file PlayerCamera.h
 * @brief プレイヤーを追うカメラ(TPSの肩越しの視点)
 * @details BalloonEngineの GameCamera を元にしている。
 *          注視点は、ターゲットの右上(カメラの右方向 + 上方向にずらした位置)。
 *          視点は、注視点から「注視点→視点ベクトル」だけ離れた位置。右スティックでそのベクトルを回す。
 *            右スティック左右 : 注視点を中心に、Y軸回りに回す
 *            右スティック上下 : 注視点を中心に、上下に回す(角度の制限あり)
 *          キーボードでは、矢印キーが右スティックになる。
 *          回る速さは、PlayerCameraParameter の rotateSpeed に、オプションの感度(OptionSettings)を掛けたもの。
 *          通常はカメラ感度、ハンドガンを構えている間(SetAiming(true))はエイム感度を掛ける。
 *          構えている間は、視点が注視点に近づく。(距離の倍率と切り替えの時間は PlayerCameraParameter)
 *          視点がステージの壁・地面にめり込む時は、注視点から視点へ球を動かして、当たる手前まで視点を引き寄せる。(戻る時だけゆっくり)
 *          構えている間は、右スティックの倒し具合が回る速さに効く。(aimStickExponent。キーボードは常に全力)
 *          さらに、SetAimTargets() で渡された狙う点が視線の近くにあると、回転が遅くなり、スティックを倒している間は視線が狙う点へ寄る。(エイムアシスト。AimAssist)
 *          撃った時の反動(AddRecoil)で、視線が上へ跳ね、すぐ元へ戻る。
 */
#pragma once
#include <vector>

#include "AimAssist.h"


namespace app
{
    namespace camera
    {
        /** 前方宣言 */
        struct PlayerCameraParameter;


        /**
         * @brief プレイヤーを追うカメラ
         * @details g_camera3D を操作する。Start() を一度呼んでから、毎フレーム SetTargetPosition() → Update() の順に呼ぶ。
         *          調整値は PlayerCameraParameter(ParamHolder が持つもの。ホットリロードで値が変わる)。
         */
        class PlayerCamera : public Noncopyable
        {
        public:
            /**
             * @brief コンストラクタ
             * @note パッドが繋がっていなくても、キーボードの入力は全てのパッド番号に入る。
             *       実際に繋がっているパッド番号を指定すること。
             * @param padIndex 右スティックを読み取るパッドの番号(0〜3)。範囲外なら回転しない
             */
            explicit PlayerCamera(const int padIndex = 0);
            ~PlayerCamera();


            /**
             * @brief 初期化
             * @details 注視点から視点までのベクトルを、パラメーターの初期値に戻す。
             */
            void Start();

            /**
             * @brief 更新
             * @details 右スティックでカメラを回し、注視点と視点を g_camera3D に設定して、g_camera3D を更新する。
             */
            void Update();

            /**
             * @brief ターゲットの座標を設定
             * @details 注視点は、この座標から右上にずらした位置になる。毎フレーム、Update() の前に設定すること。
             * @param position ターゲット(プレイヤーの足元)の座標
             */
            void SetTargetPosition(const Vector3& position);

            /**
             * @brief ハンドガンを構えているかどうかを設定
             * @details 構えている間は、カメラ感度の代わりにエイム感度で回り、視点が注視点に近づく。毎フレーム、Update() の前に設定すること。
             * @param isAiming 構えているならtrue
             */
            void SetAiming(const bool isAiming)
            {
                m_isAiming = isAiming;
            }

            /**
             * @brief エイムアシストの対象(狙う点)を設定
             * @details 構えている間、視線に近い点へ減速・引き寄せがかかる。ゾンビの胸の座標などを渡す。
             *          毎フレーム、Update() の前に設定すること。構えていない間は使わない。
             * @param targets 狙う点。コピーして持つ
             */
            void SetAimTargets(const std::vector<Vector3>& targets)
            {
                m_aimTargets = targets;
            }

            /**
             * @brief 発砲の反動を足す
             * @details カメラが上へ跳ね上がり、すぐ元の向きへ戻る。基準の向き(右スティックで回した向き)は変わらない。
             *          連射すると recoilMaxDeg まで溜まる。撃った時に呼ぶ。
             */
            void AddRecoil();


        private:
            /** パラメーター(ParamHolderが持つもの。アプリが終わるまで使える) */
            const PlayerCameraParameter& m_parameter;
            /** 右スティックを読み取るパッドの番号 */
            int m_padIndex;
            /** 注視点から視点までのベクトル */
            Vector3 m_toCameraPos;
            /** ターゲットの座標 */
            Vector3 m_targetPosition;
            /** ハンドガンを構えているかどうか */
            bool m_isAiming;
            /** 構えの度合い(0=通常、1=構え切った状態)。SetAiming() の向きへ、aimBlendSeconds かけて動く */
            float m_aimRate;
            /** 壁を避けた後の、注視点から視点までの長さ(cm)。壁に近づく時は即座に縮み、離れた時は collisionRecoverSpeed で伸びる */
            float m_allowedLength;
            /** エイムアシストの判定 */
            AimAssist m_aimAssist;
            /** エイムアシストの対象(狙う点) */
            std::vector<Vector3> m_aimTargets;
            /** 反動の目標の角度(度)。撃つたびに増え、recoilRecoverRate で0へ戻る */
            float m_recoilTargetDeg;
            /** 実際に視線へ足している反動の角度(度)。目標へ recoilRiseRate で追う */
            float m_recoilDeg;
        };
    } // namespace camera
} // namespace app
