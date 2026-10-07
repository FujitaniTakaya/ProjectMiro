/**
 * @file JsonLoader.h
 * @brief jsonファイルの読み込みに使用する
 */
#pragma once
#include <memory>
#include <string>
#include "Source/Parameter/JsonView.h"


namespace app
{
    /**
     * @brief jsonファイルを読み込み、読み込んだjsonを保持するクラス
     * @details 読み込んだjsonは外に出さず、JsonViewを通して読み取る。
     */
    class JsonLoader
    {
    public:
        JsonLoader();
        ~JsonLoader();


    public:
        /**
         * @brief jsonファイルを読み込む
         * @details 失敗した場合、保持しているjsonは変更しない。
         *          成功した場合、これまでにGetRoot()で取得したJsonViewは使えなくなる。
         * @param filePath 読み込むjsonファイルのパス
         * @return 読み込めたかどうか
         */
        bool Load(const std::string& filePath);

        /**
         * @brief 読み込んだjsonの、一番外側の要素を取得
         * @details まだ読み込んでいない場合は、要素を持たない(Size()が0)。
         */
        JsonView GetRoot() const;


    private:
        /** 読み込んだjsonの保持(nlohmann::jsonをヘッダーに出さないための実装クラス) */
        struct JsonImpl;
        std::unique_ptr<JsonImpl> m_impl;
    };
} // namespace app
