module;

#include <tl/expected.hpp>

#include <system_error>

#include <fcntl.h>
#include <unistd.h>

export module async:pipe;
import :ioobject;
import :awaitable;
import :operation;

namespace async {

  export class Pipe {
  public:
    static tl::expected<Pipe, std::error_code> Create() noexcept {
      int fd[2];
      if (pipe(fd) == -1) {
        return tl::unexpected(std::make_error_code(std::errc(errno)));
      }

      Pipe pipe{fd[0], fd[1]};

      // pipe2 is only available on Linux
      if (fcntl(fd[0], F_SETFL, O_NONBLOCK) != 0) {
        return tl::unexpected(std::make_error_code(std::errc(errno)));
      }

      if (fcntl(fd[1], F_SETFL, O_NONBLOCK) != 0) {
        return tl::unexpected(std::make_error_code(std::errc(errno)));
      }

      return std::move(pipe);
    }

    template<typename IoProvider>
    Awaitable auto Read(IoProvider& dispatcher, void* buffer,
                        std::size_t num) noexcept {
      return OperationAwaitable{dispatcher,
                                BuildOp<OpType::kRead>(arg::buffer{buffer},
                                                       arg::fd{read_end_.fd()},
                                                       arg::count{num})};
    }

    template<typename IoDispatcher>
    Awaitable auto Write(IoDispatcher& dispatcher, const void* buffer,
                         std::size_t num) noexcept {
      return OperationAwaitable{
          dispatcher,
          BuildOp<OpType::kWrite>(arg::cbuffer{buffer},
                                  arg::fd{write_end_.fd()}, arg::count{num})};
    }

  private:
    Pipe(int read_fd, int write_fd) noexcept
        : read_end_{read_fd}
        , write_end_{write_fd} {
    }

    IoObject read_end_;
    IoObject write_end_;
  };

} // namespace async
