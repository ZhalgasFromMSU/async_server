module;

#include <gtest/gtest.h>

module async;
import :threadpool;

TEST(ThreadPool, Trivial) {
  std::atomic<std::size_t> counter = 0;

  {
    async::ThreadPool<std::function<void()>> pool(1, 1);
    ASSERT_TRUE(pool.Enqueue([&] {
      ASSERT_EQ(counter.fetch_add(1, std::memory_order_relaxed), 0);
    }));

    pool.Start();
  }

  ASSERT_EQ(counter.load(std::memory_order_relaxed), 1);
}

TEST(ThreadPool, RecursiveScheduling) {
  std::atomic<std::size_t> counter = 0;
  std::atomic_flag finished;
  {
    async::ThreadPool<std::function<void()>> pool(2, 1);
    std::function<void()> inc_and_schedule = [&] {
      if (counter.fetch_add(1, std::memory_order_relaxed) < 10) {
        ASSERT_TRUE(pool.Enqueue(inc_and_schedule));
      } else {
        finished.test_and_set(std::memory_order_relaxed);
        finished.notify_one();
      }
    };

    ASSERT_TRUE(pool.Enqueue(inc_and_schedule));
    pool.Start();
    finished.wait(false, std::memory_order_relaxed);
  }

  ASSERT_EQ(counter.load(std::memory_order_relaxed), 11);
}
