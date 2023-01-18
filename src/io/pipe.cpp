#include <io/pipe.hpp>
#include <util/result.hpp>

#include <fcntl.h>
#include <unistd.h>

#include <utility>
#include <tuple>

namespace NAsync {

    std::pair<TIoObject, TIoObject> CreatePipe() noexcept {
        int pipeFds[2];
        VERIFY_SYSCALL(pipe2(pipeFds, O_NONBLOCK) == 0);
        return std::pair<TIoObject, TIoObject>{
            std::piecewise_construct_t{},
            std::forward_as_tuple(pipeFds[0]),
            std::forward_as_tuple(pipeFds[1])
        };
    }

} // namespace NAsync