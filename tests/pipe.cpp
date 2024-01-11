module;

#include <gtest/gtest.h>
#include <unistd.h>

module async;
import :pipe;

TEST(Pipe, Trivial) {
  auto mb_pipe = async::Pipe::Create();
  ASSERT_TRUE(mb_pipe) << mb_pipe.error().message();

  char buf[10];
  auto read_res = mb_pipe->Read(buf, sizeof(buf));
  ASSERT_FALSE(read_res); // EAGAIN || EWOULDBLOCK

  auto write_res = mb_pipe->Write("123", 3);
  ASSERT_TRUE(write_res) << write_res.error().message();
  ASSERT_EQ(*write_res, 3);

  read_res = mb_pipe->Read(buf, sizeof(buf));
  ASSERT_TRUE(read_res) << read_res.error().message();
  ASSERT_EQ(*read_res, 3);
  ASSERT_EQ(strncmp(buf, "123", 3), 0);
}
