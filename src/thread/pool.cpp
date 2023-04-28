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

    void TThreadPool::Start() noexcept {
        for (size_t i = 0; i < Threads_.capacity(); ++i) {
            Threads_.emplace_back([this] {
                while (!Wg_.Waited()) {
                    std::optional<TJob> job = Jobs_.Pop();
                    if (job != std::nullopt) {
                        TaskCount_ -= 1;
                        Wg_.Dec();
                        job->Execute();
                    } else if (TaskCount_ == 0) {
                        TaskCount_.wait(0);
                    }
                }
            });
        }
    }

    void TThreadPool::Finish() noexcept {
        Wg_.Block();
        Wg_.Wait();

        TaskCount_ = 1; // TaskCount_ cannot decrease after Wg_.Wait() succeeded
        TaskCount_.notify_all();
    }

    bool TThreadPool::EnqueJob(TJob job) noexcept {
        if (!Wg_.Inc()) {
            return false;
        }
        TaskCount_ += 1;
        if (!Jobs_.Emplace(std::move(job))) {
            TaskCount_ -= 1;
            Wg_.Dec();
            return false;
        }
        TaskCount_.notify_one();
        return true;
    }

} // namespace NAsync
