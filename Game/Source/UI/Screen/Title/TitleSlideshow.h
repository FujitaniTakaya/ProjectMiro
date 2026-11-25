/**
 * @file TitleSlideshow.h
 * @brief タイトル画面の背景。インゲームのスクリーンショットを順に流す
 * @details Assets/spriteData/UI/Title/ に置いた dds(1920x1080)を、ファイル名の順に1枚ずつ全画面に出す。
 *          画像を足す・減らす時は、ファイルを置くだけでよい。(コードの変更は要らない)
 *          - 切り替え : 次の画像を上に重ねて、透明度を 0 から 1 へ上げる。暗転を挟まないので、画面が暗くならない
 *          - 動き     : 出ている間、ゆっくり拡大する(静止画でも動きが出る)
 *          画像が1枚も無い時は、暗い単色の背景だけを描く。
 *          描画はRenderUI()の中で呼ぶこと。Render()の中だと、ポストプロセスに上書きされて見えなくなる。
 *          例: slideshow.Update(deltaTime);
 *              slideshow.Render(rc);
 */
#pragma once
#include <memory>
#include <string>
#include <vector>


namespace app
{
    namespace ui
    {
        /**
         * @brief タイトル画面の背景。インゲームのスクリーンショットを順に流す
         */
        class TitleSlideshow : public Noncopyable
        {
        public:
            TitleSlideshow();
            ~TitleSlideshow();


        public:
            /**
             * @brief 更新
             * @param deltaTime 前のフレームからの経過時間(秒)
             */
            void Update(const float deltaTime);

            /**
             * @brief 描画
             * @param rc レンダーコンテキスト
             */
            void Render(RenderContext& rc);


        public:
            /**
             * @brief 読み込んだ画像の枚数
             * @return 枚数。画像が無ければ0
             */
            size_t GetSlideCount() const
            {
                return m_slides.size();
            }


        private:
            /**
             * @brief 1枚の画像を、拡大をかけて描く
             * @param index 画像の番号
             * @param localTime その画像を出し始めてからの時間(秒)。拡大の大きさを決める
             * @param alpha 透明度(0〜1)
             * @param rc レンダーコンテキスト
             */
            void DrawSlide(const size_t index, const float localTime, const float alpha, RenderContext& rc);


        private:
            /** 暗い単色の下地。画像が無い時も、これだけは描く */
            SpriteRender m_base;
            /** 全スライドの画像。ファイル名の順。起動時に全部読み込む */
            std::vector<std::unique_ptr<SpriteRender>> m_slides;
            /** 今出している画像の番号 */
            size_t m_current;
            /** 今出している画像の、出し始めてからの時間(秒) */
            float m_timer;
        };
    } // namespace ui
} // namespace app
