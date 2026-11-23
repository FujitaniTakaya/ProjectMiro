/**
 * @file GameResult.h
 * @brief インゲームの結果をリザルトシーンへ渡す記録
 * @details シーンは引数なしで作られる(SceneManager::AddSceneMap)ので、インゲームの結果はここに書いておいて、リザルトで読む。
 *          例: GameResult::Get().Set(GameResultData(elapsedTime, normalKills, runningKills));   // InGameSceneがゲームセットになった時
 *              const GameResultData& data = GameResult::Get().Data();                           // ResultSceneで
 */
#pragma once
#include <cstdint>


namespace app
{
    /**
     * @brief インゲームの結果
     */
    struct GameResultData
    {
        GameResultData()
            : survivedSeconds(0.0f)
            , normalKills(0)
            , runningKills(0)
        {}

        /**
         * @brief コンストラクタ
         * @param seconds 守衛継続時間(秒)
         * @param normal 倒した通常ゾンビの数
         * @param running 倒したランニングゾンビの数
         */
        GameResultData(const float seconds, const uint32_t normal, const uint32_t running)
            : survivedSeconds(seconds)
            , normalKills(normal)
            , runningKills(running)
        {}

        /** 守衛継続時間(秒) */
        float survivedSeconds;
        /** 倒した通常ゾンビの数 */
        uint32_t normalKills;
        /** 倒したランニングゾンビの数 */
        uint32_t runningKills;
    };


    /**
     * @brief インゲームの結果をリザルトシーンへ渡す記録
     * @details 起動時の処理を持たないので、関数内staticのGet()で取得する。
     *          まだ結果が書かれていない間は、全部0。
     */
    class GameResult : public Noncopyable
    {
    private:
        GameResult();
        ~GameResult();


    public:
        /**
         * @brief インスタンスを取得
         * @return インスタンス
         */
        static GameResult& Get();


    public:
        /**
         * @brief 結果を書く
         * @details 前の結果は置き換える。
         * @param data インゲームの結果
         */
        void Set(const GameResultData& data);

        /**
         * @brief 結果を取得
         * @return 最後に Set() した結果。まだなら全部0
         */
        const GameResultData& Data() const;


    private:
        /** インゲームの結果 */
        GameResultData m_data;
    };
} // namespace app
