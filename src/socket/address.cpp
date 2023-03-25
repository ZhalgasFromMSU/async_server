#include <socket/address.hpp>

#include <netdb.h>
#include <sys/socket.h>
#include <cstring>

namespace NAsync {

    TResult<std::vector<TAddress>> ResolveSync(const char* node, const char* service) noexcept {
        std::vector<TAddress> result;
        addrinfo** lookupResult;

        int status = getaddrinfo(node, node, nullptr, lookupResult);
    }

} // namespace NAsync
