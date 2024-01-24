module;

#include <tl/expected.hpp>

#include <system_error>

#include <liburing.h>

export module async:iouring;
import :ioobject;
import :awaitable;
import :operation;

namespace async {

  export class IoUring {
  private:
    class ConstructionToken {
      friend IoUring;
      ConstructionToken() noexcept = default;
    };

  public:
    IoUring(ConstructionToken, io_uring&& ring) noexcept
        : ring_{std::move(ring)} {
    }

    ~IoUring() {
      io_uring_queue_exit(&ring_);
    }

    IoUring(const IoUring&) = delete;
    IoUring(IoUring&&) = delete;
    IoUring& operator=(const IoUring&) = delete;
    IoUring& operator=(IoUring&&) = delete;

    static tl::expected<IoUring, std::error_code> Create() noexcept {
      io_uring ring;
      int ret = io_uring_queue_init(ring_size_, &ring, 0);
      if (ret < 0) {
        return tl::unexpected(std::make_error_code(std::errc(-ret)));
      }
      return tl::expected<IoUring, std::error_code>{
          tl::in_place, ConstructionToken{}, std::move(ring)};
    }

    bool PrepareDispatch(Awaitable auto& awaitable) noexcept {
      return true;
    }

    std::error_code Dispatch(Awaitable auto& awaitable) noexcept {
      io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
      assert(sqe != nullptr && "Ring buffer is full");

      io_uring_sqe_set_data(sqe, static_cast<void*>(&awaitable));
      PrepareSQE(sqe, awaitable.GetOp());

      if (int code = io_uring_submit(&ring_); code < 0) {
        return std::make_error_code(std::errc(-code));
      }
      return std::error_code{};
    }

    tl::expected<AwaitableBase*, std::error_code> Dequeue() noexcept {
      io_uring_cqe* cqe;
      if (int code = io_uring_wait_cqe(&ring_, &cqe); code < 0) {
        return tl::unexpected{std::make_error_code(std::errc(-code))};
      }

      auto awaitable = static_cast<AwaitableBase*>(io_uring_cqe_get_data(cqe));
      awaitable->SetResult(cqe->res);
      io_uring_cqe_seen(&ring_, cqe);
      return awaitable;
    }

    static constexpr bool is_proactive = true;

  private:
    void PrepareSQE(io_uring_sqe* sqe, auto&& op) noexcept {
      // casting in following functions is required:
      // https://github.com/llvm/llvm-project/issues/78173
      if constexpr (constexpr OpType type =
                        std::remove_reference_t<decltype(op)>::op_type;
                    type == OpType::kRead) {
        io_uring_prep_read(sqe, static_cast<int>(GetArg<arg::fd>(op)),
                           static_cast<void*>(GetArg<arg::buffer>(op)),
                           static_cast<std::size_t>(GetArg<arg::count>(op)),
                           -1);
      } else if constexpr (type == OpType::kWrite) {
        io_uring_prep_write(sqe, static_cast<int>(GetArg<arg::fd>(op)),
                            static_cast<const void*>(GetArg<arg::cbuffer>(op)),
                            static_cast<std::size_t>(GetArg<arg::count>(op)),
                            -1);
      } else {
        assert(false && "Unknown operation type");
      }
    }

    static constexpr unsigned ring_size_ = 1000;
    io_uring ring_;
  };

} // namespace async
