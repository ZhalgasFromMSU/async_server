#include <util/result.hpp>

#include <gtest/gtest.h>

using namespace NAsync;

TEST(Errors, BasicAssertions) {
    {
        struct TSomeStruct {
            int Val;
        };

        struct TSomeStructWithConstructor {
            TSomeStructWithConstructor()
                : Val{1}
            {}

            int Val;
        };

       // lvalue
        TResult<TSomeStruct> res {TSomeStruct{.Val = 4}};
        ASSERT_TRUE(res);
        ASSERT_EQ((*res).Val, 4);
        ASSERT_EQ(res->Val, 4);

        // rvalue
        ASSERT_EQ((*TResult<TSomeStruct>{TSomeStruct{.Val = 4}}).Val, 4);
        ASSERT_EQ(TResult<TSomeStruct>{TSomeStruct{.Val = 4}}->Val, 4);

        // default constructor
        TResult<TSomeStruct> res2;
        ASSERT_TRUE(res2);

        TResult<TSomeStructWithConstructor> res3;
        ASSERT_EQ(res3->Val, 1);
        ASSERT_EQ(TResult<TSomeStructWithConstructor>{}->Val, 1);
    }

    {
        TResult<int> res{std::error_code{1, std::system_category()}};
        ASSERT_FALSE(res);
        ASSERT_EQ(res.Error(), std::error_code(1, std::system_category()));
    }
}

TEST(Errors, NonCopyable) {
    struct TSomeStruct {
        TSomeStruct()
            : Val{3}
        {}
        TSomeStruct(TSomeStruct&& other) = default;
        TSomeStruct& operator=(TSomeStruct&& other) = default;

        TSomeStruct(const TSomeStruct& other) = delete;
        TSomeStruct& operator=(const TSomeStruct& other) = delete;

        int Val = 4;
    };

    TResult<TSomeStruct> res{std::in_place_type_t<TSomeStruct>{}};
    ASSERT_EQ(res->Val, 3);

    TSomeStruct inner{std::move(*res)};
    ASSERT_EQ(inner.Val, 3);
}
