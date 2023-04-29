#pragma once

namespace NAsync {

    class TEpoll;

    class TWithEpoll {
    public:
        inline bool HasEpoll() {
            return Epoll_ != nullptr;
        }

        inline void SetEpoll(TEpoll* epoll) {
            Epoll_ = epoll;
        }

    protected:
        TEpoll* Epoll_ = nullptr;
    };

} // namespace NAsync
