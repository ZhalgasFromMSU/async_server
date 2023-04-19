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

        // Static factories
        static TResult<TIPv4Address> FromString(const std::string& s) noexcept;

        static inline TIPv4Address Localhost() noexcept {
            return *FromString("127.0.0.1");
        }

        static inline TIPv4Address Any() noexcept {
            return *FromString("0.0.0.0");
        }

        // Insights
        inline const in_addr& Raw() const noexcept {
            return *Raw_;
        }

        std::string ToString() const noexcept override;

    private:
        std::shared_ptr<in_addr> Raw_;
    };

    class TIPv6Address : public IAddress {
    public:
        // Constructors
        explicit TIPv6Address(in6_addr rawAddr) noexcept;

        // Static factories
        static TResult<TIPv6Address> FromString(const std::string& s) noexcept;

        static inline TIPv6Address Localhost() noexcept {
            return *FromString("::1");
        }

        static inline TIPv6Address Any() noexcept {
            return *FromString("::");
        }

        // Insights
        inline const in6_addr& Raw() const noexcept {
            return *Raw_;
        }

        std::string ToString() const noexcept override;

    private:
        std::shared_ptr<in6_addr> Raw_;
    };

} // namespace NAsync
