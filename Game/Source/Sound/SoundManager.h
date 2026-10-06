/**
 * @file SoundManager.h
 * @brief サウンドの管理をするクラス
 * @details Master / BGM / SE / Voice を持つ。シングルトン。
 *          BGM / SE / Voice はMasterの子なので、音の最終的な音量は「Master x BGM(SE/Voice) x 個別の倍率」になる。
 *          音量の調整や音の再生・停止は、各グループに対して行う。
 *          例: SoundManager::Get().GetMaster().SetVolume(0.8f);
 *              SoundManager::Get().GetSE().Play(EnSoundID::Damage);
 */
#pragma once
#include "BGMGroup.h"
#include "SoundGroup.h"
#include "SoundVolume.h"


namespace app
{
    /**
     * @brief サウンドを管理するクラス
     */
    class SoundManager : public Noncopyable
    {
    private:
        /**
         * @brief コンストラクタ
         * @details SOUND_LISTの全ての音を、サウンドエンジン(g_soundEngine)に登録する。
         */
        SoundManager();
        ~SoundManager();


    public:
        /**
         * @brief 更新処理
         * @details 再生が終わった音の回収と、フェードの更新を、BGM / SE / Voice に対して行う。
         *          エンジンの更新(g_engine->ExecuteUpdate())の前に、毎フレーム呼ぶこと。(Application::PreUpdate()から呼ぶ。)
         */
        void Update();


        /**
         * @brief BGM / SE / Voice で再生中の音を、全て一時停止する
         * @details この時点で再生中の音だけが対象。一時停止中に新しく再生した音は、止まらずに普通に鳴る。
         */
        void PauseAll();


        /**
         * @brief PauseAll()などで一時停止した音を、全て再開する
         */
        void ResumeAll();


        /**
         * @brief Masterを取得する
         * @details BGM / SE / Voice の親。音量を変えると、全ての音に掛かる。音は再生しない。
         */
        SoundVolume& GetMaster()
        {
            return m_master;
        }


        /**
         * @brief BGMを取得する
         * @details 同時に1つしか再生せず、必ずループ再生する。切り替えるときは、フェードさせることもできる。
         */
        BGMGroup& GetBGM()
        {
            return m_bgm;
        }


        /**
         * @brief SEを取得する
         */
        SoundGroup& GetSE()
        {
            return m_se;
        }


        /**
         * @brief Voiceを取得する
         */
        SoundGroup& GetVoice()
        {
            return m_voice;
        }


    private:
        /** Master。BGM / SE / Voice の親。 */
        SoundVolume m_master;
        /** BGM */
        BGMGroup m_bgm;
        /** SE */
        SoundGroup m_se;
        /** Voice */
        SoundGroup m_voice;


    public:
        /**
         * @brief インスタンスを生成する
         * @details g_soundEngineの生成後に呼ぶこと。
         */
        static void CreateInstance();


        /**
         * @brief インスタンスを破棄する
         * @details 再生中の音を止めるので、エンジン(g_engine)の破棄前に呼ぶこと。
         */
        static void DestroyInstance();


        /**
         * @brief インスタンスを取得する
         * @return インスタンス。CreateInstance()を呼ぶ前に取得してはいけない。
         */
        static SoundManager& Get();


    private:
        /** 唯一のインスタンス */
        static SoundManager* m_instance;
    };
} // namespace app
