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
  ASSERT_FALSE(mb_epoll->Enqueue<OpType::kRead>(
      BuildOpDescr(arg::fd{mb_pipe->ReadEnd().fd()}, arg::buffer{buffer},
                   arg::count{3}),
      123));
  ASSERT_FALSE(mb_epoll->Enqueue<OpType::kWrite>(
      BuildOpDescr(arg::fd{mb_pipe->WriteEnd().fd()}, arg::cbuffer{"123"},
                   arg::count{3}),
      456));

  auto write_res = mb_epoll->Dequeue();
  ASSERT_TRUE(write_res) << write_res.error().message();
  ASSERT_EQ(write_res->result, 3);
  ASSERT_EQ(write_res->user_data, 456);

  auto read_res = mb_epoll->Dequeue();
  ASSERT_TRUE(read_res) << read_res.error().message();
  ASSERT_EQ(read_res->result, 3);
  ASSERT_EQ(read_res->user_data, 123);
}
