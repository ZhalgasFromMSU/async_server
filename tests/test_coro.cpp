#include <coro/coroutine.hpp>

#include <gtest/gtest.h>

using namespace NAsync;


struct Coro : testing::Test {
    TEpoll Epoll;

    Coro() {
        Epoll.Start();
    }
};


TEST_F(Coro, Coro) {
    auto coro = [](const TIoObject& pipeRead) -> TCoroFuture<TResult<int>> {
        char buf[4];
        co_return co_await pipeRead.Read(buf, 4);
    };

    TPipe pipe;
    auto task = coro(pipe.ReadEnd());
    task.SetEpoll(&Epoll);
    task.Run();

    ASSERT_FALSE(task.IsReady());
    pipe.WriteEnd().Write("1234", 4).await_resume();
    task.Wait();
    TResult<int> res = task.Get();
    ASSERT_TRUE(res) << res.Error().message();
    ASSERT_EQ(*res, 4);
}

//struct Coro: ::testing::Test {
    //TRuntime Runtime { 1 };

    //Coro() {
        //Epoll.Start();
        //ThreadPool.Start();
    //}
//};

//struct TScopeGuard {
    //TScopeGuard(std::function<void()> onDestructor)
        //: OnDestructor(std::move(onDestructor))
    //{}

    //~TScopeGuard() {
        //OnDestructor();
    //}

    //std::function<void()> OnDestructor;
//};

//TEST_F(Coro, Coro) {
    //auto coro = [](const TIoObject& pipeOut, TWaitGroup& wg) -> TCoroFuture<int> {
        //TScopeGuard guard{[&wg] { wg.Dec(); }};
        //char out[1];
        //auto numRead = co_await TReadAwaitable(pipeOut, out, 1);
        //co_return *numRead;
    //};

    //TWaitGroup wg;
    //TPipe pipe;

    //auto task = coro(pipe.ReadEnd(), wg);
    //task.SetEpoll(&Epoll);
    //task.SetThreadPool(&ThreadPool);
    //wg.Inc();

    //auto future = task.Run();
    //ASSERT_FALSE(NPrivate::IsReady(future));

    //ASSERT_EQ(*pipe.WriteEnd().Write("1", 1).await_resume(), 1);
    //ASSERT_TRUE(NPrivate::IsReady(future));
    //ASSERT_EQ(future.get(), 1);
//}

//TEST_F(Coro, VoidCoro) {
    //std::atomic<int> sharedNumRead;

    //auto coro = [&sharedNumRead](const TIoObject& pipeOut, TWaitGroup& wg) -> TCoroFuture<void> {
        //TScopeGuard guard{[&wg] { wg.Dec(); }};
        //char out[1];
        //auto numRead = co_await TReadAwaitable(pipeOut, out, 1);
        //sharedNumRead = *numRead;
        //co_return;
    //};

    //TWaitGroup wg;
    //TPipe pipe;

    //auto task = coro(pipe.ReadEnd(), wg);
    //task.SetEpoll(&Epoll);
    //task.SetThreadPool(&ThreadPool);
    //wg.Inc();

    //auto future = task.Run();
    //ASSERT_FALSE(NPrivate::IsReady(future));

    //ASSERT_EQ(*pipe.WriteEnd().Write("1", 1).await_resume(), 1);
    //ASSERT_TRUE(NPrivate::IsReady(future));
    //ASSERT_EQ(sharedNumRead, 1);
//}

//TEST_F(Coro, NestedCoro) {
    //auto coro = [this](const TIoObject& pipeOut, TWaitGroup& wg) -> TCoroFuture<int> {
        //TScopeGuard guard {[&wg] { wg.Dec(); }};

        //auto innerCoro = [](const TIoObject& pipeOut) -> TCoroFuture<int> {
            //char out[1];
            //co_return *(co_await TReadAwaitable(pipeOut, out, 1));
        //};

        //co_return co_await innerCoro(pipeOut);
    //};

    //TWaitGroup wg;
    //TPipe pipe;

    //auto task = coro(pipe.ReadEnd(), wg);
    //task.SetEpoll(&Epoll);
    //task.SetThreadPool(&ThreadPool);
    //wg.Inc();

    //auto future = task.Run();
    //wg.BlockAndWait();
    //ASSERT_FALSE(NPrivate::IsReady(future));

    //ASSERT_EQ(*pipe.WriteEnd().Write("1", 1).await_resume(), 1);
    //wg.BlockAndWait();
    //ASSERT_TRUE(NPrivate::IsReady(future));
    //ASSERT_EQ(future.get(), 1);
//}

//TEST_F(Coro, WithoutEpoll) {
    //auto coro = []() -> TCoroFuture<int> {
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        //co_return 1;
    //};

    //auto task = coro();
    //task.SetThreadPool(&ThreadPool);

    //auto future = task.Run();
    //ASSERT_FALSE(NPrivate::IsReady(future));
    //future.wait_for(std::chrono::milliseconds(1500));
    //ASSERT_TRUE(NPrivate::IsReady(future));
    //ASSERT_EQ(future.get(), 1);
//}

//TEST_F(Coro, WithoutThreadPool) {
    //auto coro = [](const TIoObject& pipeRead, TWaitGroup& wg) -> TCoroFuture<int> {
        //TScopeGuard guard{[&wg] { wg.Dec(); }};
        //char out[1];
        //co_return *(co_await TReadAwaitable(pipeRead, out, 1));
    //};

    //TWaitGroup wg;
    //auto pipe = TPipe::Create();
    //auto task = coro(pipe.ReadEnd(), wg);
    //task.SetEpoll(&Epoll);
    //wg.Inc();

    //auto future = task.Run();
    //ASSERT_FALSE(NPrivate::IsReady(future));
    //ASSERT_EQ(*pipe.WriteEnd().Write("1", 1).await_resume(), 1);
    //wg.BlockAndWait();
    //ASSERT_TRUE(NPrivate::IsReady(future));
    //ASSERT_EQ(future.get(), 1);
//}

//TEST_F(Coro, WithoutEpollAndThreadPool) {
    //auto coro = []() -> TCoroFuture<int> {
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        //co_return 1;
    //};

    //auto future = coro().Run();
    //ASSERT_TRUE(NPrivate::IsReady(future));
    //ASSERT_EQ(future.get(), 1);
//}

//TEST_F(Coro, NestedCoroWithoutEpollAndThreadPool) {
    //auto coro = []() -> TCoroFuture<int> {
        //auto nestedCoro = []() -> TCoroFuture<int> {
            //std::this_thread::sleep_for(std::chrono::milliseconds(100));
            //co_return 1;
        //};

        //co_return co_await nestedCoro();
    //};

    //auto future = coro().Run();
    //ASSERT_TRUE(NPrivate::IsReady(future));
    //ASSERT_EQ(future.get(), 1);
//}
