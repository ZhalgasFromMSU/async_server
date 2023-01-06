#include <util/result.hpp>
#include <util/list.hpp>

#include <gtest/gtest.h>

#include <iterator>
#include <unordered_map>

using namespace NAsync;

TEST(Result, BasicAssertions) {
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

TEST(Result, NonCopyable) {
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

TEST(Errors, VerifySyscall) {
    VERIFY_SYSCALL(close(STDOUT_FILENO) >= 0);
    EXPECT_DEATH(VERIFY_SYSCALL(close(-1) >= 0), "Bad file descriptor");
}

TEST(Errors, VerifyErrorCode) {
    VERIFY_EC(std::error_code{});
    EXPECT_DEATH(VERIFY_EC(std::error_code(1, std::system_category())), std::error_code(1, std::system_category()).message());
}

TEST(Errors, VerifyResult) {
    VERIFY_RESULT(NAsync::TResult<int>{});
    NAsync::TResult<int> res {std::error_code{1, std::system_category()}};
    EXPECT_DEATH(VERIFY_RESULT(res), res.Error().message());
}

TEST(List, EraseBack) {
    NAsync::TList<int> list;

    auto numbers = {1, 2, 3, 4, 5};

    for (int i : numbers) {
        list.push_back(i);
        ASSERT_EQ(list.tail_pointer()->Val(), i);
    }
    ASSERT_EQ(list.size(), numbers.size());

    for (auto it = std::rbegin(numbers); it != std::rend(numbers); ++it) {
        ASSERT_EQ(list.tail_pointer()->Val(), *it);
        list.erase(list.tail_pointer());
    }

    ASSERT_EQ(list.size(), 0);
}

TEST(List, EraseMiddle) {
    NAsync::TList<int> list;
    std::unordered_map<int, NAsync::TList<int>::TNode*> pointers;

    for (int i : {1, 2, 3, 4, 5}) {
        list.push_back(i);
        pointers[i] = list.tail_pointer();
    }

    // erase elems in random order
    for (int i : {3, 5, 1, 2, 4}) {
        list.erase(pointers[i]);
    }

    ASSERT_EQ(list.size(), 0);
}
