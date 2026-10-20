/**
 * @file ModelData.h
 * @brief アクターのモデル・アニメーションクリップの初期化データ
 * @details キャラクターや武器など、モデルとアニメーションクリップをロードするアクターで共有する。
 */
#pragma once
#include <cstdint>
#include <memory>


namespace app
{
    namespace actor
    {
        /** アニメーションクリップの配列 */
        using AnimationClips = std::unique_ptr<AnimationClip[]>;


        /** アニメーションデータ */
        struct AnimationData
        {
            /** ファイルネーム */
            const char* fileName;
            /** ループフラグ */
            const bool isLoop;
        };


        /** モデルデータ */
        struct ModelData
        {
            /** ファイルネーム */
            const char* fileName;
            /** アニメーションデータ */
            const AnimationData* animationData;
            /** モデルの上方向 */
            const EnModelUpAxis upAxis;
            /** アニメーションクリップの数 */
            const uint8_t clipNum;
        };


        /**
         * @brief モデルデータのアニメーションクリップを同期ロードする
         * @details アニメーションデータの順に、クリップをロードして、ループフラグを設定する。
         * @note クリップが0個のときは、配列を作らずnullptrを返す。
         *       make_unique<AnimationClip[]>(0)は非nullを返し、ModelRender側でアニメーションありとして扱われてしまう。
         * @param data モデルデータ
         * @return アニメーションクリップの配列。クリップが0個ならnullptr
         */
        inline AnimationClips LoadAnimationClips(const ModelData& data)
        {
            if (data.clipNum == 0)
            {
                return nullptr;
            }

            AnimationClips clips = std::make_unique<AnimationClip[]>(data.clipNum);
            for (int i = 0; i < data.clipNum; ++i)
            {
                clips[i].Load(data.animationData[i].fileName);
                clips[i].SetLoopFlag(data.animationData[i].isLoop);
            }
            return clips;
        }
    } // namespace actor
} // namespace app
