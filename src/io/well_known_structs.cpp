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
    TPipe::TPipe() noexcept {
        int pipeFds[2];
        VERIFY_SYSCALL(pipe2(pipeFds, O_NONBLOCK) != -1);
        ReadEnd_ = TIoObject(pipeFds[0]);
        WriteEnd_ = TIoObject(pipeFds[1]);
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
