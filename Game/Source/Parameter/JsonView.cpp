/**
 * @file JsonView.cpp
 * @brief jsonの要素を読み取り専用で参照する
 */
#include "stdafx.h"
#include "JsonView.h"

#include "Json/json.hpp"


namespace app
{
    namespace
    {
        /** JsonViewの参照先を、nlohmann::jsonとして取り出す */
        const nlohmann::json& ToJson(const void* node)
        {
            return *static_cast<const nlohmann::json*>(node);
        }
    }


    size_t JsonView::Size() const
    {
        const nlohmann::json& json = ToJson(m_node);

        // 配列以外は、要素を持たないものとして扱う
        return json.is_array() ? json.size() : 0;
    }


    JsonView JsonView::operator[](size_t index) const
    {
        return JsonView(&ToJson(m_node)[index]);
    }
}
