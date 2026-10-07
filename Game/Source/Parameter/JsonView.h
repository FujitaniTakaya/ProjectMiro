/**
 * @file JsonView.h
 * @brief jsonの要素を読み取り専用で参照する
 */
#pragma once
#include <cstddef>


namespace app
{
    /**
     * @brief jsonの要素への、読み取り専用の参照
     * @details nlohmann::jsonを呼び出し側に見せないためのクラス。
     *          参照先は、JsonLoaderが保持しているjson。
     *          JsonLoaderが破棄された場合や、再度Loadに成功した場合は、参照先がなくなるので使わないこと。
     */
    class JsonView
    {
    public:
        /**
         * @brief 配列の要素数を取得
         * @return 配列の要素数。配列でない場合は0
         */
        size_t Size() const;

        /**
         * @brief 配列の要素を取得
         * @param index 要素の番号。Size()未満であること
         * @return 要素への参照
         */
        JsonView operator[](size_t index) const;


    private:
        // 参照先(nlohmann::json)を取り出せるのは、この2つだけ
        friend class JsonLoader;
        friend class ParamLoader;

        explicit JsonView(const void* node) : m_node(node) {}

        /** 参照先のnlohmann::json */
        const void* m_node;
    };
} // namespace app
