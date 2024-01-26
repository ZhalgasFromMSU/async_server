module;

#include <gtest/gtest.h>

#include <thread>

module async;

TEST(Optional, Trivial) {
  [[maybe_unused]] async::Optional<int> a;
  std::jthread thread;
}

TEST(Optional, Void) {
  async::Optional<void> a, b;
  bool value = false;
  std::jthread consumer{[&] {
    for (int i = 0; i < 100; ++i) {
      a.Pop();
      std::cout << value << '\t' << i << std::endl;
      ASSERT_FALSE(value);
      value = true;
      b.Push();
    }
  }};

  std::jthread producer{[&] {
    for (int i = 0; i < 100; ++i) {
      b.Pop();
      std::cout << value << '\t' << i << std::endl;
      ASSERT_TRUE(value);
      value = false;
      a.Push();
    }
  }};

  a.Push();
}
