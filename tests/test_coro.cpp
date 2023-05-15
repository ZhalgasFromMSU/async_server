#include <coro/coroutine.hpp>

#include <gtest/gtest.h>

using namespace NAsync;


struct Coro : testing::Test {
    TEpoll Epoll;

    Coro() {
        Epoll.Start();
    }
};


TEST_F(Coro, SimpleCoro) {
    auto coro = []() -> TCoroFuture<int> {
        co_return 0;
    };

    auto task = coro();
    task.Run();

    task.Wait();
    ASSERT_EQ(task.Get(), 0);
}

TEST_F(Coro, SimpleVoidCoro) {
    auto coro = []() -> TCoroFuture<void> {
        co_return;
    };

    auto task = coro();
    task.Run();
    task.Wait();
    task.Get();
}

TEST_F(Coro, Awaitable) {
    auto coro = [](const TIoObject& pipeRead) -> TCoroFuture<TResult<int>> {
        char buf[4];
        co_return co_await pipeRead.Read(buf, 4);
    };

    TPipe pipe;
    auto task = coro(pipe.ReadEnd());
    task.SetEpoll(&Epoll);
    task.Run();

    ASSERT_FALSE(task.IsReady());
    pipe.WriteEnd().Write("1234", 4).await_resume();
    task.Wait();
    TResult<int> res = task.Get();
    ASSERT_TRUE(res) << res.Error().message();
    ASSERT_EQ(*res, 4);
}

TEST_F(Coro, NestedCoro) {
    auto coro = [](const TIoObject& pipeRead) -> TCoroFuture<size_t> {
        auto nestedCoro = [](const TIoObject& pipeRead) -> TCoroFuture<std::string> {
            char buf[4];
            co_await pipeRead.Read(buf, 3);
            buf[3] = 0;
            co_return std::string{buf};
        };

        std::string s = co_await nestedCoro(pipeRead);
        co_return s.size();
    };

    TPipe pipe;
    TCoroFuture<size_t> task = coro(pipe.ReadEnd());
    task.SetEpoll(&Epoll);

    task.Run();
    ASSERT_FALSE(task.IsReady());
    pipe.WriteEnd().Write("123", 3).await_resume();
    task.Wait();
    ASSERT_EQ(task.Get(), 3);
}

TEST(SimpleCoro, SimpleNestedCoro) {
    auto coro = []() -> TCoroFuture<void> {
        auto nestedCoro = []() -> TCoroFuture<int> {
            co_return 1;
        };
        [[maybe_unused]] int ret = co_await nestedCoro();
        co_return;
    };

    coro().Run().Get();
    //ASSERT_EQ(task.Get(), 1);
}

