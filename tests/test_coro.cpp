#include <coro/coroutine.hpp>
#include <thread/wait_group.hpp>
#include <io/well_known_structs.hpp>
#include <io/read.hpp>
#include <io/write.hpp>
#include <util/result.hpp>

#include <gtest/gtest.h>

using namespace NAsync;

struct Coro: ::testing::Test {
    TEpoll Epoll;
    TThreadPool ThreadPool;

    Coro() {
        ThreadPool.Start();
    }
};

struct TScopeGuard {
    TScopeGuard(std::function<void()> onDestructor)
        : OnDestructor(std::move(onDestructor))
    {}

    ~TScopeGuard() {
        OnDestructor();
    }

    std::function<void()> OnDestructor;
};

template<typename T>
bool IsReady(std::future<T>& future) {
    auto res = future.wait_for(std::chrono::seconds::zero());
    VERIFY(res != std::future_status::deferred);
    return res == std::future_status::ready;
}

TEST_F(Coro, Coro) {
    auto coro = [](const TIoObject& pipeOut, TWaitGroup& wg) -> TCoroFuture<int> {
        TScopeGuard guard{[&wg] { wg.Done(); }};
        char out[1];
        auto numRead = co_await TReadAwaitable(pipeOut, out, 1);
        co_return *numRead;
    };

    TWaitGroup wg;
    TPipe pipe = TPipe::Create();

    auto task = coro(pipe.ReadEnd(), wg);
    task.SetEpoll(&Epoll);
    task.SetThreadPool(&ThreadPool);
    wg.Add(1);

    auto future = task.Run();
    ASSERT_FALSE(wg.WaitFor(std::chrono::milliseconds(10)));
    ASSERT_FALSE(IsReady(future));

    ASSERT_EQ(*Write(pipe.WriteEnd(), "1", 1), 1);
    ASSERT_TRUE(wg.WaitFor(std::chrono::seconds(1)));
    ASSERT_TRUE(IsReady(future));
    ASSERT_EQ(future.get(), 1);
}

TEST_F(Coro, VoidCoro) {
    std::atomic<int> sharedNumRead;

    auto coro = [&sharedNumRead](const TIoObject& pipeOut, TWaitGroup& wg) -> TCoroFuture<void> {
        TScopeGuard guard{[&wg] { wg.Done(); }};
        char out[1];
        auto numRead = co_await TReadAwaitable(pipeOut, out, 1);
        sharedNumRead = *numRead;
        co_return;
    };

    TWaitGroup wg;
    TPipe pipe = TPipe::Create();

    auto task = coro(pipe.ReadEnd(), wg);
    task.SetEpoll(&Epoll);
    task.SetThreadPool(&ThreadPool);
    wg.Add(1);

    auto future = task.Run();
    ASSERT_FALSE(wg.WaitFor(std::chrono::milliseconds(10)));
    ASSERT_FALSE(IsReady(future));

    ASSERT_EQ(*Write(pipe.WriteEnd(), "1", 1), 1);
    ASSERT_TRUE(wg.WaitFor(std::chrono::seconds(1)));
    ASSERT_TRUE(IsReady(future));
    ASSERT_EQ(sharedNumRead, 1);
}

TEST_F(Coro, NestedCoro) {
    auto coro = [this](const TIoObject& pipeOut, TWaitGroup& wg) -> TCoroFuture<int> {
        TScopeGuard guard {[&wg] { wg.Done(); }};

        auto innerCoro = [](const TIoObject& pipeOut) -> TCoroFuture<int> {
            char out[1];
            co_return *(co_await TReadAwaitable(pipeOut, out, 1));
        };

        co_return co_await innerCoro(pipeOut);
    };

    TWaitGroup wg;
    TPipe pipe = TPipe::Create();

    auto task = coro(pipe.ReadEnd(), wg);
    task.SetEpoll(&Epoll);
    task.SetThreadPool(&ThreadPool);
    wg.Add(1);

    auto future = task.Run();
    ASSERT_FALSE(wg.WaitFor(std::chrono::milliseconds(10)));
    ASSERT_FALSE(IsReady(future));

    ASSERT_EQ(*Write(pipe.WriteEnd(), "1", 1), 1);
    ASSERT_TRUE(wg.WaitFor(std::chrono::seconds(1)));
    ASSERT_TRUE(IsReady(future));
    ASSERT_EQ(future.get(), 1);
}

TEST_F(Coro, WithoutEpoll) {
    auto coro = []() -> TCoroFuture<int> {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        co_return 1;
    };

    auto task = coro();
    task.SetThreadPool(&ThreadPool);

    auto future = task.Run();
    ASSERT_FALSE(IsReady(future));
    future.wait_for(std::chrono::milliseconds(1500));
    ASSERT_TRUE(IsReady(future));
    ASSERT_EQ(future.get(), 1);
}

TEST_F(Coro, WithoutThreadPool) {
    auto coro = [](const TIoObject& pipeRead, TWaitGroup& wg) -> TCoroFuture<int> {
        TScopeGuard guard{[&wg] { wg.Done(); }};
        char out[1];
        co_return *(co_await TReadAwaitable(pipeRead, out, 1));
    };

    TWaitGroup wg;
    TPipe pipe = TPipe::Create();
    auto task = coro(pipe.ReadEnd(), wg);
    task.SetEpoll(&Epoll);
    wg.Add(1);

    auto future = task.Run();
    ASSERT_FALSE(IsReady(future));
    ASSERT_EQ(*Write(pipe.WriteEnd(), "1", 1), 1);
    wg.WaitFor(std::chrono::seconds(1));
    ASSERT_TRUE(IsReady(future));
    ASSERT_EQ(future.get(), 1);
}

TEST_F(Coro, WithoutEpollAndThreadPool) {
    auto coro = []() -> TCoroFuture<int> {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        co_return 1;
    };

    auto future = coro().Run();
    ASSERT_TRUE(IsReady(future));
    ASSERT_EQ(future.get(), 1);
}

TEST_F(Coro, NestedCoroWithoutEpollAndThreadPool) {
    auto coro = []() -> TCoroFuture<int> {
        auto nestedCoro = []() -> TCoroFuture<int> {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            co_return 1;
        };

        co_return co_await nestedCoro();
    };

    auto future = coro().Run();
    ASSERT_TRUE(IsReady(future));
    ASSERT_EQ(future.get(), 1);
}
