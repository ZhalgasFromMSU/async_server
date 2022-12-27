#include <thread_pool/pool.hpp>

#include <gtest/gtest.h>

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
