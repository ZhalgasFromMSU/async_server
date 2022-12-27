#include <thread_pool/pool.hpp>

namespace NAsync {

    TThreadPool::TThreadPool(size_t numThreads) noexcept {
        Threads_.reserve(numThreads);
    }

    TThreadPool::~TThreadPool() {
        Finish();
    }

    size_t TThreadPool::QueueSize() const {
        std::scoped_lock lock{QueueMutex_};
        return JobsQueue_.size();
    }

    void TThreadPool::Start() noexcept {
        for (size_t i = 0; i < Threads_.capacity(); ++i) {
            Threads_.emplace_back(&TThreadPool::WorkerLoop, this);
        }
    }

    void TThreadPool::Finish() noexcept {
        std::unique_lock lock {QueueMutex_};
        PoolStopped_ = true;
        JobsCV_.notify_all();
        lock.unlock();

        for (auto& thread : Threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    void TThreadPool::WorkerLoop() {
        while (true) {
            std::unique_lock lock {QueueMutex_};
            JobsCV_.wait(lock, [this] {
                return !JobsQueue_.empty() || PoolStopped_;
            });

            if (!JobsQueue_.empty()) {
                auto taskPtr = std::move(JobsQueue_.front());
                JobsQueue_.pop();
                lock.unlock();
                taskPtr->Execute();
            } else { // PoolStopped_ == true
                break;
            }
        }
    }

} // namespace NAsync