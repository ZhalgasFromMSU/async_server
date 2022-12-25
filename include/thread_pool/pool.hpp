#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <condition_variable>

namespace NAsync {

    class ITask {
    public:
        virtual ~ITask() {}

        virtual void Execute() = 0;
    };

    template<typename TFunc>
    class TTask: public ITask {
    public:
        TTask(TFunc func)
            : Func_(std::move(func))
        {}

        void Execute() override {
            Func_();
        }

    private:
        TFunc Func_;
    };

    class TThreadPool {
    public:
        TThreadPool(size_t numThreads = std::thread::hardware_concurrency()) noexcept;
        ~TThreadPool();

        size_t JobsCount() const;

        void Start() noexcept;  // Create threads and start executing jobs
        void Finish() noexcept;  // Wait for threads to finish current jobs and stop enqueing

        template<typename TFunc>
        void EnqueJob(TFunc&& func) noexcept {
            std::scoped_lock lock {QueueMutex_};
            JobsQueue_.emplace(new TTask(std::forward<TFunc>(func)));
            JobsCV_.notify_one();
            return true;
        }

    private:
        void WorkerLoop() noexcept;

        mutable std::mutex QueueMutex_;
        std::atomic<bool> PoolStopped_;
        std::condition_variable JobsCV_;

        std::vector<std::thread> Threads_;
        std::queue<std::unique_ptr<ITask>> JobsQueue_; // make it lock-free
    };

} // namespace NAsync