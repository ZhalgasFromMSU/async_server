module;

#include <tl/expected.hpp>

#include <memory>
#include <system_error>

#include <sys/epoll.h>
#include <unistd.h>

export module async:epoll;
import :awaitable;
import :ioobject;
import :operation;
import :eventfd;

namespace async {

  export class Epoll {
  private:
    class ConstructionToken {
      friend Epoll;
      ConstructionToken() noexcept = default;
    };

  public:
    Epoll(ConstructionToken, int epoll_fd) noexcept
        : epoll_{epoll_fd}
        , stop_crane_{*this} {
    }

    Epoll(const Epoll&) = delete;
    Epoll(Epoll&&) = delete;
    Epoll& operator=(const Epoll&) = delete;
    Epoll& operator=(Epoll&&) = delete;

    static tl::expected<Epoll, std::error_code> Create() noexcept {
      int epoll_fd = epoll_create(1);
      if (epoll_fd < 0) {
        return tl::unexpected(std::make_error_code(std::errc(errno)));
      }

      return tl::expected<Epoll, std::error_code>{
          tl::in_place, ConstructionToken{}, epoll_fd};
    }

    void DispatchStop() noexcept {
      stop_crane_.PullStop();
    }

    bool PrepareDispatch(Awaitable auto& awaitable) noexcept {
      int res = awaitable.Execute();
      if (res < 0 && (res == -EWOULDBLOCK || res == -EAGAIN)) {
        return true;
      }
      awaitable.SetResult(res);
      return false;
    }

    std::error_code Dispatch(Awaitable auto& awaitable) noexcept {
      epoll_event event{
          .events = EPOLLET | EPOLLONESHOT,
          .data =
              epoll_data{
                  .ptr = &awaitable,
              },
      };

      if constexpr (constexpr OpType type = std::remove_reference_t<
                        decltype(awaitable.GetOp())>::op_type;
                    type == OpType::kRead) {
        event.events |= EPOLLIN;
      } else if constexpr (type == OpType::kWrite) {
        event.events |= EPOLLOUT;
      }

      int status = epoll_ctl(epoll_.fd(), EPOLL_CTL_ADD,
                             GetArg<arg::fd>(awaitable.GetOp()), &event);
      if (status < 0) {
        return std::make_error_code(std::errc(errno));
      }
      return {};
    }

    tl::expected<AwaitableBase*, std::error_code> Dequeue() noexcept {
      epoll_event events[1];
      int res = epoll_wait(epoll_.fd(), events, 1, -1);
      if (res == -1) {
        return tl::unexpected{std::make_error_code(std::errc(errno))};
      }

      for (std::size_t i = 0; i < res; ++i) {
        auto awaitable = static_cast<AwaitableBase*>(events[i].data.ptr);
        if (stop_crane_.RefersToThis(awaitable)) {
          return tl::unexpected{std::error_code{}};
        }
        awaitable->SetResult(awaitable->Execute());
        return awaitable;
      }
      return tl::unexpected{
          std::make_error_code(std::errc::resource_unavailable_try_again)};
    }

  private:
    IoObject epoll_;
    StopCrane<Epoll> stop_crane_;
  };

} // namespace async
