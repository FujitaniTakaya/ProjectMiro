/**
 * @file PlayerCamera.h
 * @brief プレイヤーを追うカメラ(TPSの肩越しの視点)
 * @details BalloonEngineの GameCamera を元にしている。
 *          注視点は、ターゲットの右上(カメラの右方向 + 上方向にずらした位置)。
 *          視点は、注視点から「注視点→視点ベクトル」だけ離れた位置。右スティックでそのベクトルを回す。
 *            右スティック左右 : 注視点を中心に、Y軸回りに回す
 *            右スティック上下 : 注視点を中心に、上下に回す(角度の制限あり)
 *          キーボードでは、矢印キーが右スティックになる。
 */
#pragma once


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


        private:
            /** パラメーター(ParamHolderが持つもの。アプリが終わるまで使える) */
            const PlayerCameraParameter& m_parameter;
            /** 右スティックを読み取るパッドの番号 */
            int m_padIndex;
            /** 注視点から視点までのベクトル */
            Vector3 m_toCameraPos;
            /** ターゲットの座標 */
            Vector3 m_targetPosition;
        };
    } // namespace camera
} // namespace app
