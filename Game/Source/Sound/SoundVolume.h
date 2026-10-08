/**
 * @file SoundVolume.h
 * @brief 親子関係を持つ音量
 * @details Master / BGM / SE / Voice は、どれも「0〜1の音量を持ち、親の音量が掛かる」という同じものなので、このクラスで表す。
 *          Masterはこのクラスそのもの(音は再生しない)。音を再生するものは、SoundGroupがこのクラスを継承して表す。
 *          最終的な音量は「自分の音量 x 親の音量 x ...」になる。
 *          (例: BGMの親がMasterなら、BGMの音量は Master x BGM。)
 */
#pragma once
#include <vector>


namespace app
{
    /**
     * @brief 親子関係を持つ音量
     * @details 音量は線形の値で、0.0(無音)〜1.0(原音)。範囲外の値は丸められる。
     */
    class SoundVolume : public Noncopyable
    {
    public:
        /**
         * @brief コンストラクタ
         * @param parent 親。nullptrなら最上位。親は、このインスタンスより後に破棄すること。
         */
        explicit SoundVolume(SoundVolume* parent);


        virtual ~SoundVolume() = default;


    public:
        /**
         * @brief 音量を設定する
         * @details 子の最終的な音量も変わるので、子にも伝える。
         * @param volume 音量(0.0〜1.0)
         */
        void SetVolume(const float volume);


        /**
         * @brief 音量を取得する
         * @return 音量(0.0〜1.0)。親の音量は含まない。
         */
        float GetVolume() const
        {
            return m_volume;
        }


        /**
         * @brief 親の音量を含めた音量を取得する
         * @return 自分の音量 x 親の音量 x ...
         */
        float GetEffectiveVolume() const;


    protected:
        /**
         * @brief 最終的な音量が変わったときに呼ばれる
         * @details 自分の音量が変わったときと、親の音量が変わったときに呼ばれる。再生中の音への反映などを行う。
         */
        virtual void OnVolumeChanged()
        {
        }


    private:
        /** 自分と子に、最終的な音量が変わったことを伝える */
        void NotifyVolumeChanged();


    private:
        /** 親 */
        SoundVolume* m_parent;
        /** 子 */
        std::vector<SoundVolume*> m_children;
        /** 音量。親の音量は含まない。 */
        float m_volume;
    };
} // namespace app
