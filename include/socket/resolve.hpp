#pragma once

#include <util/result.hpp>

#include <vector>
#include <memory>
#include <optional>

namespace NAsync {

    enum class EDomain {
        kIPv4,
        kIPv6,
        kUnix,
    };

    enum class ESockType {
        kTcp,
        kUdp,
    };

    class TResolver;

    class TSocketDescription {
    public:
        TSocketDescription(TSocketDescription&&) noexcept;
        TSocketDescription& operator=(TSocketDescription&&) noexcept;
        ~TSocketDescription();

        inline EDomain Domain() const noexcept {
            return Domain_;
        }

        inline ESockType Type() const noexcept {
            return Type_;
        }

        inline int Port() const noexcept {
            return Port_;
        }

        const std::string& StrAddr() const noexcept {
            return StrAddr_;
        }

    private:
        friend class TResolver;

        struct TAddrInfo; // addrinfo

        TSocketDescription(TAddrInfo&& addrInfo) noexcept;

        std::unique_ptr<TAddrInfo> AddrInfo_;
        EDomain Domain_;
        ESockType Type_;
        std::string StrAddr_;
        uint16_t Port_ = 0;
    };

    using TResolveResult = TResult<std::vector<TSocketDescription>>;

    class TResolver {
    public:

        static TResolveResult ResolveSync(const char* node, const char* service, std::optional<EDomain> domain, std::optional<ESockType> sockType) noexcept;
    };

} // namespace NAsync