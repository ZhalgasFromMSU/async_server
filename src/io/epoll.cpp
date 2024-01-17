module;

#include <tl/expected.hpp>

#include <system_error>

#include <sys/epoll.h>
#include <unistd.h>

export module async:epoll;
import :operation;
import :ioobject;
import :queue;

namespace async {

  export class Epoll {
  private:
    class ConstructionToken {
      friend Epoll;
      ConstructionToken() noexcept = default;
    };

  public:
    Epoll(ConstructionToken, int epoll_fd) noexcept
        : epoll_{epoll_fd} {
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

    template<OpType type, OpDescr Descr>
    std::error_code Enqueue(Descr&& op, uint64_t user_data) noexcept;

    tl::expected<OpResult, std::error_code> Dequeue() noexcept;

  private:
    class OperationInterface;

    template<OpType type, OpDescr Descr>
    class Operation;

    static constexpr std::size_t queue_size_ = 1000;
    IoObject epoll_;
    Queue<OpResult> results_{queue_size_, 1 /* num_writers */,
                             1 /* num_readers */};
  };

  class Epoll::OperationInterface {
  public:
    virtual ~OperationInterface() = default;

    virtual OpResult Execute() noexcept = 0;
  };

  template<OpType type, OpDescr Descr>
  class Epoll::Operation : public Epoll::OperationInterface {
  public:
    Operation(Descr&& descr, uint64_t user_data) noexcept
        : descr_{std::forward<Descr>(descr)}
        , user_data_{user_data} {
    }

    OpResult Execute() noexcept override {
      int res;
      if constexpr (type == OpType::kRead) {
        res = read(GetArg<arg::fd>(descr_), GetArg<arg::buffer>(descr_),
                   GetArg<arg::count>(descr_));
      } else if constexpr (type == OpType::kWrite) {
        res = write(GetArg<arg::fd>(descr_), GetArg<arg::cbuffer>(descr_),
                    GetArg<arg::count>(descr_));
      } else {
        std::terminate();
      }
      if (res >= 0) {
        return OpResult{
            .result = res,
            .user_data = user_data_,
        };
      } else {
        return OpResult{
            .result = -errno,
            .user_data = user_data_,
        };
      }
    }

  private:
    Descr descr_;
    uint64_t user_data_;
  };

  template<OpType type, OpDescr Descr>
  std::error_code Epoll::Enqueue(Descr&& descr, uint64_t user_data) noexcept {

    Operation<type, Descr> op{std::forward<Descr>(descr), user_data};
    auto&& [res, udata] = op.Execute();
    if (res >= 0) {
      results_.Push(res, udata);
      return {};
    } else if (-res != EAGAIN && -res != EWOULDBLOCK) {
      results_.Push(res, udata);
      return {};
    }

    OperationInterface* op_ptr = new Operation{op};
    epoll_event event{
        .events = EPOLLET | EPOLLONESHOT,
        .data =
            epoll_data{
                .ptr = op_ptr,
            },
    };

    if constexpr (type == OpType::kRead) {
      event.events |= EPOLLIN;
    } else if constexpr (type == OpType::kWrite) {
      event.events |= EPOLLOUT;
    }

    int status =
        epoll_ctl(epoll_.fd(), EPOLL_CTL_ADD, GetArg<arg::fd>(descr), &event);
    if (status < 0) {
      return std::make_error_code(std::errc(errno));
    }
    return {};
  }

  tl::expected<OpResult, std::error_code> Epoll::Dequeue() noexcept {
    if (auto&& res = results_.TryPop(); res) {
      return *res;
    }

    epoll_event events[queue_size_];
    int res = epoll_wait(epoll_.fd(), events, queue_size_, -1);
    if (res == -1) {
      return tl::unexpected(std::make_error_code(std::errc(errno)));
    }

    for (std::size_t i = 0; i < res; ++i) {
      auto op_ptr = static_cast<OperationInterface*>(events[i].data.ptr);
      results_.Push(op_ptr->Execute());
      delete op_ptr;
    }

    return results_.Pop();
  }

} // namespace async
