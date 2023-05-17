#pragma once

#include <httpparser/request.h>
#include <httpparser/response.h>

namespace NAsync {
    using THttpRequest = httpparser::Request;
    using THttpResponse = httpparser::Response;
} // namespace NAsync
