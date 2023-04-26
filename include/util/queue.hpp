#pragma once

#include <array>
#include <atomic>
#include <optional>

namespace NAsync {

    template<typename T, size_t MaxSize = 1024>
        requires (MaxSize > 0)
    class TQueue {
    public:
        template<typename... TArgs>
        bool Push(TArgs&&... args) {
            size_t place = MoveTail();
            if (place == BufSize) {
                return false;
            }
            RingBuffer_[place].emplace(std::forward<TArgs>(args)...);
            return true;
        }

        std::optional<T> Pop() {
            size_t place = MoveHead();
            if (place == BufSize) {
                return std::nullopt;
            }
            return std::move(RingBuffer_[place]);
        }

        // Head_ == Tail_ - there is no elements
        // Head_ - 1 == Tail_ - queue is at full capacity (although location under Tail_ can be used for emplace)
        size_t Size() const noexcept {
            size_t curHead = Head_;
            size_t curTail = Tail_;
            if (curTail >= curHead) {
                return curTail - curHead;
            } else {
                return MaxSize - curHead + curTail;
            }
        }

    private:
        size_t MoveTail() {
            size_t curVal = Tail_;
            while (true) {
                size_t newVal = (curVal == BufSize - 1 ? 0 : curVal + 1);
                if (Head_ == newVal) { // Head_ can only move towards tail, so between this check and actual cas, queue size can only decrease
                    return BufSize;
                }
                if (Tail_.compare_exchange_weak(curVal, newVal)) {
                    return curVal;
                }
            }
        }

        size_t MoveHead() {
            size_t curVal = Head_;
            while (true) {
                if (Tail_ == curVal) { // same logic applies here, after check succeeds, there is no way to break down
                    return BufSize;
                }
                size_t newVal = (curVal == 0 ? BufSize - 1 : curVal - 1);
                if (Head_.compare_exchange_weak(curVal, newVal)) {
                    return curVal;
                }
            }
        }

        static constexpr size_t BufSize = MaxSize + 1;
        std::array<std::optional<T>, BufSize> RingBuffer_;
        std::atomic<size_t> Head_ = 0;
        std::atomic<size_t> Tail_ = 0;
    };
} // namespace NAsync
