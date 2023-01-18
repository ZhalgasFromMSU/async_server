#include <thread/wait_group.hpp>
#include <polling/epoll.hpp>
#include <io/pipe.hpp>

#include <gtest/gtest.h>

#include <fcntl.h>

#include <atomic>

TEST(Polling, StartAndShutdown) {
    NAsync::TEpoll epoll;
}

TEST(Polling, AddingOneFd) {
    auto [pipeRead, pipeWrite] = NAsync::CreatePipe();

    const char input[] = "1234";
    constexpr int size = sizeof(input);
    char output[size];
    memset(output, 0, size);

    auto readResult = pipeRead.Read(output, size);
    // check that can't read right now
    ASSERT_TRUE(readResult.Error() == std::error_code(EAGAIN, std::system_category())
                || readResult.Error() == std::error_code(EWOULDBLOCK, std::system_category()))
                << "Actual value: " << readResult.Error().value() << ", message: " << readResult.Error().message();

    NAsync::TEpoll epoll;
    epoll.WatchForRead(pipeRead.Fd(), [&] {
        pipeRead.Read(output, size);
    });
    ASSERT_EQ(strncmp(output, "\0\0\0\0", size), 0);
    pipeWrite.Write(input, size);
    ASSERT_EQ(strcmp(input, output), 0) << "Actual output: " << output;
}

TEST(Polling, EventRemovedFromWatchlist) {
    // auto [pipeRead, pipeWrite] = NAsync::CreatePipe();

    // std::atomic<int> counter = 0;
    // NAsync::TEpoll epoll;
    // epoll.WatchForRead(pipeRead.Fd(), [&counter] {
    //     counter += 1;
    //     counter.notify_one();
    // });

    // const char buf[] = "1234";
    // ASSERT_EQ(*pipeWrite.Write(buf, sizeof(buf)), sizeof(buf));
    // counter.wait(0);
    // ASSERT_EQ(counter, 1);
    // ASSERT_EQ()
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
