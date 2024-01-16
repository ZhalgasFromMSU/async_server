module;

#include <gtest/gtest.h>

#include <thread>
#include <vector>

module async;

TEST(WaitGroup, Trivial) {
  async::WaitGroup wg;
  ASSERT_TRUE(wg.TryInc());
  ASSERT_TRUE(wg.TryDec());
  ASSERT_FALSE(wg.TryDec());
  ASSERT_TRUE(wg.TryInc());
  wg.Block();
  ASSERT_FALSE(wg.TryInc());
  ASSERT_TRUE(wg.Dec());
  ASSERT_FALSE(wg.Dec());
  wg.Wait();
}

TEST(WaitGroup, Concurrent) {
  async::WaitGroup wg;
  std::atomic<std::size_t> counter = 0;

  {
    std::vector<std::jthread> workers;

    for (std::size_t i = 0; i < 10; ++i) {
      workers.emplace_back([&] {
        while (wg.TryInc()) {
          counter.fetch_add(1, std::memory_order_relaxed);
        }
      });
    }

    std::size_t current_count = counter.load(std::memory_order_relaxed);
    while (current_count < 1'000) {
      counter.wait(current_count, std::memory_order_relaxed);
      current_count = counter.load(std::memory_order_relaxed);
    }
    wg.Block();
  }

  std::size_t count = counter.load(std::memory_order_relaxed);
  for (std::size_t i = 0; i < count; ++i) {
    ASSERT_TRUE(wg.TryDec());
  }
  ASSERT_FALSE(wg.TryDec());
}
