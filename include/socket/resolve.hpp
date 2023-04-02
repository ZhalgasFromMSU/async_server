#pragma once

#include "socket_descr.hpp"
#include <util/result.hpp>

#include <vector>
#include <optional>

namespace NAsync {

    using TResolveResult = TResult<std::vector<TSockDescr>>;

    class TResolver {
    public:
        static TResolveResult ResolveSync(const char* node, const char* service, std::optional<EDomain> domain, std::optional<ESockType> sockType) noexcept;
    };

} // namespace NAsync