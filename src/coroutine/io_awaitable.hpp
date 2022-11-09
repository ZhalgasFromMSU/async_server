#pragma once

#include <coroutine>
#include <memory>

#include <io/io_base.hpp>
#include <thread_pool/pool.hpp>

namespace NAsync {

    struct TIoAwaitable {
        bool await_ready() {

        };

        std::unique_ptr<IIoObject> IoObject;
        TThreadPool Pool;
    };

} // namespace NAsync