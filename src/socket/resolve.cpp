#include <socket/resolve.hpp>
#include <socket/socket.hpp>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <cstring>

#include <variant>

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

    }

    struct TSockDescr::TAddrInfo: public addrinfo {
    };

    TSockDescr::TSockDescr(TAddrInfo&& addrInfo) noexcept
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
                Port_ = ntohs(addr->sin_port);
                src = &addr->sin_addr;
                domain = AF_INET;
            } else {
                sockaddr_in6* addr = static_cast<sockaddr_in6*>((void*)AddrInfo_->ai_addr);
                Port_ = ntohs(addr->sin6_port);
                src = &addr->sin6_addr;
                domain = AF_INET6;
            }

            const char* str = inet_ntop(domain, src, buffer, sizeof(buffer));
            VERIFY_SYSCALL(str != nullptr);
            StrAddr_ = str;
        }
    }

    TSockDescr::TSockDescr(TSockDescr&&) noexcept = default;
    TSockDescr& TSockDescr::operator=(TSockDescr&&) noexcept = default;
    TSockDescr::~TSockDescr() = default;

    TSocket TSockDescr::CreateSocket() && noexcept {
        return TSocket{std::move(*this)};
    }

    TResolveResult TResolver::ResolveSync(const char* node, const char* service, std::optional<EDomain> domain, std::optional<ESockType> type) noexcept {
        addrinfo* lookupResult;
        addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        if (domain) {
            hints.ai_family = FromDomain(*domain);
        }

        if (type) {
            hints.ai_socktype = FromSockType(*type);
        } else {
            hints.ai_socktype = AF_UNSPEC;
        }

        if (node == nullptr) {
            hints.ai_flags = AI_PASSIVE;
        }

        int status = getaddrinfo(node, service, &hints, &lookupResult);
        if (status != 0) {
            return std::error_code{status, resolveCategory};
        }

        TResolveResult::Type resolveResult;
        for (auto ai = lookupResult; ai != nullptr; ai = ai->ai_next) {
            TSockDescr descr {std::move(*static_cast<TSockDescr::TAddrInfo*>(ai))};
            resolveResult.emplace_back(std::move(descr));
        }
        freeaddrinfo(lookupResult);
        return std::move(resolveResult);
    }

} // namespace NAsync
