#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>

#include <iostream>


void print(void* addr) {
    auto saddr = static_cast<sockaddr_in*>(addr);
    char buffer[INET_ADDRSTRLEN];
    const char* str = inet_ntop(AF_INET, &saddr->sin_addr, buffer, sizeof(buffer));
    std::cerr << str << '\t' << ntohs(saddr->sin_port) << std::endl;
}

void print6(void* addr) {
    auto saddr = static_cast<sockaddr_in6*>(addr);
    char buffer[INET6_ADDRSTRLEN];
    const char* str = inet_ntop(AF_INET6, &saddr->sin6_addr, buffer, sizeof(buffer));
    std::cerr << str << '\t' << ntohs(saddr->sin6_port) << std::endl;
}

int main() {
    addrinfo hints;
    addrinfo* lookupResult;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    // hints.ai_flags = AI_NUMERICSERV;

    int status = getaddrinfo("localhost", "83", &hints, &lookupResult);
    if (status != 0) {
        std::cerr << gai_strerror(status) << std::endl;
    }

    for (auto ai = lookupResult; ai != nullptr; ai = ai->ai_next) {
        if (ai->ai_family == AF_INET) {
            print(ai->ai_addr);
        } else if (ai->ai_family == AF_INET6) {
            print6(ai->ai_addr);
        }
        std::cerr << ai->ai_family << std::endl;
        std::cerr << ai->ai_socktype << std::endl;
        std::cerr << ai->ai_protocol << std::endl;
    }
    freeaddrinfo(lookupResult);

    return 0;
}
