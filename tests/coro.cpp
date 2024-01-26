module;

#include <gtest/gtest.h>

#include <coroutine>

module async;

TEST(Coro, Trivial) {
  auto mb_uring = async::IoUring::Create();
  async::EventLoop loop{mb_uring.value()};
  loop.StartWorker();

  auto mb_pipe = async::Pipe::Create();

  auto read_coro = [&]() -> async::Future<std::string> {
    char buffer[10];
    int n =
        co_await mb_pipe->ReadAsync(mb_uring.value(), buffer, sizeof(buffer));
    assert(n == 3);
    co_return std::string{buffer, buffer + n};
  }();

  auto write_coro = [&]() -> async::Future<int> {
    int n = co_await mb_pipe->WriteAsync(mb_uring.value(), "123", 3);
    assert(n == 3);
    co_return n;
  }();

  read_coro.Run();
  write_coro.Run();

  ASSERT_EQ(read_coro.Get(), "123");
  ASSERT_EQ(write_coro.Get(), 3);

  loop.StopWorker();
}
