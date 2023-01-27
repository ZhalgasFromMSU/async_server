#include <coro/coroutine.hpp>
#include <coro/awaitable.hpp>
// #include <coro/promise.hpp>

#include <gtest/gtest.h>

using namespace NAsync;

TEST(Coro, Coroutine) {
    void* buf = nullptr;
    TReadAwaitable awaitable{TIoObject{1}, buf, 0};
}
