#pragma once

#include <atomic>
#include <future>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

namespace NAsync {

    class ITask {
    public:
        ~ITask() {}

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

        void Start() noexcept;  // Create threads and start executing jobs
        void Finish() noexcept;  // Wait for threads to finish current jobs and stop enqueing

        template<typename TFunc, typename... TArgs>
        std::future<std::invoke_result_t<TFunc, TArgs...>> EnqueJob(TFunc&& func, TArgs&&... args) noexcept {
            using TResult = std::invoke_result_t<std::decay_t<TFunc>, std::decay_t<TArgs>...>;
            std::promise<TResult> promise;
            std::future<TResult> future = promise.get_future();
            {
                std::scoped_lock lock {QueueMutex_};
                JobsQueue_.emplace(new TTask(
                    [promise = std::move(promise), func = std::bind(func, std::forward<TArgs>(args)...)]() mutable {
                        promise.set_value(func());
                    }
                ));
                JobsCV_.notify_one();
            }
            return future;
        }

    private:
        void WorkerLoop() noexcept;

        std::mutex QueueMutex_;
        std::atomic<bool> PoolStopped_;
        std::condition_variable JobsCV_;

        std::vector<std::thread> Threads_;
        std::queue<std::unique_ptr<ITask>> JobsQueue_;
    };

} // namespace NAsync