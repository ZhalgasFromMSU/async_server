module;

#include <gtest/gtest.h>

module async;

TEST(IoUring, Trivial) {
  using namespace async;
  auto mb_uring = IoUring::Create();
  ASSERT_TRUE(mb_uring) << mb_uring.error().message();

  auto mb_pipe = Pipe::Create();
  ASSERT_TRUE(mb_pipe) << mb_pipe.error().message();

  char buf[8];
  auto write_op_descr = BuildOpDescr(arg::fd{mb_pipe->WriteEnd().fd()},
                                     arg::cbuffer{"123"}, arg::count{3});

  auto read_op_descr = BuildOpDescr(arg::fd{mb_pipe->ReadEnd().fd()},
                                    arg::buffer{buf}, arg::count{sizeof(buf)});

  ASSERT_FALSE(mb_uring->Enqueue<OpType::kRead>(read_op_descr, 123));
  ASSERT_FALSE(mb_uring->Enqueue<OpType::kWrite>(write_op_descr, 567));

  auto write_res = mb_uring->Dequeue();
  ASSERT_TRUE(write_res) << write_res.error().message();
  ASSERT_EQ(write_res->result, 3);
  ASSERT_EQ(write_res->user_data, 567);

  auto read_res = mb_uring->Dequeue();
  ASSERT_TRUE(read_res) << read_res.error().message();
  ASSERT_EQ(read_res->result, 3);
  ASSERT_EQ(read_res->user_data, 123);
  ASSERT_EQ(strncmp(buf, "123", 3), 0);
}
