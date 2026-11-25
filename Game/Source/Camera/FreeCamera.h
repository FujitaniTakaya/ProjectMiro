/**
 * @file FreeCamera.h
 * @brief ターゲットの周りを自由に動くカメラ(撮影・デバッグ用)
 * @details ターゲットの座標(プレイヤーの足元)から高さ TARGET_HEIGHT 上を、常に見る。
 *          右スティック(キーボードは矢印) : 周回(左右=水平、上下=上下。上下は ±85 度まで)
 *          RB / LB : ズームイン / ズームアウト(押している間)
 *          十字キー右 / 左 : ロール(画面の傾き。右=時計回り)
 *          Select : 距離・向き・ロールを初期状態に戻す
 *          PlayerCamera と違って、肩越しの位置・壁よけ・エイムアシスト・反動は無い。
 *          g_camera3D を操作する。Start() を一度呼んでから、毎フレーム SetTargetPosition() → Update() の順に呼ぶ。
 *          NOTE: ロールで g_camera3D の上方向を傾けるので、このカメラを使い終わったら RestoreUp() で戻すこと。
 */
#pragma once


namespace app
{
    namespace camera
    {
        /**
         * @brief ターゲットの周りを自由に動くカメラ
         */
        class FreeCamera : public Noncopyable
        {
        public:
            /**
             * @brief コンストラクタ
             * @param padIndex 入力を読み取るパッドの番号(0〜3)。範囲外なら操作できない
             */
            explicit FreeCamera(const int padIndex = 0);
            ~FreeCamera();


        public:
            /** @brief 初期化。距離・向き・ロールを初期状態にする */
            void Start();

            /** @brief 更新。入力で周回・ズーム・ロールをして、g_camera3D に設定して更新する */
            void Update();

            /**
             * @brief ターゲットの座標を設定
             * @details 毎フレーム、Update() の前に設定すること。
             * @param position ターゲット(プレイヤーの足元)の座標
             */
            void SetTargetPosition(const Vector3& position)
            {
                m_targetPosition = position;
            }

            /** @brief g_camera3D の上方向を、Y軸(ロール無し)に戻す。他のカメラに切り替える前に呼ぶ */
            static void RestoreUp();


        private:
            /** 入力を読み取るパッドの番号 */
            int m_padIndex;
            /** ターゲット(足元)の座標 */
            Vector3 m_targetPosition;
            /** 注視点から視点への向き(長さ1) */
            Vector3 m_toCameraDir;
            /** 注視点から視点までの距離(cm) */
            float m_distance;
            /** ロールの角度(度)。正=時計回り */
            float m_rollDeg;
        };
    } // namespace camera
} // namespace app
