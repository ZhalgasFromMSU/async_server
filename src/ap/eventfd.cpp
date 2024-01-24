module;

#include <tl/expected.hpp>

#include <system_error>

#include <sys/eventfd.h>

export module async:eventfd;
import :ioobject;
import :awaitable;
import :operation;

namespace async {

  export class EventFd {
  public:
    static tl::expected<EventFd, std::error_code> Create() noexcept {
      int fd = eventfd(0, EFD_NONBLOCK);
      if (fd == -1) {
        return tl::unexpected{std::make_error_code(std::errc(errno))};
      }
      return EventFd{fd};
    }

    template<typename IoProvider>
    Awaitable auto Add(IoProvider& dispatcher,
                       const std::uint64_t& val) noexcept {
      return OperationAwaitable{
          dispatcher,
          BuildOp<OpType::kWrite>(arg::cbuffer{&val}, arg::fd{event_fd_.fd()},
                                  arg::count{sizeof(val)})};
    }

    template<typename IoProvider>
    Awaitable auto Reset(IoProvider& dispatcher, std::uint64_t& ret) noexcept {
      return OperationAwaitable{
          dispatcher,
          BuildOp<OpType::kRead>(arg::buffer{&ret}, arg::fd{event_fd_.fd()},
                                 arg::count{sizeof(ret)})};
    }

  private:
    explicit EventFd(int event_fd) noexcept
        : event_fd_{event_fd} {
    }

    IoObject event_fd_;
  };

} // namespace async
