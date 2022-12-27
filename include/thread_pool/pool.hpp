#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <condition_variable>
#include <type_traits>

namespace NAsync {

    template<typename TFunc>
    concept CVoidToVoid = std::is_invocable_r_v<void, TFunc>; // TODO use_invoke_result_t instead of is_invocable_r, because of void

    class ITask {
    public:
        virtual ~ITask() {}

        virtual void Execute() = 0;
    };

    template<CVoidToVoid TFunc>
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
        std::queue<std::unique_ptr<ITask>> JobsQueue_;
    };

} // namespace NAsync