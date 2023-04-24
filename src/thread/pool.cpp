#include <thread/pool.hpp>

namespace NAsync {

    TThreadPool::TThreadPool(size_t numThreads) noexcept {
        Threads_.reserve(numThreads);
    }

    TThreadPool::~TThreadPool() {
        if (!Wg_.Waited()) {
            Finish();
        }
    }

    size_t TThreadPool::QueueSize() const {
        return Wg_.Count();
    }

    void TThreadPool::Start() noexcept {
        for (size_t i = 0; i < Threads_.capacity(); ++i) {
            Threads_.emplace_back(&TThreadPool::WorkerLoop, this);
        }
    }

    void TThreadPool::Finish() noexcept {
        Wg_.Block();
        Wg_.Wait();
        JobsCV_.notify_all();

        for (auto& thread : Threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    bool TThreadPool::EnqueJob(std::unique_ptr<ITask> task) noexcept {
        if (!Wg_.Inc()) {
            return false;
        }
        std::scoped_lock lock {QueueMutex_};
        JobsQueue_.emplace(std::move(task));
        JobsCV_.notify_one();
        return true;
    }

    void TThreadPool::WorkerLoop() {
        while (true) {
            std::unique_lock lock {QueueMutex_};
            JobsCV_.wait(lock, [this] {
                return !JobsQueue_.empty() || Wg_.Waited();
            });

            if (Wg_.Waited()) {
                return;
            }

            auto taskPtr = std::move(JobsQueue_.front());
            JobsQueue_.pop();
            lock.unlock();
            taskPtr->Execute();
            Wg_.Dec();
        }
    }

} // namespace NAsync
