#include <io/well_known_structs.hpp>
#include <io/read_write_awaitable.hpp>
#include <system_error>
#include <util/result.hpp>

#include <fcntl.h>
#include <unistd.h>
#include <sys/eventfd.h>

#include <utility>
#include <tuple>

namespace NAsync {

    // TPipe
    TResult<TPipe> TPipe::Create() noexcept {
        int pipeFds[2];
        int status = pipe2(pipeFds, O_NONBLOCK);
        if (status == -1) {
            return std::error_code{errno, std::system_category()};
        }
        return TPipe(pipeFds[0], pipeFds[1]);
    }

    // TEventFd
    TEventFd::TEventFd() noexcept
        : TIoObject{eventfd(0, EFD_NONBLOCK)}
    {}

    void TEventFd::Set() noexcept {
        constexpr uint64_t numToWrite = 1;
        if (IsSet_.test_and_set()) { // someone already set value
            return;
        }
        VERIFY_RESULT(Write(&numToWrite, sizeof(numToWrite)).await_resume()); // write to eventfd always returns 8 bytes, so no need to check retval
    }

} // namespace NAsync
