/**
 * @file AIController.h
 * @brief 周りを見て判断し、キャラクターへの入力を作るController(AI)
 * @details ProjectBeastのEnemyControllerの骨格を元にした、汎用のAI。
 *          パッドの代わりにAIが入力を作るだけなので、PlayerControllerと同じように、どのキャラクターにも付けられる。
 *
 *          状態は4つ。
 *            Idle       : その場で止まって待つ。時間が経つと Wandering へ
 *            Wandering  : 巡回点(なければホームの周り)を歩く。目標が見えたら Chase へ
 *            Chase      : 目標を走って追う。近づいたら止まって、一定間隔でAction1を押す(攻撃)
 *            ReturnHome : ホームへ歩いて戻る。着いたら Idle へ
 *          ホームから離れすぎると ReturnHome へ。追跡は、離されすぎたり、時間が尽きたりすると諦めて ReturnHome へ。
 *
 *          目標は SetTargetProvider() で教える。AIは特定のキャラクターへのポインタを持たない。
 *          例: ai->SetTargetProvider([&player](Vector3& outPosition) { outPosition = player.GetPosition(); return true; });
 * @note 目標の元になるオブジェクトを破棄する前に、SetTargetProvider(nullptr) で外すこと。(捕まえているポインタが残らないように)
 */
#pragma once
#include <functional>
#include <memory>
#include <vector>
#include "AIControllerConfig.h"
#include "Source/MoveBehavior/IController.h"


namespace app
{
    /**
     * @brief 周りを見て判断し、キャラクターへの入力を作るController(AI)
     */
    class AIController : public IController
    {
    public:
        /**
         * @brief 目標の位置を教える関数
         * @param outPosition 目標の足元のワールド座標を入れる
         * @return 目標がいれば true。いなければ false
         */
        using TargetProvider = std::function<bool(Vector3& outPosition)>;

        /** AIの状態 */
        enum class EnAIState : uint8_t
        {
            Idle,
            Wandering,
            Chase,
            ReturnHome
        };


    public:
        /**
         * @brief コンストラクタ
         * @param config 調整値
         */
        explicit AIController(const AIControllerConfig& config = AIControllerConfig());
        ~AIController() override;


        /**
         * @brief 目標の位置を教える関数を設定する
         * @param provider 目標の位置を教える関数。nullptr なら目標なし
         * @note 目標の元になるオブジェクトを破棄する前に nullptr を設定すること
         */
        void SetTargetProvider(const TargetProvider& provider);

        /**
         * @brief ホームの位置を設定する
         * @param position ホームのワールド座標
         * @details 設定しなければ、Controllerが付けられて最初の Update() のときの、操作対象の位置がホームになる。
         */
        void SetHomePosition(const Vector3& position);

        /**
         * @brief 巡回点を追加する
         * @param position 巡回点のワールド座標
         * @details 徘徊のとき、追加した順に巡回する。1つも追加しなければ、ホームの周りのランダムな点を歩く。
         */
        void AddPatrolPoint(const Vector3& position);

        /**
         * @brief 現在の状態を取得する
         * @return 現在の状態
         */
        EnAIState GetState() const
        {
            return m_state;
        }


        /**
         * @brief キャラクターに付けられたときに呼ばれる
         * @details 状態を Idle に戻し、追跡できる時間を回復する。
         */
        void OnAttach() override;

        /**
         * @brief 今フレームの入力を作る
         * @param context 操作対象の情報
         * @return 今フレームの入力。移動の方向と、Dash / Action1 の押下状態を設定する
         */
        CharacterInput Update(const ControllerContext& context) override;


    private:
        /** ステートマシン(状態の切り替え役)。AIController.cpp にだけ定義がある */
        class StateMachine;
        /** 各状態。AIController.cpp にだけ定義がある */
        class IdleState;
        class WanderingState;
        class ChaseState;
        class ReturnHomeState;


    private:
        /** ホームから離れすぎているかどうか */
        bool IsFarFromHome() const;
        /**
         * @brief 目標が見えるかどうか調べて、見えたら目標の位置を覚える
         * @return 見えれば true
         */
        bool TryFindTarget();
        /** 操作対象から目標までの間に、遮るものがないかどうか */
        bool HasLineOfSight(const Vector3& targetPosition) const;
        /** 徘徊の次の目的地を決める */
        Vector3 ChooseWanderDestination();
        /** 目的地に向かう移動入力を設定する(y成分は無視する) */
        void MoveToward(const Vector3& destination);


    private:
        /** 調整値 */
        AIControllerConfig m_config;
        /** 目標の位置を教える関数 */
        TargetProvider m_targetProvider;
        /** ホームの位置 */
        Vector3 m_homePosition;
        /** ホームの位置が SetHomePosition() で指定されているかどうか */
        bool m_isHomeFixed;
        /** 巡回点 */
        std::vector<Vector3> m_patrolPoints;
        /** 次に向かう巡回点の番号 */
        size_t m_patrolIndex;
        /** 今フレームの操作対象の情報 */
        ControllerContext m_context;
        /** 今フレームの入力(Update() の戻り値) */
        CharacterInput m_output;
        /** 現在の状態 */
        EnAIState m_state;
        /** 次の状態のID(状態が自分で書き換える。ステートマシンが次のフレームの頭で切り替える) */
        uint32_t m_nextStateID;
        /** Controllerが付けられてから、まだ Update() していないかどうか */
        bool m_isFirstUpdate;
        /** 目標を捉えているかどうか */
        bool m_hasTarget;
        /** 目標の位置(目標を捉えている間、追跡して更新する) */
        Vector3 m_targetPosition;
        /** 目標を最後に見た位置 */
        Vector3 m_lastKnownTargetPosition;
        /** 追跡できる残りの時間(秒) */
        float m_chaseTimeLeft;
        /** ステートマシン */
        std::unique_ptr<StateMachine> m_machine;
    };
}
