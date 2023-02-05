#include <io/well_known_structs.hpp>
#include <util/result.hpp>

#include <fcntl.h>
#include <unistd.h>
#include <sys/eventfd.h>

#include <utility>
#include <tuple>

namespace NAsync {

    namespace {
        int CreateEventFd() noexcept {
            int status = eventfd(0, EFD_NONBLOCK);
            VERIFY_SYSCALL(status >= 0);
            return status;
        }
    }

    TPipe::TPipe(int readFd, int writeFd) noexcept
        : WriteEnd_{writeFd}
        , ReadEnd_{readFd}
    {}

    TPipe TPipe::Create() noexcept {
        int pipeFds[2];
        VERIFY_SYSCALL(pipe2(pipeFds, O_NONBLOCK) == 0);
        return TPipe(pipeFds[0], pipeFds[1]);
    }

    const TIoObject& TPipe::ReadEnd() const noexcept {
        return ReadEnd_;
    }

    const TIoObject& TPipe::WriteEnd() const noexcept {
        return WriteEnd_;
    }

    TEventFd::TEventFd() noexcept
        : TIoObject(CreateEventFd())
    {}

    bool TEventFd::IsSet() const noexcept {
        return IsSet_;
    }

    void TEventFd::Set() noexcept {
        constexpr uint64_t numToWrite = 1;
        IsSet_ = true;
        VERIFY_RESULT(Write(*this, &numToWrite, sizeof(numToWrite))); // write to eventfd always returns 8 bytes, so no need to check retval
    }

    void TEventFd::Reset() noexcept {
        uint64_t numToRead;
        IsSet_ = false;
        VERIFY_RESULT(Read(*this, &numToRead, sizeof(numToRead))); // read from eventfd always returns 8 bytes
    }

} // namespace NAsync