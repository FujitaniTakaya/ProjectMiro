/**
 * @file ParamLoader.h
 * @brief jsonからパラメーターの値を読み込むのに使用する
 */
#pragma once
#include <cstdint>
#include <string>

#include "JsonView.h"


namespace app
{
    /**
     * @brief jsonからパラメーターの値を読み込むのに使用する関数群
     */
    class ParamLoader
    {
    public:
        // NOTE: 無効な値は、リテラルで初期化している。
        //       Vector3::Zeroなど、他のファイルのstatic変数から初期化すると、
        //       初期化の順番によってはゼロのままコピーされてしまうため。

        /** 無効なbool値 */
        static constexpr bool InvalidBool = false;
        /** 無効なint値 */
        static constexpr int InvalidInt = -1;
        /** 無効なuint32_t値 */
        static constexpr uint32_t InvalidUInt32 = 0;
        /** 無効なfloat値 */
        static constexpr float InvalidFloat = 0.0f;
        /** 無効なstring値 */
        static inline const std::string InvalidString = "";
        /** 無効なVector2値 */
        static inline const Vector2 InvalidVector2 = { 0.0f, 0.0f };
        /** 無効なVector3値 */
        static inline const Vector3 InvalidVector3 = { 0.0f, 0.0f, 0.0f };
        /** 無効なQuaternion値 */
        static inline const Quaternion InvalidQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f };
        /** 無効なVector4値 */
        static inline const Vector4 InvalidVector4 = { 1.0f, 1.0f, 1.0f, 1.0f };



    public:
        /**
         * @brief jsonからboolを読み込む
         * @param json 読み込むjsonファイル
         * @param key 読み込むキー
         * @param invalid 無効な値を返す場合の値(デフォルトはInvalidBool)
         * @return 読み込んだbool
         */
        static bool ToBool(
            const JsonView& json,
            const char* key,
            bool invalid = InvalidBool
        );

        /**
         * @brief jsonからintを読み込む
         * @param json 読み込むjsonファイル
         * @param key 読み込むキー
         * @param invalid 無効な値を返す場合の値(デフォルトはInvalidInt)
         * @return 読み込んだint
         */
        static int ToInt(
            const JsonView& json,
            const char* key,
            int invalid = InvalidInt
        );

        /**
         * @brief jsonからuint32_tを読み込む
         * @param json 読み込むjsonファイル
         * @param key 読み込むキー
         * @param invalid 無効な値を返す場合の値(デフォルトはInvalidUInt32)
         * @return 読み込んだuint32_t
         */
        static uint32_t ToUInt32(
            const JsonView& json,
            const char* key,
            uint32_t invalid = InvalidUInt32
        );

        /**
         * @brief jsonからfloatを読み込む
         * @param json 読み込むjsonファイル
         * @param key 読み込むキー
         * @param invalid 無効な値を返す場合の値(デフォルトはInvalidFloat)
         * @return 読み込んだfloat
         */
        static float ToFloat(
            const JsonView& json,
            const char* key,
            float invalid = InvalidFloat
        );

        /**
         * @brief jsonからstringを読み込む
         * @param json 読み込むjsonファイル
         * @param key 読み込むキー
         * @param invalid 無効な値を返す場合の値(デフォルトはInvalidString)
         * @return 読み込んだstring
         */
        static std::string ToString(
            const JsonView& json,
            const char* key,
            const std::string& invalid = InvalidString
        );

        /**
         * @brief jsonからVector2を読み込む
         * @param json 読み込むjsonファイル
         * @param key 読み込むキー
         * @param invalid 無効な値を返す場合の値(デフォルトはInvalidVector2)
         * @return 読み込んだVector2
         */
        static Vector2 ToVector2(
            const JsonView& json,
            const char* key,
            const Vector2& invalid = InvalidVector2
        );

        /**
         * @brief jsonからVector3を読み込む
         * @param json 読み込むjsonファイル
         * @param key 読み込むキー
         * @param invalid 無効な値を返す場合の値(デフォルトはInvalidVector3)
         * @return 読み込んだVector3
         */
        static Vector3 ToVector3(
            const JsonView& json,
            const char* key,
            const Vector3& invalid = InvalidVector3
        );

        /**
         * @brief jsonからQuaternionを読み込む
         * @param json        読み込むjsonファイル
         * @param key         読み込むキー
         * @param isDegree    trueなら [rotX, rotY, rotZ](度数法)から変換、
         *                    falseなら [x, y, z, w] をそのまま読む
         * @param invalid     無効な値を返す場合の値(デフォルトはInvalidQuaternion)
         * @return 読み込んだQuaternion
         */
        static Quaternion ToRotation(
            const JsonView& json,
            const char* key,
            bool isDegree = true,
            const Quaternion& invalid = InvalidQuaternion
        );

        /**
         * @brief jsonからVector4を読み込む
         * @param json 読み込むjsonファイル
         * @param key 読み込むキー
         * @param isConvert 0-255の値を0.0-1.0に変換するかどうか
         * @param invalid 無効な値を返す場合の値(デフォルトはInvalidVector4)
         * @return 読み込んだVector4
         */
        static Vector4 ToVector4(
            const JsonView& json,
            const char* key,
            bool isConvert = true,
            const Vector4& invalid = InvalidVector4
        );


    private:
        // インスタンス化を禁止
        ParamLoader() = delete;
        ~ParamLoader() = delete;
    };
} // namespace app
