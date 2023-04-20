#include "utils.hpp"
#include <net/socket.hpp>
#include <net/accept_connect_awiatable.hpp>
#include <io/read_write_awaitable.hpp>
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
    TThreadPool threadPool;
    threadPool.Start();

    auto serverTask = server();
    serverTask.SetEpoll(&epoll);
    serverTask.SetThreadPool(&threadPool);
    auto serverFuture = serverTask.Run();

    auto clientTask = client();
    clientTask.SetEpoll(&epoll);
    clientTask.SetThreadPool(&threadPool);
    auto clientFuture = clientTask.Run();

    serverFuture.wait_for(std::chrono::seconds(1));
    ASSERT_TRUE(NPrivate::IsReady(serverFuture));
    clientFuture.wait_for(std::chrono::seconds(1));
    ASSERT_TRUE(NPrivate::IsReady(clientFuture));
}

TEST(Socket, PingPong) {
    std::atomic<uint16_t> port = 0;
    auto server = [&port]() -> TCoroFuture<std::string> {
        TResult<TListeningSocket> acceptor = TListeningSocket::Create(std::make_pair(TIPv6Address::Localhost(), kAnyPort));
        port = std::get<TIPv6SocketAddress>(acceptor->Address()).second;
        port.notify_one();
        TResult<TSocket> socket = co_await acceptor->Accept();
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
        VERIFY_EC(co_await socket->Connect(std::make_pair(TIPv6Address::Localhost(), port.load())));

        char buf[] = "pingpong";
        size_t numWritten = 0;
        while (numWritten < sizeof(buf)) {
            auto written = co_await socket->Write(buf, sizeof(buf) - numWritten);
            numWritten += *written;
        }
    };

    TEpoll epoll;
    TThreadPool tp;
    tp.Start();

    auto serverTask = server();
    serverTask.SetEpoll(&epoll);
    serverTask.SetThreadPool(&tp);
    auto serverFuture = serverTask.Run();

    auto clientTask = client();
    clientTask.SetEpoll(&epoll);
    clientTask.SetThreadPool(&tp);
    auto clientFuture = clientTask.Run();

    clientFuture.wait_for(std::chrono::seconds(1));
    ASSERT_TRUE(NPrivate::IsReady(clientFuture));
    serverFuture.wait_for(std::chrono::seconds(1));
    ASSERT_TRUE(NPrivate::IsReady(serverFuture));

    ASSERT_TRUE(serverFuture.get() == "pingpong");
}

