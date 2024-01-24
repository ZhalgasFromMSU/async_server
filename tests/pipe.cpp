module;

#include <gtest/gtest.h>

#include <unistd.h>

module async;

TEST(Pipe, Trivial) {
  auto mb_pipe = async::Pipe::Create();
  ASSERT_TRUE(mb_pipe) << mb_pipe.error().message();

  auto mb_epoll = async::Epoll::Create();
  ASSERT_TRUE(mb_epoll) << mb_epoll.error().message();

  char buf[10];
  auto read_awaitable = mb_pipe->Read(*mb_epoll, buf, 10);
  auto write_awaitable = mb_pipe->Write(*mb_epoll, "123", 3);

  {
    int res = read_awaitable.Execute();
    ASSERT_TRUE(res == -EAGAIN || res == -EWOULDBLOCK);
  }

  {
    int res = write_awaitable.Execute();
    ASSERT_EQ(res, 3);
  }

  {
    int res = read_awaitable.Execute();
    ASSERT_EQ(res, 3);
  }
}
