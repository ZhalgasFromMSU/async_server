module;

#include <gtest/gtest.h>

module async;

TEST(Operation, Trivial) {
  using namespace async;

  const char* buf = "123";
  auto descr = BuildOpDescr(arg::fd{1}, arg::count{12}, arg::cbuffer{buf});

  ASSERT_EQ(GetArg<arg::fd>(descr), 1);
  ASSERT_EQ(GetArg<arg::count>(descr), 12);
  ASSERT_EQ(GetArg<arg::cbuffer>(descr), buf);
}
