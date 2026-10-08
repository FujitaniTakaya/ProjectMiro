/**
 * @file RandomDevice.h
 * @brief 乱数を生成するクラス
 */
#pragma once
#include <cstdint>
#include <random>
#include <vector>


namespace app
{
    namespace util
    {
        /**
         * @brief 乱数を生成するクラス
         * @details インスタンスは作らず、全てstatic関数で使う。乱数生成器はゲーム全体で1つ。
         *          排他制御はしていないので、メインスレッドからだけ使うこと。
         */
        class RandomDevice
        {
        public:
            /**
             * @brief 指定した範囲で整数の乱数を生成する
             * @param min 最小値
             * @param max 最大値(含む)
             * @return min以上max以下の乱数
             */
            static int Random(const int min, const int max);


            /**
             * @brief 指定した範囲で小数の乱数を生成する
             * @param min 最小値
             * @param max 最大値(含まない)
             * @return min以上max未満の乱数
             */
            static float Random(const float min, const float max);


            /**
             * @brief パーセントで確率を判定する
             * @param percent 0.0f～100.0fの範囲で指定する。範囲外は丸める。
             * @return percentの確率でtrueを返す
             */
            static bool Percent(const float percent);


            /**
             * @brief コンテナからランダムに要素を1つ選ぶ
             * @tparam T コンテナの要素型
             * @param container 選ぶ元のコンテナ。空であってはいけない。
             * @return 選ばれた要素
             */
            template <typename T>
            static const T& Random(const std::vector<T>& container)
            {
                K2_ASSERT(!container.empty(), "空のコンテナからは選べない。");
                const int index = Random(0, static_cast<int>(container.size()) - 1);
                return container[index];
            }


            /**
             * @brief 乱数列の種を指定する
             * @details 同じ種なら同じ乱数列になる。デバッグで挙動を再現したいときに使う。
             *          指定しなければ、起動ごとに違う乱数列になる。
             * @param seed 種
             */
            static void Seed(const uint32_t seed);


            /**
             * @brief 乱数生成器を取得する
             * @details std::shuffleなど、標準ライブラリに渡したいときに使う。
             * @return 乱数生成器
             */
            static std::mt19937& GetEngine();


        private:
            RandomDevice() = delete;
            ~RandomDevice() = delete;
        };
    } // namespace util
} // namespace app
