#pragma once

#include <io/io_object.hpp>

namespace NAsync {

    std::pair<TIoObject, TIoObject> CreatePipe() noexcept;

} // namespace NAsync