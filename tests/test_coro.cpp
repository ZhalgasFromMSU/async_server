#include <coro/coroutine.hpp>

#include <gtest/gtest.h>

using namespace NAsync;

TCoroFuture<void> Coroutine(TEpoll& epoll, TThreadPool& threadPool, const TIoObject& pipeRead) {
    char out[10];
    auto res = co_await TReadPollable(pipeRead, out, 3);
    std::cerr << *res << std::endl;
    out[*res] = 0;
    std::cerr << out << std::endl;

    co_return;
}

TEST(Coro, Coroutine) {
    TEpoll epoll;
    TThreadPool threadPool;
    threadPool.Start();

    auto pipe = TPipe::Create();
    TCoroFuture<void> future = Coroutine(epoll, threadPool, pipe.ReadEnd());

    Write(pipe.WriteEnd(), "1234", 3);
    future.get();
}
