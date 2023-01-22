#include <thread/wait_group.hpp>
#include <polling/epoll.hpp>
#include <io/well_known_structs.hpp>

#include <gtest/gtest.h>

#include <fcntl.h>

#include <atomic>
#include <memory>
#include <random>

using namespace NAsync;

/*
    NA: TSAN doesn't work well with epoll, so there will be a lot of false positives when using thread sanitizer
    Current solution - run tests without TSAN many times ¯\_(ツ)_/¯
    TODO: Use redundant synchronization when using TSAN
    https://nullprogram.com/blog/2022/10/03/
*/

struct Epoll: testing::Test {
    TEpoll epoll;
};

TEST_F(Epoll, StartAndShutdown) {
    auto [write, read] = CreateReversedPipe();
    epoll.WatchForRead(read.Fd(), {});
}

TEST_F(Epoll, Eventfd) {
    TEventFd eventfd;

    TWaitGroup wg;
    wg.Add(1);

    epoll.WatchForRead(eventfd.Fd(), [&wg] {
        wg.Done();
    });

    eventfd.Set();
    wg.WaitFor(std::chrono::milliseconds(100));
}

TEST_F(Epoll, AddingOneFd) {
    auto [pipeWrite, pipeRead] = CreateReversedPipe();

    const char input[] = "1234";
    constexpr int size = sizeof(input);
    char output[size];
    memset(output, 0, size);

    auto readResult = pipeRead.Read(output, size);
    // check that can't read right now
    ASSERT_EQ(readResult.Error(), std::error_code(EAGAIN, std::system_category()));

    TWaitGroup wg;
    wg.Add(1);
    VERIFY_EC(epoll.WatchForRead(pipeRead.Fd(), [&] {
        ASSERT_EQ(*pipeRead.Read(output, size), size);
        wg.Done();
    }));
    ASSERT_EQ(strncmp(output, "\0\0\0\0", size), 0);
    ASSERT_EQ(*pipeWrite.Write(input, size), size);
    wg.WaitFor(std::chrono::milliseconds(100));
    ASSERT_EQ(strcmp(input, output), 0);
}

TEST_F(Epoll, EventRemovedFromWatchlist) {
    TEventFd eventfd;

    TWaitGroup wg;
    wg.Add(1);
    epoll.WatchForRead(eventfd.Fd(), [&wg, &eventfd] {
        eventfd.Reset();
        wg.Done();
    });

    eventfd.Set();
    wg.WaitFor(std::chrono::milliseconds(100));
    ASSERT_FALSE(eventfd.IsSet());
    eventfd.Set(); // Calling callback second time will call std::terminate, because TWaitGroup counter cannot go below 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // for epoll to have time to wake up
}

TEST_F(Epoll, MultipleFds) {
    int numEvents = 10;
    std::vector<TEventFd> eventFds(numEvents);

    TWaitGroup wg;
    std::atomic<int> value;
    for (int i = 0; i < numEvents; ++i) {
        epoll.WatchForRead(eventFds[i].Fd(), [&value, &wg, i] {
            value = i;
            wg.Done();
        });
    }

    for (int i = numEvents - 1; i >= 0; --i) {
        wg.Add(1);
        eventFds[i].Set();
        wg.WaitFor(std::chrono::milliseconds(100));
        ASSERT_EQ(value, i);
    }
}

TEST_F(Epoll, MultipleFdsAsync) {
    int numTasks = 10;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(1, 100);

    std::vector<int> randomSleepIntervalsMs(numTasks); // create random sleep intervals for all threads, to dismiss shenanigans with async randomness
    for (auto& sleepTimeMs : randomSleepIntervalsMs) {
        sleepTimeMs = distr(gen);
    }

    int expectedSum = (numTasks + 1) * numTasks / 2;
    int counter = 0; // callbacks are executed synchronously, so no need for synchronization

    std::vector<std::thread> threads;
    for (int i = 1; i <= numTasks; ++i) {
        threads.emplace_back(
            [this, &counter, &randomSleepIntervalsMs, i] {
                std::this_thread::sleep_for(std::chrono::milliseconds(randomSleepIntervalsMs[i - 1]));
                TWaitGroup wg;
                TEventFd eventfd;
                wg.Add(1);
                epoll.WatchForRead(eventfd.Fd(), [&counter, &wg, i] {
                    counter += i;
                    wg.Done();
                });
                std::this_thread::sleep_for(std::chrono::milliseconds(randomSleepIntervalsMs[i - 1]));
                eventfd.Set();
                wg.WaitFor(std::chrono::milliseconds(100));
            }
        );
    }

    for (auto& thread : threads) {
        thread.join();
    }

    ASSERT_EQ(counter, expectedSum);
}