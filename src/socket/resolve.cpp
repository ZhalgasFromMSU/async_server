#include <socket/resolve.hpp>

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
            hints.ai_flags = AI_PASSIVE; // use any address instead of loopback
        }

        int status = getaddrinfo(node, service, &hints, &lookupResult);
        if (status != 0) {
            return std::error_code{status, resolveCategory};
        }

        TResolveResult::Type resolveResult;
        for (auto ai = lookupResult; ai != nullptr; ai = ai->ai_next) {
            TSockDescr descr {std::move(*ai)};
            resolveResult.emplace_back(std::move(descr));
        }
        freeaddrinfo(lookupResult);
        return std::move(resolveResult);
    }

} // namespace NAsync
