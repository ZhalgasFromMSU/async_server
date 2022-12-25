#include <thread_pool/pool.hpp>

#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(ThreadPool, EnqueJob) {
    size_t numJobs {1};
    NAsync::TThreadPool threadPool {numJobs /* threads */};
    std::atomic<size_t> numCompleted {0};

    auto incAfterSleep = [&numCompleted] {
      numCompleted += 1;
    };

    for (size_t i = 0; i < numJobs; ++i) {
        threadPool.EnqueJob([&numCompleted, i] {
            std::cerr << i << std::endl;
            size_t check = numCompleted;
            while (check < i) {
                numCompleted.wait(check);
                check = numCompleted;
            }
            numCompleted += 1;
        });
    }

    ASSERT_EQ(numCompleted, 0);
    ASSERT_EQ(threadPool.JobsCount(), numJobs);
    threadPool.Start();
    threadPool.Finish();
    ASSERT_EQ(numCompleted, numJobs);
}