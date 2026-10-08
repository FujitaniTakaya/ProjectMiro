/**
 * @file StateMachineBase.h
 * @brief アクターのステートマシンの基底クラス群
 */
#pragma once
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "Source/Util/CRC32.h"
/**
 * @brief 数値に変換するマクロ
 * @note constexpr変数に受けることで、ハッシュ値を必ずコンパイル時に計算する
 *       (戻り値を直接 Hash32() にすると、実行時に毎回CRC32を計算してしまう)
 * @param name ステート名
 */
#define appState(name)                         \
public:                                        \
    static constexpr uint32_t ID()             \
    {                                          \
        constexpr uint32_t id = Hash32(#name); \
        return id;                             \
    }


namespace app
{
    /** 前方宣言 */
    class StateMachineBase;


    /**
     * @brief ステートの基底クラス
     */
    class IState
    {
    public:
        IState()
            : m_stateID(0)
        {
        }

        virtual ~IState() = default;


    public:
        /** ステートに入ったときの処理 */
        virtual void Enter() = 0;
        /** ステートの更新処理 */
        virtual void Update() = 0;
        /** ステートから出るときの処理 */
        virtual void Exit() = 0;


    private:
        /** StateMachineBase が AddState() で設定する */
        friend class StateMachineBase;
        /** ステートID(IsEqualCurrentState() で検索せずに比較するために持つ) */
        uint32_t m_stateID;
    };




    /*************************************************************/


    /**
     * @brief アクターのステートマシンの基底クラス
     */
    class StateMachineBase
    {
    public:
        /**
         * @brief ステートマシンを更新する
         * @note 持ち主が毎フレーム呼び出すこと
         */
        void Update();


        /** ステートを変更させる */
        void ChangeState();

        /**
         * @brief 現在のステートと指定したIDが等しいかどうか
         * @note 他のオブジェクトが、持ち主のステートを毎フレームチェックするために必要
         * @param stateID ステートID
         * @return 等しいかどうか
         */
        bool IsEqualCurrentState(const uint32_t stateID) const;

        /**
         * @brief 現在のステートの Enter() を再度呼び出す
         * @note モデルロード完了後にアニメーションを再適用するために使用する
         */
        void ReEnterCurrentState();


    protected:
        /**
         * @brief ステートを追加する
         * @tparam TState ステートの型
         * @tparam TStateMachine ステートマシンの型
         * @param stateMachine ステートのコンストラクタに渡す、持ち主のステートマシン
         */
        template <typename TState, typename TStateMachine>
        void AddState(TStateMachine&& stateMachine)
        {
            // すでに登録されている場合は警告を出して、先に登録したものを残す
            if (FindState(TState::ID()))
            {
                K2_ASSERT(false, "重複しています");
                return;
            }
            // ステートを生成してIDを持たせる
            auto state = std::make_unique<TState>(std::forward<TStateMachine>(stateMachine));
            state->m_stateID = TState::ID();
            // ステートを追加
            m_states.push_back({ TState::ID(), std::move(state) });
        }


        /**
         * @brief ステートの変更先を取得する
         * @return 変更先のステートポインタ
         */
        virtual IState* GetChangeState() = 0;


        /**
         * @brief ステートを取得する
         * @param stateID ステートID
         * @return ステートポインタ
         */
        IState* FindState(const uint32_t stateID);


    public:
        StateMachineBase();
        virtual ~StateMachineBase() = default;


    protected:
        /** ステートのIDと実体の組 */
        struct StateEntry
        {
            uint32_t id;
            std::unique_ptr<IState> state;
        };


    protected:
        /** ステートの一覧(数が少ないので線形探索する) */
        std::vector<StateEntry> m_states;
        /** 現在のステート */
        IState* m_currentState;
    };
} // namespace app
