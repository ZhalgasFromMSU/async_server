module;

#include <tl/expected.hpp>

#include <system_error>

#include <linux/io_uring.h>
#include <sys/syscall.h>
#include <unistd.h>

module async:iouring;
import :ioobject;

namespace async {

  class IoUring {
  public:
    static tl::expected<IoUring, std::error_code> Create() noexcept {
    }

    void Enqueue() noexcept {
    }

    void Dequeue() noexcept;

  private:
    static constexpr std::size_t ring_size_ = 1000;

    IoObject ring;
  };

} // namespace async
