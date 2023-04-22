#pragma once

#include <memory>
#include <atomic>

namespace NAsync {

    template<typename T>
    class TLockFreeQueue {
    public:
        ~TLockFreeQueue() {
        }

        void Push(T value) noexcept {
            std::shared_ptr<TNode> newTail{new TNode{
                .Value = std::move(value),
                .Next = nullptr,
            }};

            while (Tail_->Next.compare_exchange_weak(nullptr, newTail)) {
            }

        }

        T Pop() noexcept;
        bool Empty() noexcept;

    private:
        struct TNode {
            T Value;
            std::atomic<std::shared_ptr<TNode>> Next;
        };

        std::atomic<std::shared_ptr<TNode>> Head_ = nullptr;
        std::atomic<std::shared_ptr<TNode>> Tail_ = nullptr;
    };

} // namespace NAsync
