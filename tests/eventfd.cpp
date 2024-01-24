module;

#include <gtest/gtest.h>

module async;

TEST(EventFd, Trivial) {
  auto mb_eventfd = async::EventFd::Create();
  ASSERT_TRUE(mb_eventfd) << mb_eventfd.error().message();

  auto mb_epoll = async::Epoll::Create();
  ASSERT_TRUE(mb_epoll) << mb_epoll.error().message();

  std::uint64_t val = 10;
  ASSERT_EQ(mb_eventfd->Add(*mb_epoll, val).Execute(), 8);
  std::uint64_t res;
  ASSERT_EQ(mb_eventfd->Reset(*mb_epoll, res).Execute(), 8);
  ASSERT_EQ(res, 10);
}
