module;

#include <gtest/gtest.h>

module async;

TEST(EventFd, Trivial) {
  auto mb_eventfd = async::EventFd::Create();
  ASSERT_TRUE(mb_eventfd) << mb_eventfd.error().message();

  mb_eventfd->Add(10);
  ASSERT_EQ(mb_eventfd->Reset(), 10);
}

TEST(StopCrane, Trivial) {
  auto mb_epoll = async::Epoll::Create();
  ASSERT_TRUE(mb_epoll) << mb_epoll.error().message();

  async::StopCrane crane{*mb_epoll};
  crane.PullStop();

  auto mb_res = mb_epoll->Dequeue();
  ASSERT_TRUE(mb_res) << mb_epoll.error().message();

  ASSERT_TRUE(crane.RefersToThis(*mb_res));
}
