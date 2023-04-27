#include <util/result.hpp>
#include <util/queue.hpp>

#include <gtest/gtest.h>

#include <iterator>
#include <unordered_map>
#include <future>

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

TEST(Result, Movable) {
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

    TResult<TSomeStruct> res{};
    ASSERT_EQ(res->Val, 3);

    TSomeStruct inner{std::move(*res)};
    ASSERT_EQ(inner.Val, 3);
}

TEST(Result, NonCopyable) {
    struct TNonCopy {
        TNonCopy(int x) : X{x} {}
        TNonCopy(const TNonCopy&) = delete;
        TNonCopy& operator=(const TNonCopy&) = delete;

        int X;
    };

    auto foo = []() -> TResult<TNonCopy> {
        return TResult<TNonCopy>{10};
    };

    auto res = foo();
    ASSERT_EQ(res->X, 10);
}


TEST(Errors, Verify) {
    VERIFY(1 < 2);
    EXPECT_DEATH(VERIFY(1 > 2), "Assertion failed");
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

TEST(Queue, PushPop) {
    NAsync::TQueue<int, 2> q;
    ASSERT_TRUE(q.Emplace(1));
    ASSERT_TRUE(q.Emplace(2));
    ASSERT_FALSE(q.Emplace(3));


    ASSERT_EQ(*q.Pop(), 1);
    ASSERT_EQ(*q.Pop(), 2);
    ASSERT_EQ(q.Pop(), std::nullopt);
}

TEST(Queue, LongPush) {
    struct TLongConstructor {
        TLongConstructor() {

        }

        TLongConstructor(bool) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    NAsync::TQueue<TLongConstructor, 1> q;
    auto future1 = std::async([&q] {
        ASSERT_TRUE(q.Emplace(true));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20)); // started emplacing
    ASSERT_EQ(q.Size(), 1);
    ASSERT_EQ(q.Pop(), std::nullopt);
    future1.wait();
    ASSERT_TRUE(q.Pop());
}
