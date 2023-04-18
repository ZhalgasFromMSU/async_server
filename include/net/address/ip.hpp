#pragma once

#include <util/result.hpp>

#include <string>
#include <memory>

struct in_addr;
struct in6_addr;

namespace NAsync {

    class IAddress {
    public:
        virtual ~IAddress() = default;
        virtual std::string ToString() const noexcept = 0;
    };

    class TIPv4Address : public IAddress {
    public:
        // Constructors
        explicit TIPv4Address(in_addr rawAddr) noexcept;
        TIPv4Address(const TIPv4Address&) = delete;
        TIPv4Address& operator=(const TIPv4Address&) = delete;
        TIPv4Address(TIPv4Address&&) noexcept;
        TIPv4Address& operator=(TIPv4Address&&) noexcept;
        ~TIPv4Address();

        // Static factories
        static TResult<TIPv4Address> FromString(const std::string& s) noexcept;
        static TIPv4Address Localhost() noexcept;
        static TIPv4Address Any() noexcept;

        // Insights
        const in_addr& Raw() const noexcept;
        std::string ToString() const noexcept override;

    private:
        std::unique_ptr<in_addr> RawAddr_;
    };

    class TIPv6Address : public IAddress {
    public:
        // Constructors
        explicit TIPv6Address(in6_addr rawAddr) noexcept;
        TIPv6Address(const TIPv6Address&) = delete;
        TIPv6Address& operator=(const TIPv6Address&) = delete;
        TIPv6Address(TIPv6Address&&) noexcept;
        TIPv6Address& operator=(TIPv6Address&&) noexcept;
        ~TIPv6Address();

        // Static factories
        static TResult<TIPv6Address> FromString(const std::string& s) noexcept;
        static TIPv6Address Localhost() noexcept;
        static TIPv6Address Any() noexcept;

        // Insights
        const in6_addr& Raw() const noexcept;
        std::string ToString() const noexcept override;

    private:
        std::unique_ptr<in6_addr> RawAddr_;
    };

} // namespace NAsync