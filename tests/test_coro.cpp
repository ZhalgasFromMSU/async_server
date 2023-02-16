#include <coro/coroutine.hpp>
#include <thread/wait_group.hpp>

#include <gtest/gtest.h>

using namespace NAsync;

struct Coro: ::testing::Test {
    TEpoll Epoll;
    TThreadPool ThreadPool;

    Coro() {
        ThreadPool.Start();
    }
};

TEST_F(Coro, Coroutine) {
    auto coro = [](TWaitGroup& wg, const TIoObject& pipeRead) -> TCoroFuture<void> {
        char out[10];
        auto res = co_await TReadPollable(pipeRead, out, 3);
        out[*res] = 0;

        wg.Done();
        co_return;
    };

    TWaitGroup wg;
    wg.Add(1);

    auto pipe = TPipe::Create();
    TCoroFuture<void> task = coro(wg, pipe.ReadEnd());
    task.SetEpoll(&Epoll);
    task.SetExecutor(&ThreadPool);
    task.Run();

    Write(pipe.WriteEnd(), "123", 3);
    ASSERT_TRUE(wg.WaitFor(std::chrono::seconds(1)));
}

TEST_F(Coro, Future) {
    auto coro = [](const TIoObject& pipeRead) -> TCoroFuture<int> {
        char out[10];
        auto res = co_await TReadPollable(pipeRead, out, 3);
        out[*res] = 0;
        co_return 1;
    };

    auto pipe = TPipe::Create();
    TCoroFuture<int> task = coro(pipe.ReadEnd());
    task.SetEpoll(&Epoll);
    task.SetExecutor(&ThreadPool);
    auto future = task.Run();
    Write(pipe.WriteEnd(), "123", 3);
    ASSERT_EQ(future.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    ASSERT_EQ(future.get(), 1);
}
