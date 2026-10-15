/**
 * @file ParamLoader.cpp
 * @brief jsonからパラメーターの値を読み込むのに使用する
 */
#include "stdafx.h"

#include "ParamLoader.h"

#include <algorithm>

#include "Json/json.hpp"

// NOTE: 型が違う時は、assertで止めずにログを出して、無効な値を返す。
//       ホットリロード中にjsonを書き間違えても、ゲームが落ちないようにするため。(Layout/UIAnimationと同じ方針)
#define VALUE_DIFFER(key) K2_LOG("jsonの値の型が違います。key=%s\n", key)

namespace app
{
    namespace
    {
        /**
         * @brief keyに対応する要素を探す
         * @note find()はキーを文字列に変換せずに検索できる。json[key]はキーごとにstd::stringを作ってしまう。
         * @param node JsonViewの参照先(nlohmann::json)
         * @return 見つかった要素。無ければnullptr
         */
        const nlohmann::json* Find(const void* node, const char* key)
        {
            const nlohmann::json& json = *static_cast<const nlohmann::json*>(node);
            const auto it = json.find(key);
            return (it != json.end()) ? &(*it) : nullptr;
        }


        /**
         * @brief 要素数がNで、全ての要素が数値の配列を、floatに読み出す
         * @param array 読み込む配列
         * @param out 読み出し先
         * @return 配列でない・要素数が違う・数値でない要素がある場合はfalse(outの中身は不定)
         */
        template <size_t N>
        bool ReadFloats(const nlohmann::json& array, float (&out)[N])
        {
            if (!array.is_array() || array.size() != N)
            {
                return false;
            }

            size_t i = 0;
            for (const auto& element : array)
            {
                if (!element.is_number())
                {
                    return false;
                }

                out[i++] = element.get<float>();
            }
            return true;
        }
    } // namespace


    bool ParamLoader::ToBool(
        const JsonView& json,
        const char* key,
        bool invalid
    )
    {
        const nlohmann::json* value = Find(json.m_node, key);
        if (!value)
        {
            return invalid;
        }
        if (!value->is_boolean())
        {
            VALUE_DIFFER(key);
            return invalid;
        }

        return value->get<bool>();
    }


    int ParamLoader::ToInt(
        const JsonView& json,
        const char* key,
        int invalid
    )
    {
        const nlohmann::json* value = Find(json.m_node, key);
        if (!value)
        {
            return invalid;
        }
        if (!value->is_number_integer())
        {
            VALUE_DIFFER(key);
            return invalid;
        }

        return value->get<int>();
    }


    uint32_t ParamLoader::ToUInt32(
        const JsonView& json,
        const char* key,
        uint32_t invalid
    )
    {
        const nlohmann::json* value = Find(json.m_node, key);
        if (!value)
        {
            return invalid;
        }
        if (!value->is_number_unsigned())
        {
            VALUE_DIFFER(key);
            return invalid;
        }

        return value->get<uint32_t>();
    }


    float ParamLoader::ToFloat(
        const JsonView& json,
        const char* key,
        float invalid
    )
    {
        const nlohmann::json* value = Find(json.m_node, key);
        if (!value)
        {
            return invalid;
        }
        if (!value->is_number())
        {
            VALUE_DIFFER(key);
            return invalid;
        }

        return value->get<float>();
    }


    std::string ParamLoader::ToString(
        const JsonView& json,
        const char* key,
        const std::string& invalid
    )
    {
        const nlohmann::json* value = Find(json.m_node, key);
        if (!value)
        {
            return invalid;
        }
        if (!value->is_string())
        {
            VALUE_DIFFER(key);
            return invalid;
        }

        return value->get<std::string>();
    }


    Vector2 ParamLoader::ToVector2(
        const JsonView& json,
        const char* key,
        const Vector2& invalid
    )
    {
        const nlohmann::json* value = Find(json.m_node, key);
        if (!value)
        {
            return invalid;
        }

        float v[2];
        if (!ReadFloats(*value, v))
        {
            VALUE_DIFFER(key);
            return invalid;
        }

        return Vector2(v[0], v[1]);
    }


    Vector3 ParamLoader::ToVector3(
        const JsonView& json,
        const char* key,
        const Vector3& invalid
    )
    {
        const nlohmann::json* value = Find(json.m_node, key);
        if (!value)
        {
            return invalid;
        }

        float v[3];
        if (!ReadFloats(*value, v))
        {
            VALUE_DIFFER(key);
            return invalid;
        }

        return Vector3(v[0], v[1], v[2]);
    }


    Quaternion ParamLoader::ToRotation(
        const JsonView& json,
        const char* key,
        bool isDegree,
        const Quaternion& invalid
    )
    {
        const nlohmann::json* value = Find(json.m_node, key);
        if (!value)
        {
            return invalid;
        }

        // 度数法から変換するモード: 配列サイズ3 [rotX, rotY, rotZ]
        if (isDegree)
        {
            float rotDeg[3];
            if (!ReadFloats(*value, rotDeg))
            {
                VALUE_DIFFER(key);
                return invalid;
            }

            Quaternion rotX, rotY, rotZ;
            rotX.SetRotationDegX(rotDeg[0]);
            rotY.SetRotationDegY(rotDeg[1]);
            rotZ.SetRotationDegZ(rotDeg[2]);

            Quaternion result = rotY;
            result *= rotX;
            result *= rotZ;
            return result;
        }

        // Quaternionを直接読むモード: 配列サイズ4 [x, y, z, w]
        float q[4];
        if (!ReadFloats(*value, q))
        {
            VALUE_DIFFER(key);
            return invalid;
        }

        return Quaternion(q[0], q[1], q[2], q[3]);
    }


    Vector4 ParamLoader::ToVector4(
        const JsonView& json,
        const char* key,
        bool isConvert,
        const Vector4& invalid
    )
    {
        constexpr float minValue = 0.0f;
        constexpr float maxValue = 1.0f;
        constexpr float convertValue = 255.0f;

        const nlohmann::json* value = Find(json.m_node, key);
        if (!value)
        {
            return invalid;
        }

        float c[4];
        if (!ReadFloats(*value, c))
        {
            VALUE_DIFFER(key);
            return invalid;
        }

        // 0-255の値を0.0-1.0に変換する
        if (isConvert)
        {
            for (float& component : c)
            {
                component = std::clamp(component / convertValue, minValue, maxValue);
            }
        }

        return Vector4(c[0], c[1], c[2], c[3]);
    }
} // namespace app
