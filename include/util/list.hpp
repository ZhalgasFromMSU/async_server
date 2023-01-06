#pragma once

namespace NAsync {

    // Elements can be deleted using pointers (rather than iterators), which will come in handy with epoll
    // TODO Make it lock-free
    template<typename T>
    class TList {
    public:
        class TNode {
        public:
            TNode(const T& val)
                : Val_(val)
            {}

            TNode(T&& val)
                : Val_(std::move(val))
            {}

            T& Val() noexcept {
                return Val_;
            }

            const T& Val() const noexcept {
                return Val_;
            }

        private:
            friend TList;

            T Val_;
            TNode* Prev_ = nullptr;
            TNode* Next_ = nullptr;
        };

        ~TList() noexcept {
            while (Head_ != nullptr) {
                auto nextPtr = Head_->Next_;
                delete Head_;
                Head_ = nextPtr;
            }
        }

        size_t size() const noexcept {
            return Size_;
        }

        // No const T& push_back, because I don't use it
        void push_back(T val) {
            TNode* newNode = new TNode(std::move(val));
            if (Tail_ == nullptr) {
                Head_ = Tail_ = newNode;
            } else {
                newNode->Prev_ = Tail_;
                Tail_->Next_ = newNode;
                Tail_ = newNode;
            }

            Size_ += 1;
        }

        void erase(TNode* ptr) {
            if (ptr->Prev_ != nullptr) {
                ptr->Prev_->Next_ = ptr->Next_;
            }

            if (ptr->Next_ != nullptr) {
                ptr->Next_->Prev_ = ptr->Prev_;
            }

            if (ptr == Tail_) {
                Tail_ = ptr->Prev_;
            }

            if (ptr == Head_) {
                Head_ = ptr->Next_;
            }

            delete ptr;
            Size_ -= 1;
        }

        TNode* tail_pointer() noexcept {
            return Tail_;
        }

    private:
        size_t Size_ = 0;
        TNode* Head_ = nullptr;
        TNode* Tail_ = nullptr;
    };

} // namespace NAsync