module;

#include <utility>

#include <cassert>
#include <unistd.h>

module async:ioobject;

namespace async {

  class IoObject {
  public:
    IoObject() = default;
    explicit IoObject(int fd)
        : fd_{fd} {
    }

    ~IoObject() {
      if (fd_ != -1) {
        int ret = close(fd_);
        assert(ret == 0);
      }
    }

    IoObject(const IoObject&) noexcept = delete;
    IoObject& operator=(const IoObject&) noexcept = delete;

    IoObject(IoObject&& other) noexcept
        : fd_{std::exchange(other.fd_, -1)} {
    }

    IoObject& operator=(IoObject&& other) noexcept {
      if (fd_ != -1) {
        close(fd_);
      }
      fd_ = std::exchange(other.fd_, -1);
      return *this;
    }

    int fd() const noexcept {
      return fd_;
    }

  protected:
    int fd_ = -1;
  };

} // namespace async
