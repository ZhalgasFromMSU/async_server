#pragma once

#include <array>
#include <atomic>
#include <optional>

namespace NAsync {

    template<typename T, size_t MaxSize>
        requires (MaxSize > 0)
    class TQueue {
    public:
        //
        //    Almost atomic queue. It has no locking, but it can fail spuriously in following cases
        //        1) Tail_ approaches Head_ from behind (possible beacuse it is ring buffer). Pop() was called right before Push()
        //           Head_ moved one step forward, but optional, containing value didn't destroy object. In this case new element can't be added, although
        //           we will have enough space to hold it just in a moment
        //        2) Same with opposite situation. Head_ approaches Tail_, but it can't place new object yet, because previous call to Pop() didn't finish
        //
        //    Head_ ->            Tail_ ->
        //      |                   |
        // nil val val val val val nil nil nil
        //
        template<typename... TArgs>
        bool Push(TArgs&&... args) { // put element to the end of a queue
            size_t place = MoveTail();
            if (place == BufSize) {
                return false;
            }
            RingBuffer_[place].emplace(std::forward<TArgs>(args)...);
            ContainsValue_[place] = true;
            return true;
        }

        std::optional<T> Pop() { // pop element from top of the queue
            size_t place = MoveHead();
            if (place == BufSize) {
                return std::nullopt;
            }
            std::optional<T> ret = std::move(RingBuffer_[place]);
            RingBuffer_[place].reset();
            ContainsValue_[place] = false;
            return ret;
        }

        // Head_ == Tail_ - there is no elements
        // Head_ - 1 == Tail_ - queue is at full capacity (although location under Tail_ can be used for emplace)
        size_t Size() const noexcept {
            size_t curHead = Head_;
            size_t curTail = Tail_;
            if (curTail >= curHead) {
                return curTail - curHead;
            } else {
                return BufSize - curHead + curTail;
            }
        }

    private:
        size_t MoveTail() {
            size_t curVal = Tail_;
            while (true) {
                size_t newVal = (curVal + 1) % BufSize;
                if (Head_ == newVal || ContainsValue_[curVal]) {
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
                if (Tail_ == curVal || !ContainsValue_[curVal]) {
                    return BufSize;
                }
                size_t newVal = (curVal + 1) % BufSize;
                if (Head_.compare_exchange_weak(curVal, newVal)) {
                    return curVal;
                }
            }
        }

        static constexpr size_t BufSize = MaxSize + 1;
        std::array<std::atomic<bool>, BufSize> ContainsValue_;
        std::array<std::optional<T>, BufSize> RingBuffer_;
        std::atomic<size_t> Head_ = 0;
        std::atomic<size_t> Tail_ = 0;
    };
} // namespace NAsync
