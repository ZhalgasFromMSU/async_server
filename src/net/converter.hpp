#pragma once

#include <net/socket.hpp>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <cstring>

namespace NAsync::NPrivate {

    // Converting
    class TConverter {
    public:
        TConverter() noexcept {
            memset(&Raw_, 0, sizeof(Raw_));
        }

        TConverter(const TSocketAddress& address) noexcept
            : TConverter{}
        {
            if (auto ptr4 = std::get_if<TIPv4SocketAddress>(&address)) {
                auto rawPtr = PtrCast<sockaddr_in>(&Raw_);
                rawPtr->sin_addr = ptr4->first.Raw();
                rawPtr->sin_port = htons(ptr4->second);
                rawPtr->sin_family = AF_INET;

                Size_ = sizeof(sockaddr_in);
            } else if (auto ptr6 = std::get_if<TIPv6SocketAddress>(&address)) {
                auto rawPtr = PtrCast<sockaddr_in6>(&Raw_);
                rawPtr->sin6_addr = ptr6->first.Raw();
                rawPtr->sin6_port = htons(ptr6->second);
                rawPtr->sin6_family = AF_INET6;

                Size_ = sizeof(sockaddr_in6);
            } else {
                VERIFY(false);
            }
        }

        int Domain() const {
            return Raw_.ss_family;
        }

        sockaddr* MutRaw() noexcept {
            return PtrCast<sockaddr>(&Raw_);
        }

        const sockaddr* Raw() const noexcept {
            return PtrCast<sockaddr>(&Raw_);
        }

        socklen_t* MutSize() noexcept {
            return &Size_;
        }

        socklen_t Size() const noexcept {
            return Size_;
        }

        TSocketAddress FromRawStorage() const noexcept {
            if (Raw_.ss_family == AF_INET) {
                const auto* raw4 = PtrCast<sockaddr_in>(&Raw_);
                return TSocketAddress{
                    std::in_place_type<TIPv4SocketAddress>,
                    std::piecewise_construct,
                    std::forward_as_tuple(raw4->sin_addr),
                    std::forward_as_tuple(ntohs(raw4->sin_port))
                };
            } else if (Raw_.ss_family == AF_INET6) {
                const auto* raw6 = PtrCast<sockaddr_in6>(&Raw_);
                return TSocketAddress{
                    std::in_place_type<TIPv6SocketAddress>,
                    std::piecewise_construct,
                    std::forward_as_tuple(raw6->sin6_addr),
                    std::forward_as_tuple(ntohs(raw6->sin6_port))
                };
            } else {
                VERIFY(false);
            }
        }

    private:
        template<typename To, typename From>
        static std::conditional_t<std::is_const_v<From>, const To, To>* PtrCast(From* ptr) noexcept {
            if constexpr (std::is_const_v<From>) {
                return static_cast<const To*>(static_cast<const void*>(ptr));
            } else {
                return static_cast<To*>(static_cast<void*>(ptr));
            }
        }

        sockaddr_storage Raw_;
        socklen_t Size_ = sizeof(Raw_);
    };

} // namespace NAsync::NPrivate
