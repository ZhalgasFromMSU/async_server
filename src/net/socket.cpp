#include <polling/epoll.hpp>

#include <net/socket.hpp>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <cstring>
#include <system_error>

namespace NAsync {

    namespace {

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
    
    } // namespace

    // TListeningSocket
    TResult<TListeningSocket> TListeningSocket::Create(TSocketAddress address, int queueSize) noexcept {
        TConverter cv{address};

        int sockFd = socket(cv.Domain(), SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (sockFd == -1) {
            return std::error_code{errno, std::system_category()};
        }

        int bindRes = bind(sockFd, cv.Raw(), cv.Size());
        if (bindRes == -1) {
            VERIFY(close(sockFd) == 0);
            return std::error_code{errno, std::system_category()};
        }

        TConverter cv2;
        int gsnRes = getsockname(sockFd, cv2.MutRaw(), cv2.MutSize());
        if (gsnRes == -1) {
            VERIFY(close(sockFd) == 0);
            return std::error_code{errno, std::system_category()};
        }

        TSocketAddress actualAddress = cv2.FromRawStorage();

        int listenRes = listen(sockFd, queueSize);
        if (listenRes == -1) {
            VERIFY(close(sockFd) == 0);
            return std::error_code{errno, std::system_category()};
        }

        return TListeningSocket{sockFd, std::move(actualAddress)};
    }

    TAcceptAwaitable TListeningSocket::Accept() const noexcept {
        return TAcceptAwaitable{*this};
    }

    // TSocket
    template<>
    TResult<TSocket> TSocket::Create<TIPv4Address>(bool streamSocket) noexcept {
        int sockFd = socket(AF_INET, (streamSocket ? SOCK_STREAM : SOCK_DGRAM) | SOCK_NONBLOCK, 0);
        if (sockFd == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return TSocket{sockFd};
    }

    template<>
    TResult<TSocket> TSocket::Create<TIPv6Address>(bool streamSocket) noexcept {
        int sockFd = socket(AF_INET6, (streamSocket ? SOCK_STREAM : SOCK_DGRAM) | SOCK_NONBLOCK, 0);
        if (sockFd == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return TSocket{sockFd};
    }

    TConnectAwaitable TSocket::Connect(TSocketAddress dest) noexcept {
        Remote_.emplace(std::move(dest));
        return TConnectAwaitable{*this};
    }

    // TAcceptAwaitable
    bool TAcceptAwaitable::await_ready() noexcept {
        TConverter cv;
        int sockFd = accept4(Acceptor_.Fd(), cv.MutRaw(), cv.MutSize(), SOCK_NONBLOCK);
        if (sockFd == -1) {
            if (errno & (EAGAIN | EWOULDBLOCK)) {
                return false;
            }
            Socket_.emplace(std::error_code{errno, std::system_category()});
        } else {
            Socket_.emplace(TSocket{sockFd, cv.FromRawStorage()});
        }
        return true;
    }

    bool TAcceptAwaitable::await_suspend(std::coroutine_handle<> handle) noexcept {
        return !Epoll_->Watch(TEpoll::EMode::kRead, Acceptor_, handle);
    }

    TResult<TSocket> TAcceptAwaitable::await_resume() noexcept {
        if (Socket_) {
            return std::move(*Socket_);
        }

        TConverter cv;
        int sockFd = accept4(Acceptor_.Fd(), cv.MutRaw(), cv.MutSize(), SOCK_NONBLOCK);
        if (sockFd == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return TSocket{sockFd, cv.FromRawStorage()};
    }

    // TConnectAwaitable
    bool TConnectAwaitable::await_ready() noexcept {
        TConverter converter{*Socket_.RemoteAddress()};
        int status = connect(Socket_.Fd(), converter.Raw(), converter.Size());
        if (status == -1) {
            if (errno & (EINPROGRESS | EAGAIN)) {
                return false;
            }
            Error_ = std::error_code{errno, std::system_category()};
        } else {
            Error_ = std::error_code{};
        }
        return true;
    }

    bool TConnectAwaitable::await_suspend(std::coroutine_handle<> handle) noexcept {
        return !Epoll_->Watch(TEpoll::EMode::kWrite, Socket_, handle);
    }

    std::error_code TConnectAwaitable::await_resume() noexcept {
        if (Error_) {
            return std::move(*Error_);
        }
        int acceptResult;
        socklen_t size = sizeof(acceptResult);
        int status = getsockopt(Socket_.Fd(), SOL_SOCKET, SO_ERROR, &acceptResult, &size);
        if (status == -1) {
            return std::error_code{errno, std::system_category()};
        }

        if (acceptResult != 0) {
            return std::error_code{acceptResult, std::system_category()};
        }

        return std::error_code{};
    }

} // namespace NAsync
