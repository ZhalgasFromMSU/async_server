#include "socket.hpp"

#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h>

#include <iostream>

namespace NAsync {

    TSocket::TSocket(int fd) noexcept
        : Fd_(fd)
    {}

    TSocket::~TSocket() noexcept {
        close(Fd_);
    }

    int TSocket::GetFd() const noexcept {
        return Fd_;
    }

    std::optional<TSocket> TSocket::Connect() noexcept {
        int fd = socket()
        return TSocket(0);
    }

    TServerManager::TServerManager(int port, TSocketOptions options) noexcept {
        Fd_ = socket(
            options.IpV6 ? AF_INET6 : AF_INET,
            options.UDP ? SOCK_DGRAM : SOCK_STREAM,
            0
        );
        if (Fd_ < 0) {
            std::cerr << "can't create socket, code: " << errno << std::endl;
            std::terminate();
        }

        if (options.IpV6) {
            std::cerr << "ipv6 not supported yet" << std::endl;
            std::terminate();
            sockaddr_in6 addr;
            addr.sin6_family = AF_INET6;
            addr.sin6_port = htons(port);
            addr.sin6_flowinfo = 0;
            addr.sin6_scope_id = 0;
        } else {
            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = INADDR_ANY;
            int status = bind(Fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
            if (status < 0) {
                std::cerr << "couldn't bind, code: " << errno << std::endl;
                std::terminate();
            }
        }

        if (!options.UDP) {
            int status = listen(Fd_, ConnectionQueueSize_);
            if (status < 0) {
                std::cerr << "listen on port failed, code: " << errno << std::endl;
            }
        }
    }

    TServerManager::~TServerManager() noexcept {
        int status = shutdown(Fd_, SHUT_RDWR);
        if (status < 0) {
            std::cerr << "shutdown failed" << std::endl;
            std::terminate();
        }
    }

    TSocket TServerManager::AcceptConnection(bool returnNonBlocking) const noexcept {
        int fd = accept4(Fd_, nullptr, nullptr, SOCK_NONBLOCK);
        return TSocket(fd);
    }

} // namespace NAsync