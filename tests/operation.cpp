module;

#include <gtest/gtest.h>

module async;

TEST(Operation, Trivial) {
  using namespace async;

  const char* buf = "123";
  auto descr =
      BuildOp<OpType::kRead>(arg::fd{1}, arg::count{12}, arg::cbuffer{buf});

  static_assert(decltype(descr)::op_type == OpType::kRead);

  ASSERT_EQ(GetArg<arg::fd>(descr), 1);
  ASSERT_EQ(GetArg<arg::count>(descr), 12);
  ASSERT_EQ(GetArg<arg::cbuffer>(descr), buf);
}
