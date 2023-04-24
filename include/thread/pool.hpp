#pragma once

#include "wait_group.hpp"
#include <util/task.hpp>

#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <condition_variable>

namespace NAsync {

    class TThreadPool {
    public:
        TThreadPool(size_t numThreads = std::thread::hardware_concurrency()) noexcept;
        ~TThreadPool();

        size_t QueueSize() const;

        void Start() noexcept;  // Create threads and start executing jobs
        void Finish() noexcept;  // Stop enqueing and wait for threads to finish queue

        [[nodiscard]] bool EnqueJob(std::unique_ptr<ITask> task) noexcept;

        template<CVoidToVoid TFunc>
        [[nodiscard]] bool EnqueJob(TFunc&& func) noexcept {
            return EnqueJob(std::make_unique<TTask<TFunc>>(std::forward<TFunc>(func)));
        }

    private:
        void WorkerLoop();

        TWaitGroup Wg_;
        mutable std::mutex QueueMutex_;
        std::condition_variable JobsCV_;
        std::vector<std::thread> Threads_;
        std::queue<std::unique_ptr<ITask>> JobsQueue_; // TODO use lock-free queue
    };

} // namespace NAsync
