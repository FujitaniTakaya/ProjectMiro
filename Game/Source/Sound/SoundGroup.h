/**
 * @file SoundGroup.h
 * @brief 音を再生するサウンドのグループ
 * @details SoundVolume(親子関係を持つ音量)に、「再生中の音を保持して、音量を反映する」機能を足したもの。
 *          SEやVoiceはこのクラスそのもの。BGMのように挙動が違うものは、このクラスを継承して表す。
 *          音量を変えると、再生中の音にも反映される。
 *          音量はマスタリングボイスなどには触らず、各音の SoundSource::SetVolume() に設定する。
 *          音ごとに、フェード(徐々に音量を変える)と一時停止ができる。
 */
#pragma once
#include <map>

#include "SoundHandle.h"
#include "SoundVolume.h"
#include "VolumeFader.h"


namespace app
{
    /** 個別の音量倍率の初期値 */
    static constexpr float DEFAULT_VOLUME_MAGNIFICATION = 1.0f;


    /**
     * @brief 音を再生するサウンドのグループ
     * @details 音の最終的な音量は「このグループの最終的な音量 x 個別の倍率 x フェードの割合」になる。
     *          このグループで再生した音に対して、SoundSource::SetVolume()を直接呼ばないこと。(音量が上書きされる。)
     */
    class SoundGroup : public SoundVolume
    {
    private:
        /**
         * @brief 再生中の音の情報
         */
        struct PlayingSound
        {
            /** サウンドソース。NewGOで作られるのでメモリの管理はエンジンが行う。 */
            SoundSource* m_source;
            /** 個別の音量倍率 */
            float m_volumeMagnification;
            /** フェードの割合 */
            VolumeFader m_fader;
            /** フェードアウトが終わったら止めるか */
            bool m_isStopAtFadeEnd;
            /** 一時停止中か */
            bool m_isPaused;


            PlayingSound(SoundSource* source, const float volumeMagnification, const float initialFadeRate)
                : m_source(source)
                , m_volumeMagnification(volumeMagnification)
                , m_fader(initialFadeRate)
                , m_isStopAtFadeEnd(false)
                , m_isPaused(false)
            {
            }
        };


        /** ハンドルと再生中の音の対応 */
        using SoundList = std::map<SoundHandle, PlayingSound>;


    public:
        /**
         * @brief コンストラクタ
         * @param parent 親の音量。nullptrなら最上位。親は、このグループより後に破棄すること。
         */
        explicit SoundGroup(SoundVolume* parent);


        /**
         * @brief デストラクタ
         * @details 再生中の音を止める。エンジン(g_engine)の破棄前に破棄すること。
         */
        ~SoundGroup() override;


    public:
        /**
         * @brief 更新処理
         * @details 再生が終わった音を、エンジンに破棄される前に保持しているリストから外す。フェードも進める。
         *          エンジンの更新(g_engine->ExecuteUpdate())の前に、毎フレーム呼ぶこと。
         * @param deltaTime 前回の更新からの経過時間(秒)
         */
        void Update(const float deltaTime);


        /**
         * @brief 音を再生する
         * @param id 音のID。None / Max は何も鳴らさずにINVALID_SOUND_HANDLEを返す。
         * @param volumeMagnification 個別の音量倍率
         * @param isLoop ループ再生するか
         * @param fadeInSeconds フェードインにかける時間(秒)。0以下なら、最初から通常の音量で鳴らす。
         * @return ハンドル。再生できなかった場合はINVALID_SOUND_HANDLE。
         */
        SoundHandle Play(const EnSoundID id, const float volumeMagnification = DEFAULT_VOLUME_MAGNIFICATION, const bool isLoop = false, const float fadeInSeconds = 0.0f);


        /**
         * @brief 音を停止する
         * @param handle 停止する音のハンドル。このグループで再生した音のみ。
         */
        void Stop(const SoundHandle handle);


        /**
         * @brief このグループで再生している音を全て停止する
         */
        void StopAll();


        /**
         * @brief 音をフェードアウトさせて、終わったら止める
         * @param handle フェードアウトさせる音のハンドル。このグループで再生した音のみ。
         * @param seconds かける時間(秒)。0以下なら、すぐに止める。
         */
        void FadeOut(const SoundHandle handle, const float seconds);


        /**
         * @brief このグループで再生している音を全て、フェードアウトさせて、終わったら止める
         * @param seconds かける時間(秒)。0以下なら、すぐに止める。
         */
        void FadeOutAll(const float seconds);


        /**
         * @brief このグループで再生している音を全て、一時停止する
         * @details この時点で再生中の音だけが対象。一時停止中に新しく再生した音は、止まらずに普通に鳴る。
         */
        void Pause();


        /**
         * @brief 一時停止中の音を全て、再開する
         * @details Pause()で止めた音だけが対象。
         */
        void Resume();


        /**
         * @brief 音を一時停止する
         * @param handle 一時停止する音のハンドル。このグループで再生した音のみ。
         */
        void Pause(const SoundHandle handle);


        /**
         * @brief 一時停止中の音を再開する
         * @param handle 再開する音のハンドル。このグループで再生した音のみ。
         */
        void Resume(const SoundHandle handle);


    protected:
        /**
         * @brief 最終的な音量が変わったときに、再生中の音に反映する
         */
        void OnVolumeChanged() override;


        /**
         * @brief 再生できるIDか
         * @details Max以上は再生できない。Noneは「鳴らさない」の意味なので、黙って弾く。
         *          None以外の範囲外の値は、バグなのでDebugではassertする。
         * @param id 音のID
         * @return 再生できるIDならtrue
         */
        static bool IsPlayableSoundID(const EnSoundID id);


    private:
        /**
         * @brief 音の最終的な音量を計算する
         * @param sound 再生中の音
         * @return グループの最終的な音量 x 個別の倍率 x フェードの割合
         */
        float CalcSoundVolume(const PlayingSound& sound) const;


        /**
         * @brief サウンドソースを作って再生する
         * @param id 音のID
         * @param isLoop ループ再生するか
         * @param volume 音量
         * @return サウンドソース。登録されていないIDなどで再生できなかった場合はnullptr。
         */
        SoundSource* CreateAndPlay(const EnSoundID id, const bool isLoop, const float volume);


        /**
         * @brief サウンドソースを停止して、エンジンに破棄を依頼する
         * @param source サウンドソース
         */
        void StopSource(SoundSource* source);


        /**
         * @brief 音を一時停止する
         * @param sound 再生中の音
         */
        void PauseSound(PlayingSound& sound);


        /**
         * @brief 一時停止中の音を再開する
         * @param sound 再生中の音
         */
        void ResumeSound(PlayingSound& sound);


        /**
         * @brief 音のフェードアウトを始める。終わったら止める。
         * @param sound 再生中の音
         * @param seconds かける時間(秒)
         */
        void StartFadeOut(PlayingSound& sound, const float seconds);


    private:
        /** 再生中の音 */
        SoundList m_sounds;
    };
} // namespace app
