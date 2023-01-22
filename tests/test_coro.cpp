#include <coro/coroutine.hpp>

#include <gtest/gtest.h>

using namespace NAsync;

TCoroutine<void> Coroutine() {
    std::cerr << "Zdes\n";
    co_await std::suspend_always{};
    std::cerr << "Zdes\n";
    co_return;
}

TEST(Coro, Coroutine) {
    auto handle = Coroutine();
    handle.Handle();
    handle.Handle();
}
