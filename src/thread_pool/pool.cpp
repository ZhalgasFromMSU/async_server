#include "pool.hpp"

namespace NAsync {

    TThreadPool::TThreadPool(size_t numThreads) noexcept {
        Threads_.reserve(numThreads);
    }

    void TThreadPool::Start() noexcept {
        for (size_t i = 0; i < Threads_.capacity(); ++i) {
            Threads_.emplace_back(&TThreadPool::WorkerLoop, this);
        }
    }

    void TThreadPool::Finish() noexcept {
        PoolStopped_.store(true);
        JobsCV_.notify_all();
        for (auto& thread : Threads_) {
            thread.join();
        }
    }

    void TThreadPool::WorkerLoop() noexcept {
        while (true) {
            std::unique_lock lock {QueueMutex_};
            JobsCV_.wait(lock, [this] {
                return !JobsQueue_.empty() || PoolStopped_;
            });
            if (PoolStopped_) {
                break;
            }
            std::unique_ptr<ITask> taskPtr = std::move(JobsQueue_.front());
            JobsQueue_.pop();
            lock.unlock();

            taskPtr->Execute();
        }
    }

} // namespace NAsync