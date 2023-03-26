#pragma once

#include <util/result.hpp>

#include <vector>
#include <memory>

namespace NAsync {

    class IAddress { // sockaddr_in and sockaddr_in6 implement this interface
    public:
        virtual ~IAddress() = default;

        virtual std::string ToString() const = 0;

        // Pure bytes and sizeof underlying sockaddr
        virtual std::pair<const uint8_t*, size_t> ToBytes() const = 0;
    };

    using TResolveResult = TResult<std::vector<std::shared_ptr<IAddress>>>;

    TResolveResult ResolveSync(const char* node, const char* service) noexcept;
    // TODO Write async resolve (getaddrinfo_a)

} // namespace NAsync
