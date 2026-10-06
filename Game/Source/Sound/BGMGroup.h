/**
 * @file BGMGroup.h
 * @brief BGMのサウンドのグループ
 * @details BGMは、ゲーム上に1つしか存在せず、必ずループ再生される。
 */
#pragma once
#include "SoundGroup.h"


namespace app
{
    /**
     * @brief BGMのサウンドのグループ
     * @details SoundGroupとの違いは、Play()の挙動だけ。
     */
    class BGMGroup : public SoundGroup
    {
    public:
        /**
         * @brief コンストラクタ
         * @param parent 親の音量。nullptrなら最上位。親は、このグループより後に破棄すること。
         */
        explicit BGMGroup(SoundVolume* parent);


    public:
        /**
         * @brief BGMを再生する
         * @details 再生中のBGMがあれば、フェードアウトさせて置き換える。必ずループ再生する。
         *          SoundGroup::Play()を隠すので、ループするかどうか(isLoop)は受け取らない。
         *          再生できないID(None / Max)のときは、再生中のBGMには何もしない。
         * @param id 音のID
         * @param volumeMagnification 個別の音量倍率
         * @param fadeSeconds 切り替えにかける時間(秒)。古いBGMのフェードアウトと、新しいBGMのフェードインに、同じ時間をかける。
         *                    0以下なら、古いBGMをすぐに止めて、新しいBGMを最初から通常の音量で鳴らす。
         * @return ハンドル。再生できなかった場合はINVALID_SOUND_HANDLE。
         */
        SoundHandle Play(const EnSoundID id, const float volumeMagnification = DEFAULT_VOLUME_MAGNIFICATION, const float fadeSeconds = 0.0f);
    };
} // namespace app
