module;

#include <tl/expected.hpp>

#include <system_error>

#include <liburing.h>

export module async:iouring;
export import :operation;
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

    IoUring(const IoUring&) noexcept = delete;
    IoUring(IoUring&&) noexcept = delete;
    IoUring& operator=(const IoUring&) noexcept = delete;
    IoUring& operator=(IoUring&&) noexcept = delete;

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

    std::error_code Enqueue(OpDescr auto&& op, uint64_t user_data) noexcept;
    tl::expected<std::pair<int, uint64_t>, std::error_code> Dequeue() noexcept;

    // std::optional<int> TryDequeue() noexcept {
    // io_uring_cqe* cqe;
    // if (io_uring_peek_cqe(&ring_, &cqe) == 0) {
    // int n_read = cqe->res;
    // io_uring_cqe_seen(&ring_, cqe);
    // return n_read;
    // } else {
    // return std::nullopt;
    // }
    // }

    // tl::expected<int, std::error_code> Dequeue() noexcept {
    // io_uring_cqe* cqe;
    // int ret = io_uring_wait_cqe(&ring_, &cqe);
    // if (ret < 0) {
    // return tl::unexpected(std::make_error_code(std::errc(-ret)));
    // }
    // int n_read = cqe->res;
    // io_uring_cqe_seen(&ring_, cqe);
    // return n_read;
    // }

  private:
    static constexpr unsigned ring_size_ = 1000;
    io_uring ring_;
  };

  std::error_code IoUring::Enqueue(OpDescr auto&& op,
                                   uint64_t user_data) noexcept {
    io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
    assert(sqe != nullptr && "Ring buffer is full");

    io_uring_sqe_set_data64(sqe, user_data);
    if constexpr (op.type == OpType::kRead) {
      io_uring_prep_read(sqe, GetArg<arg::fd>(op), GetArg<arg::buffer>(op),
                         GetArg<arg::count>(op), -1);
    } else if constexpr (op.type == OpType::kWrite) {
      io_uring_prep_write(sqe, GetArg<arg::fd>(op), GetArg<arg::cbuffer>(op),
                          GetArg<arg::count>(op), -1);
    }

    int code = io_uring_submit(&ring_);
    if (code < 0) {
      return std::make_error_code(std::errc(-code));
    }

    return std::error_code{};
  }

  tl::expected<std::pair<int, uint64_t>, std::error_code>
  IoUring::Dequeue() noexcept {
    io_uring_cqe* cqe;
    int code = io_uring_wait_cqe(&ring_, &cqe);
    if (code < 0) {
      return tl::unexpected(std::make_error_code(std::errc(-code)));
    }

    auto ret = std::make_pair(cqe->res, io_uring_cqe_get_data64(cqe));
    io_uring_cqe_seen(&ring_, cqe);
    return ret;
  }

} // namespace async
