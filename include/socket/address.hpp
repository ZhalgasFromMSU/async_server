#pragma once

#include <util/result.hpp>

#include <vector>

namespace NAsync {

    class TAddress {
    public:

    private:

    };

    TResult<std::vector<TAddress>> ResolveSync(const char* node, const char* service) noexcept;
    // TODO Write async resolve (getaddrinfo_a)

} // namespace NAsync
