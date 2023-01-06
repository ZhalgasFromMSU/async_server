#pragma once

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
        void Finish() noexcept;  // Wait for threads to finish current jobs and stop enqueing

        template<CVoidToVoid TFunc>
        [[nodiscard]] bool EnqueJob(TFunc&& func) noexcept {
            std::scoped_lock lock {QueueMutex_};
            if (PoolStopped_) {
                return false;
            }
            JobsQueue_.emplace(new TTask(std::forward<TFunc>(func)));
            JobsCV_.notify_one();
            return true;
        }

    private:
        void WorkerLoop();

        mutable std::mutex QueueMutex_;
        bool PoolStopped_ = false;
        std::condition_variable JobsCV_;
        std::vector<std::thread> Threads_;
        std::queue<std::unique_ptr<ITask>> JobsQueue_; // TODO use lock-free list
    };

} // namespace NAsync