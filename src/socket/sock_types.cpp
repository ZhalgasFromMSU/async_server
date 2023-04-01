#include <socket/sock_types.hpp>
#include <util/result.hpp>

#include <sys/socket.h>


namespace NAsync {
    int FromDomain(EDomain domain) noexcept {
        if (domain == EDomain::kIPv4) {
            return AF_INET;
        } else if (domain == EDomain::kIPv6) {
            return AF_INET6;
        } else if (domain == EDomain::kUnix) {
            return AF_UNIX;
        }
        VERIFY(false);
    }

    EDomain ToDomain(int domain) noexcept {
        if (domain == AF_INET) {
            return EDomain::kIPv4;
        } else if (domain == AF_INET6) {
            return EDomain::kIPv6;
        } else if (domain == AF_UNIX) {
            return EDomain::kUnix;
        }
        VERIFY(false);
    }

    int FromSockType(ESockType type) noexcept {
        if (type == ESockType::kTcp) {
            return SOCK_STREAM;
        } else if (type == ESockType::kUdp) {
            return SOCK_DGRAM;
        } else if (type == ESockType::kRaw) {
            return SOCK_RAW;
        }
        VERIFY(false);
    }

    ESockType ToSockType(int type) noexcept {
        if (type == SOCK_STREAM) {
            return ESockType::kTcp;
        } else if (type == SOCK_DGRAM) {
            return ESockType::kUdp;
        } else if (type == SOCK_RAW){
            return ESockType::kRaw;
        }
        VERIFY(false);
    }

} // namespace NAsync
