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
    Awaitable auto AddAsync(IoProvider& dispatcher,
                            const std::uint64_t& val) noexcept {
      return OperationAwaitable{
          dispatcher,
          BuildOp<OpType::kWrite>(arg::cbuffer{&val}, arg::fd{event_fd_.fd()},
                                  arg::count{sizeof(val)})};
    }

    void Add(std::uint64_t val) noexcept {
      int ret =
          OperationExecutable{BuildOp<OpType::kWrite>(arg::fd{event_fd_.fd()},
                                                      arg::cbuffer{&val},
                                                      arg::count{sizeof(val)})}
              .Execute();
      assert(ret == sizeof(val) && "Incomplete write");
    }

    template<typename IoProvider>
    Awaitable auto ResetAsync(IoProvider& dispatcher,
                              std::uint64_t& ret) noexcept {
      return OperationAwaitable{
          dispatcher,
          BuildOp<OpType::kRead>(arg::buffer{&ret}, arg::fd{event_fd_.fd()},
                                 arg::count{sizeof(ret)})};
    }

    std::uint64_t Reset() noexcept {
      std::uint64_t ret;
      int code =
          OperationExecutable{BuildOp<OpType::kRead>(arg::fd{event_fd_.fd()},
                                                     arg::buffer{&ret},
                                                     arg::count{sizeof(ret)})}
              .Execute();
      assert(code == sizeof(ret) && "Incomplete read");
      return ret;
    }

  private:
    explicit EventFd(int event_fd) noexcept
        : event_fd_{event_fd} {
    }

    IoObject event_fd_;
  };

  export template<typename IoProvider>
  class StopCrane {
  public:
    explicit StopCrane(IoProvider& dispatcher) noexcept
        : event_fd_{EventFd::Create().value()}
        , awaitable_{event_fd_.ResetAsync(dispatcher, res_)} {
      awaitable_.await_suspend({});
    }

    void PullStop() noexcept {
      event_fd_.Add(1);
    }

    bool RefersToThis(AwaitableBase* base) noexcept {
      return base == static_cast<AwaitableBase*>(&awaitable_);
    }

  private:
    EventFd event_fd_;
    uint64_t res_;
    decltype(event_fd_.ResetAsync(std::declval<IoProvider&>(),
                                  res_)) awaitable_;
  };

} // namespace async
