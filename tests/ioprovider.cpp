module;

#include <gtest/gtest.h>

module async;

template<typename IoProvider>
void TestTrivial(IoProvider& provider) {
  auto mb_pipe = async::Pipe::Create();
  ASSERT_TRUE(mb_pipe) << mb_pipe.error().message();

  char buffer[10];
  auto read_awaitable = mb_pipe->ReadAsync(provider, buffer, 5);
  auto write_awaitable = mb_pipe->WriteAsync(provider, "123", 3);

  write_awaitable.await_suspend({});
  auto mb_write_res = provider.Dequeue();
  ASSERT_TRUE(mb_write_res) << mb_write_res.error().message();
  ASSERT_EQ(write_awaitable.await_resume(), 3);

  read_awaitable.await_suspend({});
  auto mb_read_res = provider.Dequeue();
  ASSERT_TRUE(mb_read_res) << mb_read_res.error().message();
  ASSERT_EQ(read_awaitable.await_resume(), 3);
}

TEST(IoUring, Trivial) {
  auto mb_uring = async::IoUring::Create();
  ASSERT_TRUE(mb_uring) << mb_uring.error().message();

  TestTrivial(*mb_uring);
}

TEST(Epoll, Trivial) {
  auto mb_epoll = async::Epoll::Create();
  ASSERT_TRUE(mb_epoll) << mb_epoll.error().message();

  TestTrivial(*mb_epoll);
}

template<typename IoProvider>
void TestStop(IoProvider& dispatcher) {
  dispatcher.DispatchStop();
  auto stop_res = dispatcher.Dequeue();
  ASSERT_FALSE(stop_res);
  ASSERT_FALSE(stop_res.error());
}

TEST(Epoll, Stop) {
  auto mb_epoll = async::Epoll::Create();
  ASSERT_TRUE(mb_epoll) << mb_epoll.error().message();

  TestStop(*mb_epoll);
}

TEST(IoUring, Stop) {
  auto mb_uring = async::IoUring::Create();
  ASSERT_TRUE(mb_uring) << mb_uring.error().message();

  TestStop(*mb_uring);
}
