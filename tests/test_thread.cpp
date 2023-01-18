#include <thread/pool.hpp>
#include <thread/wait_group.hpp>

#include <gtest/gtest.h>

#include <stack>
#include <future>

TEST(ThreadPool, EnqueJob) {
    // Check that thread pool waits for all tasks to be finished
    size_t numJobs {10};
    NAsync::TThreadPool threadPool {numJobs /* threads */};
    std::atomic<size_t> numCompleted {0};

    for (size_t i = 0; i < numJobs; ++i) {
        ASSERT_TRUE(threadPool.EnqueJob([&numCompleted, i, numJobs] {
            size_t check = numCompleted;
            while (numJobs - 1 - i > check) {
                numCompleted.wait(check);
                check = numCompleted;
            }
            numCompleted += 1;
            numCompleted.notify_all();
        }));
    }

    ASSERT_EQ(numCompleted, 0);
    ASSERT_EQ(threadPool.QueueSize(), numJobs);
    threadPool.Start();
    threadPool.Finish();
    ASSERT_EQ(numCompleted, numJobs);
}

TEST(ThreadPool, EnqueAfterFinish) {
    // Check that thread pool will not accept new job if already finished

    NAsync::TThreadPool threadPool {1};
    threadPool.Finish();

    ASSERT_FALSE(threadPool.EnqueJob([] {
        return;
    }));
}

TEST(WaitGroup, WaitSuccess) {
    int numTasks = 10;

    NAsync::TWaitGroup wg;
    wg.Add(numTasks);

    std::future<void> waitResult = std::async(&NAsync::TWaitGroup::Wait, &wg);

    std::stack<std::future<int>> tasks;
    for (int i = 0; i < numTasks; ++i) {
        tasks.emplace(std::async(
            std::launch::deferred,
            [&wg, i] {
                wg.Done();
                return i;
            }
        ));
    }

    while (numTasks != 0) {
        ASSERT_NE(waitResult.wait_for(std::chrono::seconds::zero()), std::future_status::ready);
        ASSERT_EQ(tasks.top().get(), numTasks - 1);
        tasks.pop();
        numTasks -= 1;
    }
    ASSERT_TRUE(tasks.empty());
    ASSERT_EQ(waitResult.wait_for(std::chrono::microseconds(1000000)), std::future_status::ready);
}

TEST(WaitGroup, WaitFail) {
    NAsync::TWaitGroup wg;
    wg.Add(1);

    std::future<bool> waitResult = std::async(&NAsync::TWaitGroup::WaitFor, &wg, std::chrono::microseconds(1000));
    std::this_thread::sleep_for(std::chrono::microseconds(2000));
    ASSERT_EQ(waitResult.wait_for(std::chrono::seconds::zero()), std::future_status::ready);
    ASSERT_EQ(waitResult.get(), false);
}
