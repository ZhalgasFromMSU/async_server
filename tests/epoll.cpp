module;

#include <gtest/gtest.h>

module async;

TEST(Epoll, Trivial) {
  using namespace async;
  auto mb_epoll = Epoll::Create();
  ASSERT_TRUE(mb_epoll) << mb_epoll.error().message();

  auto mb_pipe = Pipe::Create();
  ASSERT_TRUE(mb_pipe) << mb_pipe.error().message();

  char buffer[10];
  auto read_awaitable = mb_pipe->Read(*mb_epoll, buffer, 5);
  auto write_awaitable = mb_pipe->Write(*mb_epoll, "123", 3);

  write_awaitable.await_suspend({});
  auto mb_write_res = mb_epoll->Dequeue();
  ASSERT_TRUE(mb_write_res) << mb_write_res.error().message();
  ASSERT_EQ(write_awaitable.await_resume(), 3);

  read_awaitable.await_suspend({});
  auto mb_read_res = mb_epoll->Dequeue();
  ASSERT_TRUE(mb_read_res) << mb_read_res.error().message();
  ASSERT_EQ(read_awaitable.await_resume(), 3);
}
