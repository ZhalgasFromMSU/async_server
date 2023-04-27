#pragma once

#include "wait_group.hpp"
#include <util/queue.hpp>
#include <util/task.hpp>

#include <thread>
#include <vector>
#include <coroutine>

namespace NAsync {

    class TThreadPool {
    public:
        TThreadPool(size_t numThreads = std::thread::hardware_concurrency()) noexcept;
        ~TThreadPool();

        void Start() noexcept;  // Create threads and start executing jobs
        void Finish() noexcept;  // Stop enqueing and wait for threads to finish queue

        [[nodiscard]] bool EnqueJob(TJob job) noexcept;

    private:
        TWaitGroup Wg_;
        TQueue<TJob, 1024> Jobs_;

        std::vector<std::thread> Threads_;
    };

} // namespace NAsync
