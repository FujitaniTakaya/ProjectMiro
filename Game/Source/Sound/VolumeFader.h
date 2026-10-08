/**
 * @file VolumeFader.h
 * @brief 音量の割合を、徐々に変えるクラス
 * @details 音ごとのフェード(フェードイン・フェードアウト)に使う。
 *          グループの音量(ユーザーが設定する音量)とは別の、一時的な割合(0.0〜1.0)を扱う。
 */
#pragma once


namespace app
{
    /**
     * @brief 音量の割合を、徐々に変えるクラス
     * @details 割合は0.0(無音)〜1.0(そのまま)。初期値から、目標値まで一定の速さで変わる。
     */
    class VolumeFader
    {
    public:
        /**
         * @brief コンストラクタ
         * @param initialRate 割合の初期値(0.0〜1.0)
         */
        explicit VolumeFader(const float initialRate);


    public:
        /**
         * @brief 目標の割合まで、指定した時間をかけて変え始める
         * @details 今の割合から変え始める。フェード中に呼ぶと、そのときの割合から新しく変え始める。
         * @param targetRate 目標の割合(0.0〜1.0)
         * @param seconds かける時間(秒)。0以下なら、すぐに目標の割合になる。
         */
        void FadeTo(const float targetRate, const float seconds);


        /**
         * @brief 時間を進める
         * @param deltaTime 進める時間(秒)
         */
        void Update(const float deltaTime);


        /**
         * @brief 今の割合を取得する
         * @return 割合(0.0〜1.0)
         */
        float Get() const
        {
            return m_current;
        }


        /**
         * @brief 目標の割合に向かって変わっている途中か
         * @return 変わっている途中ならtrue
         */
        bool IsFading() const
        {
            return m_current != m_target;
        }


    private:
        /** 今の割合 */
        float m_current;
        /** 目標の割合 */
        float m_target;
        /** 1秒あたりに変わる割合 */
        float m_speed;
    };
} // namespace app
