#pragma once

#include <util/result.hpp>

#include <future>

namespace NAsync::NPrivate {

    template<typename T>
    bool IsReady(std::future<T>& future) {
        auto res = future.wait_for(std::chrono::seconds::zero());
        VERIFY(res != std::future_status::deferred);
        return res == std::future_status::ready;
    }

} // namespace NAsync::NPrivate
