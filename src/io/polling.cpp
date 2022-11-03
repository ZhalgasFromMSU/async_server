#include "polling.hpp"

#include <sys/epoll.h>

#include <unordered_map>

namespace NAsync {

    class TPoller::TEpollImpl {
    public:
        TEpollImpl()
            : Fd_(epoll_create1(0))
        {}

        void AddToRead(const IIoObject* ioObject, bool edgeTriggered) noexcept {
            DescriptorsMappings_[ioObject->GetFd()] = ioObject;
        }

        void RemoveFromRead() noexcept {

        }

    private:
        int Fd_;
        std::unordered_map<int, const IIoObject*> DescriptorsMappings_; // fd -> corresponding object
    };

} // namespace NAsync