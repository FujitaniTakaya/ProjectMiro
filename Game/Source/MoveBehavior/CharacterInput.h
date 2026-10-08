/**
 * @file CharacterInput.h
 * @brief キャラクターへの入力(Controllerの出力)
 * @details Controller(PlayerController / AIController など)が作り、キャラクターが読む。
 *          パッドの入力もAIの判断も、全てこの形に変換されるので、
 *          キャラクターはどのControllerが付いているかを知らなくてよい。(Controllerを付け替えられる)
 *          キャラクターのコードは g_pad を直接読まず、この入力だけを読むこと。
 */
#pragma once
#include <cstdint>


namespace app
{
    /**
     * @brief 入力ボタン
     * @details パッドのボタン名(A/B/X/Y)ではなく、役割で名前を付けている。
     *          ボタン名で付けると、キャラクターごとに意味が変わってしまい、付け替えられなくなるため。
     *          例: Beastでは、Bが「ペンギンでは忍び足」「シロクマでは走り」と意味が割れていた。
     */
    enum class EnInputButton : uint8_t
    {
        /** ジャンプ(パッドのA) */
        Jump,
        /** 走り。Sneakと同時には立たない */
        Dash,
        /** 忍び足。Dashと同時には立たない。DashもSneakも立っていなければ歩き */
        Sneak,
        /** アクション1(パッドのX。ペンギンならスライド、シロクマなら攻撃) */
        Action1,
        /** アクション2(パッドのY。ペンギンなら再集合の呼びかけ) */
        Action2,

        Count
    };


    /**
     * @brief キャラクターへの入力
     * @details 移動方向と、ボタンの押下状態を持つ。
     *          Controllerは移動方向(SetMove)と押下状態(SetPress)だけを設定する。
     *          IsTrigger() は ControllerSlot が前フレームとの差から計算するので、Controllerは設定しなくてよい。
     */
    class CharacterInput
    {
    public:
        CharacterInput()
            : m_move(Vector3::Zero)
            , m_pressMask(0)
            , m_triggerMask(0)
        {
        }


        /**
         * @brief 移動入力を設定する
         * @details y成分は0にし、長さが1を超える場合は1にそろえる。
         *          長さはスティックの倒し具合(0.0f〜1.0f)として扱われる。
         * @param worldMove ワールド空間の移動入力
         */
        void SetMove(const Vector3& worldMove)
        {
            m_move = Vector3(worldMove.x, 0.0f, worldMove.z);
            if (m_move.LengthSq() > 1.0f)
            {
                m_move.Normalize();
            }
        }

        /**
         * @brief 移動入力を取得する
         * @return ワールド空間(XZ平面)の移動入力。長さが倒し具合(速度の割合)
         */
        const Vector3& GetMove() const
        {
            return m_move;
        }

        /**
         * @brief 移動入力があるかどうか
         * @return 移動入力があれば true
         */
        bool IsMoving() const
        {
            return m_move.LengthSq() > MOVE_THRESHOLD_SQ;
        }


        /**
         * @brief ボタンの押下状態を設定する
         * @param button ボタン
         * @param isPress 押しているかどうか
         */
        void SetPress(const EnInputButton button, const bool isPress = true)
        {
            if (isPress)
            {
                m_pressMask |= ToBit(button);
            }
            else
            {
                m_pressMask &= ~ToBit(button);
            }
        }

        /**
         * @brief ボタンを押しているかどうか
         * @param button ボタン
         * @return 押していれば true
         */
        bool IsPress(const EnInputButton button) const
        {
            return (m_pressMask & ToBit(button)) != 0;
        }

        /**
         * @brief ボタンを押した瞬間かどうか
         * @param button ボタン
         * @return 前フレームで押しておらず、今フレームで押していれば true
         */
        bool IsTrigger(const EnInputButton button) const
        {
            return (m_triggerMask & ToBit(button)) != 0;
        }

        /**
         * @brief 前フレームの入力との差から、押した瞬間を計算する
         * @note ControllerSlot だけが呼ぶ
         * @param previous 前フレームの入力
         */
        void UpdateTrigger(const CharacterInput& previous)
        {
            m_triggerMask = m_pressMask & ~previous.m_pressMask;
        }


    private:
        /** ボタンをビットに変換する */
        static uint32_t ToBit(const EnInputButton button)
        {
            return 1u << static_cast<uint32_t>(button);
        }


    private:
        static_assert(static_cast<uint32_t>(EnInputButton::Count) <= 32, "ボタンはuint32_tに収まる数にすること");

        /** 移動入力が「ある」とみなす長さの二乗(Beastの STICK_AMOUNT_THRESHOLD = 0.0001 の二乗) */
        static constexpr float MOVE_THRESHOLD_SQ = 0.0001f * 0.0001f;


    private:
        /** 移動入力(ワールド空間のXZ平面。長さが倒し具合) */
        Vector3 m_move;
        /** 押しているボタン */
        uint32_t m_pressMask;
        /** 押した瞬間のボタン */
        uint32_t m_triggerMask;
    };
} // namespace app
