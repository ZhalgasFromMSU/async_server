module;

#include <gtest/gtest.h>

#include <coroutine>

module async;

TEST(Coro, Trivial) {
  auto coro = []() -> async::Future<int> {
    std::cout << "Zdes\n" << std::endl;
    co_return 5;
  };

  [[maybe_unused]] async::Future<int> future = coro();
  future.Run();
}
