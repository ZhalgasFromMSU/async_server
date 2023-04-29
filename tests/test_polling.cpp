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
    Epoll() {
        epoll.Start();
    }

    TEpoll epoll;
};

TEST_F(Epoll, StartAndShutdown) {
    TPipe pipe;
    epoll.Watch(TEpoll::EMode::kRead, pipe.ReadEnd(), []{});
}

TEST_F(Epoll, Eventfd) {
    TEventFd eventfd;

    TWaitGroup wg;
    wg.Inc();

    epoll.Watch(TEpoll::EMode::kRead, eventfd, [&wg] {
        wg.Dec();
    });

    eventfd.Set();
    wg.Block();
    wg.Wait();
}

TEST_F(Epoll, AddingOneFd) {
    TPipe pipe;

    const char input[] = "1234";
    constexpr int size = sizeof(input);
    char output[size];
    memset(output, 0, size);

    auto readResult = pipe.ReadEnd().Read(output, size).await_resume();
    // check that can't read right now
    ASSERT_EQ(readResult.Error(), std::error_code(EAGAIN, std::system_category()));

    TWaitGroup wg;
    wg.Inc();
    VERIFY_EC(epoll.Watch(TEpoll::EMode::kRead, pipe.ReadEnd(), [&] {
        ASSERT_EQ(*pipe.ReadEnd().Read(output, size).await_resume(), size);
        wg.Dec();
    }));
    ASSERT_EQ(strncmp(output, "\0\0\0\0", size), 0);
    ASSERT_EQ(*pipe.WriteEnd().Write(input, size).await_resume(), size);
    wg.Block();
    wg.Wait();
    ASSERT_EQ(strcmp(input, output), 0);
}

TEST_F(Epoll, EventRemovedFromWatchlist) {
    TPipe pipe;

    TWaitGroup wg;
    wg.Inc();
    epoll.Watch(TEpoll::EMode::kRead, pipe.ReadEnd(), [&wg] {
        wg.Dec();
    });

    pipe.WriteEnd().Write("123", 3).await_resume();
    wg.Block();
    wg.Wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // for epoll to have time to wake up
    VERIFY_EC(epoll.Watch(TEpoll::EMode::kRead, pipe.ReadEnd(), [] {}));

}

TEST_F(Epoll, MultipleFds) {
    int numEvents = 10;
    std::vector<TEventFd> eventFds(numEvents);

    std::atomic<int> value = 10;
    for (int i = 0; i < numEvents; ++i) {
        epoll.Watch(TEpoll::EMode::kRead, eventFds[i], [&value, i] {
            value = i;
            value.notify_one();
        });
    }

    for (int i = numEvents - 1; i >= 0; --i) {
        int cur = value;
        eventFds[i].Set();
        value.wait(cur);
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
                wg.Inc();
                epoll.Watch(TEpoll::EMode::kRead, eventfd, [&counter, &wg, i] {
                    counter += i;
                    wg.Dec();
                });
                std::this_thread::sleep_for(std::chrono::milliseconds(randomSleepIntervalsMs[i - 1]));
                eventfd.Set();
                wg.Block();
                wg.Wait();
            }
        );
    }

    for (auto& thread : threads) {
        thread.join();
    }

    ASSERT_EQ(counter, expectedSum);
}
