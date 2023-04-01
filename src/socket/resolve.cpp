#include <socket/resolve.hpp>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <cstring>

#include <variant>
#include <iostream>

namespace NAsync {

    namespace {
        class TResolveCategory: public std::error_category {
        public:
            const char* name() const noexcept override {
                return "resolve category";
            }

            std::string message(int aiErr) const override {
                if (aiErr != EAI_SYSTEM) {
                    return gai_strerror(aiErr);
                } else {
                    return std::error_code{errno, std::system_category()}.message();
                }
            }
        };

        const TResolveCategory resolveCategory{};

        int ConvertDomain(const std::optional<EDomain>& domain) noexcept {
            if (!domain) {
                return AF_INET | AF_INET6;
            } else if (*domain == EDomain::kIPv4) {
                return AF_INET;
            } else if (*domain == EDomain::kIPv6) {
                return AF_INET6;
            } else if (*domain == EDomain::kUnix) {
                return AF_UNIX;
            }
            VERIFY(false);
        }

        EDomain ToDomain(int domain) noexcept {
            if (domain == AF_INET) {
                return EDomain::kIPv4;
            } else if (domain == AF_INET6) {
                return EDomain::kIPv6;
            } else if (domain == AF_UNIX) {
                return EDomain::kUnix;
            }
            VERIFY(false);
        }

        int ConvertSockType(const std::optional<ESockType>& type) {
            if (!type) {
                return SOCK_STREAM | SOCK_DGRAM;
            } else if (*type == ESockType::kTcp) {
                return SOCK_STREAM;
            } else if (*type == ESockType::kUdp) {
                return SOCK_DGRAM;
            }
            VERIFY(false);
        }

        ESockType ToSockType(int type) {
            if (type == SOCK_STREAM) {
                return ESockType::kTcp;
            } else if (type == SOCK_DGRAM) {
                return ESockType::kUdp;
            }
            VERIFY(false);
        }
    }

    struct TSocketDescription::TAddrInfo: public addrinfo {
    };

    TSocketDescription::TSocketDescription(TAddrInfo&& addrInfo) noexcept
        : AddrInfo_{std::make_unique<TAddrInfo>(std::move(addrInfo))}
        , Domain_{ToDomain(AddrInfo_->ai_family)}
        , Type_{ToSockType(AddrInfo_->ai_socktype)}
    {
        if (Domain_ == EDomain::kUnix) {
            StrAddr_ = static_cast<sockaddr_un*>((void*)AddrInfo_->ai_addr)->sun_path;
        } else {
            char buffer[std::max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)];
            int domain;
            const void* src;
            if (Domain_ == EDomain::kIPv4) {
                sockaddr_in* addr = static_cast<sockaddr_in*>((void*)AddrInfo_->ai_addr);
                Port_ = addr->sin_port;
                src = &addr->sin_addr;
                domain = AF_INET;
            } else {
                sockaddr_in6* addr = static_cast<sockaddr_in6*>((void*)AddrInfo_->ai_addr);
                Port_ = addr->sin6_port;
                src = &addr->sin6_addr;
                domain = AF_INET6;
            }

            const char* str = inet_ntop(domain, src, buffer, sizeof(buffer));
            VERIFY_SYSCALL(str != nullptr);
            StrAddr_ = str;
        }
    }

    TSocketDescription::TSocketDescription(TSocketDescription&&) noexcept = default;
    TSocketDescription& TSocketDescription::operator=(TSocketDescription&&) noexcept = default;
    TSocketDescription::~TSocketDescription() = default;

    TResolveResult TResolver::ResolveSync(const char* node, const char* service, std::optional<EDomain> domain, std::optional<ESockType> type) noexcept {
        addrinfo* lookupResult;
        addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = ConvertDomain(domain);
        hints.ai_socktype = ConvertSockType(type);

        int status = getaddrinfo(node, service, &hints, &lookupResult);
        if (status != 0) {
            return std::error_code{status, resolveCategory};
        }

        TResolveResult::Type resolveResult;
        for (auto ai = lookupResult; ai != nullptr; ai = ai->ai_next) {
            if (ai->ai_family == AF_INET) {
                std::cerr << "Zdes\t" << static_cast<sockaddr_in*>((void*)ai->ai_addr)->sin_port << std::endl;
            } else if (ai->ai_family == AF_INET6) {
                std::cerr << "Zdes6\t" << static_cast<sockaddr_in6*>((void*)ai->ai_addr)->sin6_port << std::endl;
            }

            TSocketDescription descr {std::move(*static_cast<TSocketDescription::TAddrInfo*>(ai))};
            resolveResult.emplace_back(std::move(descr));
        }
        freeaddrinfo(lookupResult);
        return std::move(resolveResult);
    }

} // namespace NAsync