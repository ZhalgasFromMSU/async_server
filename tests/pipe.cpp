module;

#include <gtest/gtest.h>

#include <unistd.h>

module async;

TEST(Pipe, Trivial) {
  auto mb_pipe = async::Pipe::Create();
  ASSERT_TRUE(mb_pipe) << mb_pipe.error().message();

  char buf[10];
  {
    int res = mb_pipe->Read(buf, 10);
    ASSERT_TRUE(res == -EAGAIN || res == -EWOULDBLOCK);
  }

  {
    int res = mb_pipe->Write("123", 3);
    ASSERT_EQ(res, 3);
  }

  {
    int res = mb_pipe->Read(buf, 10);
    ASSERT_EQ(res, 3);
    ASSERT_EQ(strncmp(buf, "123", 3), 0);
  }
}
