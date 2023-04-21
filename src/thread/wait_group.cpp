#include <thread/wait_group.hpp>
#include <util/result.hpp>

#include <mutex>

namespace NAsync {

    void TWaitGroup::Add(int delta) noexcept {
        std::scoped_lock lock{Mutex_};
        Counter_ += delta;
        if (Counter_ == 0) {
            CondVar_.notify_all();
            return;
        }
        VERIFY(Counter_ >= 0);
    }

    void TWaitGroup::Done() noexcept {
        Add(-1);
    }

    void TWaitGroup::Wait() noexcept {
        // We cannot simply WaitFor(std::chrono::duration::max()), because of overflow in cond_var.wait_until(now() + timeout)
        std::unique_lock lock{Mutex_};
        CondVar_.wait(lock, [this] {
            return Counter_ == 0;
        });
    }

    bool TWaitGroup::WaitFor(std::chrono::microseconds timeout) noexcept {
        std::unique_lock lock{Mutex_};
        return CondVar_.wait_for(lock, timeout, [this] {
            return Counter_ == 0;
        });
    }

} // namespace NAsync
