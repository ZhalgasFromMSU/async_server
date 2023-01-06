#include <polling/epoll.hpp>

#include <gtest/gtest.h>

#include <cassert>

TEST(Polling, Trivial) {
    NAsync::TEpoll epoll;
}

// TEST(Polling, TestThatFailsAddress) {
//     int a[100];
//     std::cerr << a[101] << std::endl;
// }

// TEST(Polling, TestThatFailsThread) {

//     int val = 0;
//     std::thread thread1([&val] { val = 1; });
//     std::thread thread2([&val] { val = 2; });
//     thread1.join();
//     thread2.join();
// }
