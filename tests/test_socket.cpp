#include "utils.hpp"
#include <net/socket.hpp>
#include <coro/coroutine.hpp>

#include <gtest/gtest.h>

#include <sys/socket.h>

using namespace NAsync;

TEST(Socket, CreateSocket) {
    auto res = TListeningSocket::Create(std::make_pair(TIPv6Address::Localhost(), kAnyPort));

    ASSERT_TRUE(res) << res.Error().message();

    int val;
    socklen_t size = sizeof(val);
    VERIFY_SYSCALL(getsockopt(res->Fd(), SOL_SOCKET, SO_ACCEPTCONN, &val, &size) == 0);
    ASSERT_EQ(val, 1);
}

TEST(Socket, AcceptConnect) {
    std::atomic<uint16_t> port = 0;
    auto server = [&port]() -> TCoroFuture<void> {
        auto acceptor = TListeningSocket::Create(std::make_pair(TIPv6Address::Localhost(), kAnyPort));
        VERIFY_RESULT(acceptor);
        port = std::get<TIPv6SocketAddress>(acceptor->Address()).second;
        port.notify_one();

        auto socket = co_await acceptor->Accept();
        VERIFY_RESULT(socket);
        co_return;
    };

    auto client = [&port]() -> TCoroFuture<void> {
        auto socket = TSocket::Create<TIPv6Address>(true /* streamingSocket */);
        VERIFY_RESULT(socket);

        port.wait(0);
        std::error_code error = co_await socket->Connect(std::make_pair(TIPv6Address::Localhost(), port.load()));
        VERIFY_EC(error);
        co_return;
    };

    TEpoll epoll;
    epoll.Start();

    TCoroFuture<void> serverTask = server();
    serverTask.SetEpoll(&epoll);
    serverTask.Run();

    TCoroFuture<void> clientTask = client();
    clientTask.SetEpoll(&epoll);
    clientTask.Run();

    for (int i = 0; i < 100; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (serverTask.IsReady() && clientTask.IsReady()) {
            break;
        }
    }
    ASSERT_TRUE(serverTask.IsReady());
    ASSERT_TRUE(clientTask.IsReady());
}

TEST(Socket, PingPong) {
    std::atomic<uint16_t> port = 0;
    auto server = [&port]() -> TCoroFuture<std::string> {
        TResult<TListeningSocket> acceptor = TListeningSocket::Create(std::make_pair(TIPv6Address::Localhost(), kAnyPort));
        port = std::get<TIPv6SocketAddress>(acceptor->Address()).second;
        port.notify_one();
        TResult<TSocket> socket = co_await acceptor->Accept();
        VERIFY_RESULT(socket);
        char buf[sizeof("pingpong")];
        size_t numRead = 0;
        while (numRead < sizeof(buf)) {
            auto read = co_await socket->Read(buf, sizeof(buf) - numRead);
            numRead += *read;
        }
        co_return std::string(buf);
    };

    auto client = [&port]() -> TCoroFuture<void> {
        TResult<TSocket> socket = TSocket::Create<TIPv6Address>(true /* streamingSocket */);
        port.wait(0);
        std::error_code error = co_await socket->Connect(std::make_pair(TIPv6Address::Localhost(), port.load()));
        VERIFY_EC(error);
        char buf[] = "pingpong";
        size_t numWritten = 0;
        while (numWritten < sizeof(buf)) {
            auto written = co_await socket->Write(buf, sizeof(buf) - numWritten);
            numWritten += *written;
        }
    };

    TEpoll epoll;
    epoll.Start();

    TCoroFuture<std::string> serverTask = server();
    serverTask.SetEpoll(&epoll);
    serverTask.Run();

    TCoroFuture<void> clientTask = client();
    clientTask.SetEpoll(&epoll);
    clientTask.Run();

    for (int i = 0; i < 100; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (serverTask.IsReady() && clientTask.IsReady()) {
            break;
        }
    }
    ASSERT_TRUE(*serverTask.Peek() == "pingpong");
}

