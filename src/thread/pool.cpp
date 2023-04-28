#include <thread/pool.hpp>

namespace NAsync {

    TThreadPool::TThreadPool(size_t numThreads) noexcept {
        Threads_.reserve(numThreads);
    }

    TThreadPool::~TThreadPool() {
        if (!Wg_.Waited()) {
            Finish();
        }

        for (auto& thread : Threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    void TThreadPool::Start() noexcept {
        for (size_t i = 0; i < Threads_.capacity(); ++i) {
            Threads_.emplace_back([this] {
                while (!Wg_.Waited()) {
                    std::optional<TJob> job = Jobs_.Pop();
                    if (job != std::nullopt) {
                        job->Execute();
                        Wg_.Dec();
                    }
                }
            });
        }
    }

    void TThreadPool::Finish() noexcept {
        Wg_.Block();
        Wg_.Wait();
    }

    bool TThreadPool::EnqueJob(TJob job) noexcept {
        if (!Wg_.Inc()) {
            return false;
        }
        if (!Jobs_.Emplace(std::move(job))) {
            Wg_.Dec();
            return false;
        }
        return true;
    }

} // namespace NAsync
