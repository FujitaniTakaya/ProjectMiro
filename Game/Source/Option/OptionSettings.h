/**
 * @file OptionSettings.h
 * @brief オプション画面で変える設定の値
 * @details カメラ感度と、ハンドガンを構えている時の感度(エイム感度)を持つ。どちらも倍率で、1.0が基準(PlayerCameraParameter の rotateSpeed)。
 *          音量は、SoundManager が持つ SoundVolume が正なので、ここには持たない。
 *          NOTE: 保存と読み込みはまだ無い。起動するたびに初期値に戻る。保存を足すときは、音量もここからまとめて読み書きする。
 */
#pragma once


namespace app
{
    /**
     * @brief オプション画面で変える設定の値
     */
    class OptionSettings : public Noncopyable
    {
    private:
        OptionSettings();
        ~OptionSettings() = default;


    public:
        /** 感度の下限 */
        static constexpr float MIN_SENSITIVITY = 0.5f;
        /** 感度の上限 */
        static constexpr float MAX_SENSITIVITY = 2.0f;
        /** 感度の刻み。感度はこの刻みの倍数に丸められる */
        static constexpr float SENSITIVITY_STEP = 0.1f;
        /** 感度の初期値 */
        static constexpr float DEFAULT_SENSITIVITY = 1.0f;


    public:
        /**
         * @brief カメラ感度を取得
         * @return カメラ感度(倍率)。通常時の、右スティックでカメラを回す速さに掛ける
         */
        float GetCameraSensitivity() const
        {
            return m_cameraSensitivity;
        }

        /**
         * @brief カメラ感度を設定
         * @details 範囲(MIN_SENSITIVITY〜MAX_SENSITIVITY)に収めて、SENSITIVITY_STEP の倍数に丸める。
         * @param sensitivity カメラ感度(倍率)
         */
        void SetCameraSensitivity(const float sensitivity);

        /**
         * @brief エイム感度を取得
         * @return エイム感度(倍率)。ハンドガンを構えている間の、右スティックでカメラを回す速さに掛ける
         */
        float GetAimSensitivity() const
        {
            return m_aimSensitivity;
        }

        /**
         * @brief エイム感度を設定
         * @details 範囲(MIN_SENSITIVITY〜MAX_SENSITIVITY)に収めて、SENSITIVITY_STEP の倍数に丸める。
         * @param sensitivity エイム感度(倍率)
         */
        void SetAimSensitivity(const float sensitivity);


    private:
        /** カメラ感度 */
        float m_cameraSensitivity;
        /** エイム感度 */
        float m_aimSensitivity;


    public:
        /**
         * @brief インスタンスを取得する
         * @details 起動時の処理が無いので、関数内staticにしている。
         * @return インスタンス
         */
        static OptionSettings& Get();
    };
} // namespace app
