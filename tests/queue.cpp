module;

#include <gtest/gtest.h>

#include <algorithm>
#include <thread>

module async;

TEST(Queue, Trivial) {
  async::Queue<int> queue(1, 1, 1);
  ASSERT_TRUE(queue.TryPush(1));
  ASSERT_FALSE(queue.TryPush(2));
  ASSERT_TRUE(queue.TryPop().value() == 1);
  ASSERT_FALSE(queue.TryPop());
}

TEST(Queue, Pop) {
  async::Queue<int> queue(1, 1, 1);
  std::jthread consumer{[&] {
    if (queue.Pop() != 1) {
      std::terminate();
    }
  }};
  std::jthread producer{[&] {
    queue.Push(1);
  }};
}

TEST(Queue, SpSc) {
  async::Queue<std::size_t> queue(1000, 1, 1);

  std::jthread consumer{[&] {
    std::size_t counter = 0;
    while (counter < 1000) {
      ASSERT_EQ(queue.Pop(), counter++);
    }
    ASSERT_FALSE(queue.TryPop());
  }};

  std::jthread producer{[&] {
    std::size_t counter = 0;
    while (counter < 1000) {
      queue.Push(counter++);
    }
  }};
}

TEST(Queue, SpMc) {
  std::size_t n_consumers = 8;
  async::Queue<std::size_t> queue(8, 1, n_consumers);

  std::atomic<std::size_t> sum = 0;
  {
    std::vector<std::jthread> consumers;
    for (std::size_t i = 0; i < n_consumers; ++i) {
      consumers.emplace_back([&] {
        sum.fetch_add(queue.Pop(), std::memory_order_relaxed);
      });
    }

    std::jthread producer{[&] {
      for (std::size_t i = 0; i < n_consumers; ++i) {
        queue.Push(i);
      }
    }};
  }

  ASSERT_EQ(sum.load(std::memory_order_relaxed), 28);
}

TEST(Queue, MpMc) {
  std::size_t n_consumers = 8;
  std::size_t n_producers = 8;
  async::Queue<std::size_t> queue(1000, n_producers, n_consumers);

  std::vector<std::atomic_flag> flags(1000);
  {
    std::atomic<std::size_t> consumer_counter = 0;
    std::vector<std::jthread> consumers(n_consumers);
    for (auto&& consumer : consumers) {
      consumer = std::jthread{[&] {
        while (consumer_counter.fetch_add(1, std::memory_order_relaxed) <
               1000) {
          ASSERT_FALSE(
              flags[queue.Pop()].test_and_set(std::memory_order_relaxed));
        }
      }};
    }

    std::atomic<std::size_t> producer_counter = 0;
    std::vector<std::jthread> producers(n_producers);
    for (auto&& producer : producers) {
      producer = std::jthread{[&] {
        while (true) {
          std::size_t to_push =
              producer_counter.fetch_add(1, std::memory_order_relaxed);
          if (to_push >= 1000) {
            break;
          }

          queue.Push(std::move(to_push));
        }
      }};
    }
  }

  ASSERT_TRUE(std::ranges::all_of(flags, [](auto& flag) {
    return flag.test();
  }));
}

TEST(Queue, PingPong) {
  async::Queue<void> queue1{1, 1, 1}, queue2{1, 1, 1};
  bool ball = false;
  queue1.Push();

  std::jthread consumer{[&] {
    for (int i = 0; i < 1000; ++i) {
      queue1.Pop();
      ASSERT_FALSE(ball);
      ball = true;
      queue2.Push();
    }
  }};

  std::jthread producer{[&] {
    for (int i = 0; i < 1000; ++i) {
      queue2.Pop();
      ASSERT_TRUE(ball);
      ball = false;
      queue1.Push();
    }
  }};
}
