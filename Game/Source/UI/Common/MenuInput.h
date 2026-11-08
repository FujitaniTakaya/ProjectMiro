/**
 * @file MenuInput.h
 * @brief メニューを選ぶための方向入力
 * @details 十字キーと左スティック(キーボードのW/A/S/Dも左スティックの入力になる)を、「倒した瞬間に±1」にして返す。
 *          押し続けても、倒し続けても、最初の1回しか返さない。左右だけは、押し続けると連続して返す(値を動かす用)。
 *          毎フレーム、Update() を1回呼んでから、GetMoveX() GetMoveY() を読む。
 */
#pragma once


namespace app
{
    namespace ui
    {
        /**
         * @brief メニューを選ぶための方向入力
         */
        class MenuInput
        {
        public:
            MenuInput();


        public:
            /**
             * @brief 更新
             * @param pad 入力を読み取るパッド
             * @param deltaTime 前のフレームからの経過時間(秒)
             */
            void Update(const GamePad& pad, const float deltaTime);

            /**
             * @brief 倒している方向を直接渡して更新
             * @details Update() が、パッドから方向を読み取って呼ぶ。
             * @param heldX 左右。-1で左、+1で右、0で倒していない
             * @param heldY 上下。-1で上、+1で下、0で倒していない
             * @param deltaTime 前のフレームからの経過時間(秒)
             */
            void UpdateDirection(const int heldX, const int heldY, const float deltaTime);

            /** 倒している状態を忘れる。画面を開き直す時に呼ぶ。 */
            void Reset();


        public:
            /**
             * @brief このフレームに動かす左右の量を取得
             * @details 倒した瞬間に±1。倒し続けると、REPEAT_DELAY 秒後から REPEAT_INTERVAL 秒ごとに±1。
             * @return -1で左、+1で右、0で動かさない
             */
            int GetMoveX() const
            {
                return m_x.m_move;
            }

            /**
             * @brief このフレームに動かす上下の量を取得
             * @details 倒した瞬間だけ±1。倒し続けても動かさない。
             * @return -1で上、+1で下、0で動かさない
             */
            int GetMoveY() const
            {
                return m_y.m_move;
            }


        public:
            /**
             * @brief 入力を受け付けるパッドを探す
             * @param padIndex パッドの番号
             * @return パッド。番号が範囲外の場合や、パッドが無い場合はnullptr
             */
            static const GamePad* FindPad(const int padIndex);


        public:
            /** 左スティックを倒したと見なす倒し具合(0〜1) */
            static constexpr float STICK_TILT_THRESHOLD = 0.5f;
            /** 倒し続けた時に、連続して返し始めるまでの時間(秒) */
            static constexpr float REPEAT_DELAY = 0.4f;
            /** 連続して返す間隔(秒) */
            static constexpr float REPEAT_INTERVAL = 0.08f;


        private:
            /**
             * @brief 1つの軸の状態
             */
            struct Axis
            {
                Axis();

                /**
                 * @brief 更新
                 * @param held 倒している方向(-1、0、+1)
                 * @param deltaTime 前のフレームからの経過時間(秒)
                 * @param isRepeat 倒し続けた時に連続して返すかどうか
                 */
                void Update(const int held, const float deltaTime, const bool isRepeat);

                /** 前のフレームに倒していた方向 */
                int m_held;
                /** 同じ方向を倒し続けている時間(秒) */
                float m_holdTime;
                /** このフレームに動かす量 */
                int m_move;
            };


        private:
            /** 左右 */
            Axis m_x;
            /** 上下 */
            Axis m_y;
        };
    } // namespace ui
} // namespace app
