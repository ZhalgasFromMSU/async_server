module;

#include <tl/expected.hpp>

#include <system_error>

#include <liburing.h>

export module async:iouring;
import :operation;
import :ioobject;

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

    ~IoUring() {
      io_uring_queue_exit(&ring_);
    }

    template<OpType type>
    std::error_code Enqueue(OpDescr auto&& op, uint64_t user_data) noexcept;

    tl::expected<OpResult, std::error_code> Dequeue() noexcept;

  private:
    static constexpr unsigned ring_size_ = 1000;
    io_uring ring_;
  };

  template<OpType type>
  std::error_code IoUring::Enqueue(OpDescr auto&& op,
                                   uint64_t user_data) noexcept {
    io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
    assert(sqe != nullptr && "Ring buffer is full");

    io_uring_sqe_set_data64(sqe, user_data);
    // casting in following functions is required:
    // https://github.com/llvm/llvm-project/issues/78173
    if constexpr (type == OpType::kRead) {
      io_uring_prep_read(sqe, static_cast<int>(GetArg<arg::fd>(op)),
                         static_cast<void*>(GetArg<arg::buffer>(op)),
                         static_cast<std::size_t>(GetArg<arg::count>(op)), -1);
    } else if constexpr (type == OpType::kWrite) {
      io_uring_prep_write(sqe, static_cast<int>(GetArg<arg::fd>(op)),
                          static_cast<const void*>(GetArg<arg::cbuffer>(op)),
                          static_cast<std::size_t>(GetArg<arg::count>(op)), -1);
    }

    int code = io_uring_submit(&ring_);
    if (code < 0) {
      return std::make_error_code(std::errc(-code));
    }

    return std::error_code{};
  }

  tl::expected<OpResult, std::error_code> IoUring::Dequeue() noexcept {
    io_uring_cqe* cqe;
    int code = io_uring_wait_cqe(&ring_, &cqe);
    if (code < 0) {
      return tl::unexpected(std::make_error_code(std::errc(-code)));
    }

    OpResult ret{.result = cqe->res, .user_data = io_uring_cqe_get_data64(cqe)};
    io_uring_cqe_seen(&ring_, cqe);
    return ret;
  }

} // namespace async
