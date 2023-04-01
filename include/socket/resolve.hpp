#pragma once

#include "sock_types.hpp"
#include <util/result.hpp>

#include <vector>
#include <memory>
#include <optional>

namespace NAsync {

    class TResolver;
    class TSocket;

    class TSockDescr {
    public:
        TSockDescr(TSockDescr&&) noexcept;
        TSockDescr& operator=(TSockDescr&&) noexcept;
        ~TSockDescr();

        inline EDomain Domain() const noexcept {
            return Domain_;
        }

        inline ESockType Type() const noexcept {
            return Type_;
        }

        inline uint16_t Port() const noexcept {
            return Port_;
        }

        const std::string& StrAddr() const noexcept {
            return StrAddr_;
        }

        TSocket CreateSocket() && noexcept;

    private:
        friend class TResolver;
        friend class TSocket;

        struct TAddrInfo; // addrinfo

        TSockDescr(TAddrInfo&& addrInfo) noexcept;

        std::unique_ptr<TAddrInfo> AddrInfo_;
        EDomain Domain_;
        ESockType Type_;
        std::string StrAddr_;
        uint16_t Port_ = 0;
    };

    using TResolveResult = TResult<std::vector<TSockDescr>>;

    class TResolver {
    public:
        static TResolveResult ResolveSync(const char* node, const char* service, std::optional<EDomain> domain, std::optional<ESockType> sockType) noexcept;
    };

} // namespace NAsync