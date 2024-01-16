module;

#include <gtest/gtest.h>

#include <unistd.h>

module async;

TEST(Pipe, Trivial) {
  auto mb_pipe = async::Pipe::Create();
  ASSERT_TRUE(mb_pipe) << mb_pipe.error().message();

  char buf[10];
  ASSERT_EQ(read(mb_pipe->ReadEnd().fd(), buf, sizeof(buf)), -1);
  ASSERT_NE(errno & (EAGAIN || EWOULDBLOCK), 0);

  ASSERT_EQ(write(mb_pipe->WriteEnd().fd(), "123", 3), 3);

  ASSERT_EQ(read(mb_pipe->ReadEnd().fd(), buf, sizeof(buf)), 3);
  ASSERT_EQ(strncmp(buf, "123", 3), 0);
}
