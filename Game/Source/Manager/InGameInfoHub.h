/**
 * @file InGameInfoHub.h
 * @brief ゲーム中の情報を中継するクラス
 * @details アクターとUIが、お互いを知らずに情報をやり取りするための中継点。(BeastのBattleManagerにあたる)
 *          情報を出す側(Miroなど)は Notify…() で渡し、受け取る側(UIなど)は SetOn…() で関数を登録しておく。
 *          Notify…() は、値が前回から変わった時だけ、登録された関数を呼ぶ。
 *          SetOn…() は、すでに通知された値があれば、登録した時にその最新値を1回返す。(登録が通知より後でも、表示がずれない)
 *          例: InGameInfoHub::Get().SetOnPlayerHpChanged([menu](const float hp, const float maxHp) { menu->SetHp(hp, maxHp); });
 *              InGameInfoHub::Get().NotifyPlayerHp(miro.GetHp(), miro.GetMaxHp());
 *          プレイヤーの武器の残弾数も同じ形で、SetOnPlayerAmmoChanged() / NotifyPlayerAmmo() を使う。
 *          ステージのコアの耐久値も同じ形で、SetOnCoreHpChanged() / NotifyCoreHp() を使う。
 *          プレイヤーの構えも同じ形で、SetOnPlayerAimChanged() / NotifyPlayerAim() を使う。
 *          弾を撃った出来事は、SetOnPlayerFired() / NotifyPlayerFired() を使う。(値ではないので、呼ぶたびに通知される)
 * @note 登録した関数が参照するもの(UIなど)を破棄する前に、ResetObservers() を呼ぶこと。
 */
#pragma once
#include <cstdint>
#include <functional>


namespace app
{
    /**
     * @brief ゲーム中の情報を中継するクラス
     * @details 起動時の処理を持たないので、関数内staticのGet()で取得する。
     */
    class InGameInfoHub : public Noncopyable
    {
    private:
        InGameInfoHub();
        ~InGameInfoHub();


    public:
        /**
         * @brief インスタンスを取得
         * @return インスタンス
         */
        static InGameInfoHub& Get();


    public:
        /** プレイヤーのHPが変わった時に呼ぶ関数(引数: 現在のHP、最大HP) */
        using PlayerHpCallback = std::function<void(const float hp, const float maxHp)>;

        /**
         * @brief プレイヤーのHPが変わった時に呼ぶ関数を登録
         * @details すでに登録されている場合は、置き換える。通知済みの値があれば、登録した時に最新値で1回呼ぶ。
         * @param func 登録する関数。空の関数を渡すと、登録を解除する
         */
        void SetOnPlayerHpChanged(PlayerHpCallback func);

        /**
         * @brief プレイヤーのHPを通知
         * @details 前回の通知から値が変わった時だけ、登録された関数を呼ぶ。毎フレーム呼んでよい。
         * @param hp 現在のHP
         * @param maxHp 最大HP
         */
        void NotifyPlayerHp(const float hp, const float maxHp);

        /** プレイヤーの武器の残弾数が変わった時に呼ぶ関数(引数: マガジンの残弾数) */
        using PlayerAmmoCallback = std::function<void(const uint32_t remainingAmmo)>;

        /**
         * @brief プレイヤーの武器の残弾数が変わった時に呼ぶ関数を登録
         * @details すでに登録されている場合は、置き換える。通知済みの値があれば、登録した時に最新値で1回呼ぶ。
         * @param func 登録する関数。空の関数を渡すと、登録を解除する
         */
        void SetOnPlayerAmmoChanged(PlayerAmmoCallback func);

        /**
         * @brief プレイヤーの武器の残弾数を通知
         * @details 前回の通知から値が変わった時だけ、登録された関数を呼ぶ。毎フレーム呼んでよい。
         * @param remainingAmmo マガジンの残弾数
         */
        void NotifyPlayerAmmo(const uint32_t remainingAmmo);

        /** ステージのコアの耐久値が変わった時に呼ぶ関数(引数: 現在の耐久値、最大耐久値) */
        using CoreHpCallback = std::function<void(const float hp, const float maxHp)>;

        /**
         * @brief ステージのコアの耐久値が変わった時に呼ぶ関数を登録
         * @details すでに登録されている場合は、置き換える。通知済みの値があれば、登録した時に最新値で1回呼ぶ。
         * @param func 登録する関数。空の関数を渡すと、登録を解除する
         */
        void SetOnCoreHpChanged(CoreHpCallback func);

        /**
         * @brief ステージのコアの耐久値を通知
         * @details 前回の通知から値が変わった時だけ、登録された関数を呼ぶ。毎フレーム呼んでよい。
         * @param hp 現在の耐久値
         * @param maxHp 最大耐久値
         */
        void NotifyCoreHp(const float hp, const float maxHp);

        /** プレイヤーの構えが変わった時に呼ぶ関数(引数: 構えているか) */
        using PlayerAimCallback = std::function<void(const bool isAiming)>;

        /**
         * @brief プレイヤーの構えが変わった時に呼ぶ関数を登録
         * @details すでに登録されている場合は、置き換える。通知済みの値があれば、登録した時に最新値で1回呼ぶ。
         * @param func 登録する関数。空の関数を渡すと、登録を解除する
         */
        void SetOnPlayerAimChanged(PlayerAimCallback func);

        /**
         * @brief プレイヤーの構えを通知
         * @details 前回の通知から値が変わった時だけ、登録された関数を呼ぶ。毎フレーム呼んでよい。
         * @param isAiming 構えているならtrue
         */
        void NotifyPlayerAim(const bool isAiming);

        /** プレイヤーが弾を撃った時に呼ぶ関数 */
        using PlayerFiredCallback = std::function<void()>;

        /**
         * @brief プレイヤーが弾を撃った時に呼ぶ関数を登録
         * @details すでに登録されている場合は、置き換える。値ではなく出来事なので、登録時に呼ぶことはない。
         * @param func 登録する関数。空の関数を渡すと、登録を解除する
         */
        void SetOnPlayerFired(PlayerFiredCallback func);

        /**
         * @brief プレイヤーが弾を撃ったことを通知
         * @details 出来事なので、呼ぶたびに登録された関数を呼ぶ。(値が変わった時だけ呼ぶ他の通知とは違う)
         */
        void NotifyPlayerFired();

        /**
         * @brief 登録された関数を全て解除し、通知された値も忘れる
         * @details シーンの破棄など、ゲームの区切りで呼ぶ。
         */
        void ResetObservers();


    private:
        /** プレイヤーのHPが変わった時に呼ぶ関数 */
        PlayerHpCallback m_onPlayerHpChanged;
        /** プレイヤーのHPが通知済みか */
        bool m_hasPlayerHp;
        /** 最後に通知された、プレイヤーの現在のHP */
        float m_playerHp;
        /** 最後に通知された、プレイヤーの最大HP */
        float m_playerMaxHp;
        /** プレイヤーの武器の残弾数が変わった時に呼ぶ関数 */
        PlayerAmmoCallback m_onPlayerAmmoChanged;
        /** プレイヤーの武器の残弾数が通知済みか */
        bool m_hasPlayerAmmo;
        /** 最後に通知された、プレイヤーの武器の残弾数 */
        uint32_t m_playerAmmo;
        /** ステージのコアの耐久値が変わった時に呼ぶ関数 */
        CoreHpCallback m_onCoreHpChanged;
        /** ステージのコアの耐久値が通知済みか */
        bool m_hasCoreHp;
        /** 最後に通知された、ステージのコアの現在の耐久値 */
        float m_coreHp;
        /** 最後に通知された、ステージのコアの最大耐久値 */
        float m_coreMaxHp;
        /** プレイヤーの構えが変わった時に呼ぶ関数 */
        PlayerAimCallback m_onPlayerAimChanged;
        /** プレイヤーの構えが通知済みか */
        bool m_hasPlayerAim;
        /** 最後に通知された、プレイヤーが構えているか */
        bool m_playerAim;
        /** プレイヤーが弾を撃った時に呼ぶ関数 */
        PlayerFiredCallback m_onPlayerFired;
    };
} // namespace app
