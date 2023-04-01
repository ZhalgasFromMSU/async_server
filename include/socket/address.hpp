#pragma once

#include <util/result.hpp>

#include <vector>
#include <memory>

namespace NAsync {

    class TIpAddress {
    public:
        explicit TIpAddress(std::string ipAddr) noexcept;

        inline bool IsIPv6() const noexcept {
            return IsIPv6_;
        }

        inline const std::string& ToString() const noexcept {
            return IpAddress_;
        }

    private:
        bool IsIPv6_;
        std::string IpAddress_;
    };

    // using TResolveResult = TResult<std::vector<std::shared_ptr<IAddress>>>;
    // const std::error_category& resolve_category() noexcept;

    // TResolveResult ResolveSync(const char* node, const char* service, EDomain domain) noexcept;
    // TODO Write async resolve (getaddrinfo_a)

} // namespace NAsync
