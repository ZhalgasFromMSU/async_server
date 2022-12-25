#include <thread_pool/pool.hpp>

namespace NAsync {

    TThreadPool::TThreadPool(size_t numThreads) noexcept {
        Threads_.reserve(numThreads);
    }

    TThreadPool::~TThreadPool() {
        if (!PoolStopped_) {
            Finish();
        }
    }

    size_t TThreadPool::JobsCount() const {
        std::scoped_lock lock{QueueMutex_};
        return JobsQueue_.size();
    }

    void TThreadPool::Start() noexcept {
        for (size_t i = 0; i < Threads_.capacity(); ++i) {
            Threads_.emplace_back(&TThreadPool::WorkerLoop, this);
        }
    }

    void TThreadPool::Finish() noexcept {
        PoolStopped_ = true;
        JobsCV_.notify_all();
        for (auto& thread : Threads_) {
            thread.join();
        }
    }

    void TThreadPool::WorkerLoop() noexcept {
        while (!PoolStopped_) {
            std::unique_lock lock {QueueMutex_};
            JobsCV_.wait(lock, [this] {
                return !JobsQueue_.empty() || PoolStopped_;
            });
            if (JobsQueue_.empty()) {
                break;
            }
            std::unique_ptr<ITask> taskPtr = std::move(JobsQueue_.front());
            JobsQueue_.pop();
            lock.unlock();

            taskPtr->Execute();
        }
    }

} // namespace NAsync