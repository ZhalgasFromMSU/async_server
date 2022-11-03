#pragma once

namespace NAsync {

    class IIoObject {
    public:
        virtual ~IIoObject() = default;
        virtual int GetFd() const noexcept = 0;
    };

} // namespace NAsync