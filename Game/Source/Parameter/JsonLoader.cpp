/**
 * @file JsonLoader.cpp
 * @brief jsonファイルの読み込みに使用する
 */
#include "stdafx.h"

#include "JsonLoader.h"

#include <fstream>
#include <utility>

#include "Json/json.hpp"


namespace app
{
    struct JsonLoader::JsonImpl
    {
        /** 読み込んだjson */
        nlohmann::json json;
    };


    JsonLoader::JsonLoader()
        : m_impl(std::make_unique<JsonImpl>())
    {
    }


    // NOTE: JsonImplの定義が見える、このファイルで定義する必要がある。
    JsonLoader::~JsonLoader() = default;


    bool JsonLoader::Load(const std::string& filePath)
    {
        // ファイルストリームを開く
        std::ifstream file(filePath);

        // ファイルが開けなかった場合は、読み込み失敗
        if (!file.is_open())
        {
            return false;
        }

        nlohmann::json jsonTemp;

        // jsonの読み込みを試す
        // NOTE: 一度ファイル全体をメモリに読み込んでからパースする方法も試したが、5.9MBのjsonで約3%しか速くならなかった。
        //       一方で、「最初のjsonの後ろに余計な文字があるファイル」を拒否するようになり、挙動が変わるため採用しなかった。
        try
        {
            file >> jsonTemp;
        }
        // 例外が発生した場合は、読み込み失敗
        catch (...)
        {
            return false;
        }

        // 読み込めた場合だけ、jsonを更新する
        m_impl->json = std::move(jsonTemp);
        return true;
    }


    JsonView JsonLoader::GetRoot() const
    {
        return JsonView(&m_impl->json);
    }
} // namespace app
