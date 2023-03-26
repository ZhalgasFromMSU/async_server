#include <socket/address.hpp>

#include <netdb.h>
#include <sys/socket.h>
#include <cstring>

namespace NAsync {

    namespace {
        class TIpV6Address: public IAddress {
        public:

            std::pair<const uint8_t*, size_t> ToBytes() const override {
                return std::make_pair(&Addr_, sizeof(Addr_));
            }

        private:
            sockaddr_in6 Addr_;
        };

        class TIpV4Address: public IAddress {
        public:

            std::pair<const uint8_t*, size_t> ToBytes() const override {
                return std::make_pair(&Addr_, sizeof(Addr_));
            }

        private:
            sockaddr_in Addr_;
        };
    }

    TResolveResult ResolveSync(const char* node, const char* service) noexcept {
        TResolveResult result;
        addrinfo** lookupResult = nullptr;

        int status = getaddrinfo(node, node, nullptr, lookupResult);
        return result;
    }

} // namespace NAsync
