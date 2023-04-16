#include <socket/socket.hpp>
#include <socket/resolve.hpp>
#include <socket/accept.hpp>
#include <socket/connect.hpp>
#include <coro/coroutine.hpp>

#include <gtest/gtest.h>

using namespace NAsync;

TEST(Socket, AcceptConnect) {
    // auto descrs = TResolver::ResolveSync("::1", "9999", EDomain::kIPv6, ESockType::kTcp);
    // ASSERT_TRUE(descrs) << descrs.Error().message();

    // TResult<TSocket> socket = std::move(descrs->at(0)).CreateSocket();
    // VERIFY_RESULT(socket);

    // VERIFY_EC(socket->Bind());
    // VERIFY_EC(socket->Listen());

    // bool stopped = false;
    // auto server = [&stopped, &socket]() -> TCoroFuture<void> {
    //     while (!stopped) {
    //         TResult<TSocket> newSock = co_await socket->Accept();
    //         VERIFY_RESULT(newSock);

    //         char buffer[10];
    //         TResult<int> readRes = co_await TReadAwaitable(*newSock, buffer, sizeof(buffer));
    //         VERIFY_RESULT(readRes);
    //         buffer[*readRes] = 0;
    //         std::cerr << buffer << std::endl;
    //     }
    // };

    // TSockDescr descr {EDomain::kIPv6, ESockType::kTcp};
    // TResult<TSocket> writeSocket = std::move(descr).CreateSocket();
    // VERIFY_RESULT(writeSocket);

    // auto client = [&stopped, &writeSocket, &descrs]() -> TCoroFuture<void> {
    //     stopped = true;
    //     VERIFY_EC(co_await writeSocket->Connect(descrs->at(0)));
    //     TResult<int> writeRes = co_await TWriteAwaitable(*writeSocket, "123", 3);
    //     VERIFY_RESULT(writeRes);
    // };

    // TEpoll epoll;
    // TThreadPool threadPool {2};
    // threadPool.Start();
    // auto serverTask = server();
    // serverTask.SetEpoll(&epoll);
    // serverTask.SetThreadPool(&threadPool);
    // auto fut1 = serverTask.Run();

    // auto clientTask = client();
    // clientTask.SetEpoll(&epoll);
    // clientTask.SetThreadPool(&threadPool);
    // auto fut2 = clientTask.Run();

    // fut1.wait_for(std::chrono::seconds(1));
    // fut2.wait_for(std::chrono::seconds(1));
}