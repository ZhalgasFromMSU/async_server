#pragma once

#include "base.hpp"

namespace NAsync {

    class TSocket: public IIoObject {
    public:
        int GetFd() const noexcept override;

    private:
        int Fd_;
    };

} // namespace NAsync