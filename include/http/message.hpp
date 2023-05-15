#pragma once

#include <string>
#include <vector>

namespace NAsync {

    enum class EHttpMethod {
        kGet,
        kHead,
        kPost,
        kPut,
        kDelete,
        kConnect,
        kOptions,
        kTrace,
        kPatch,
    };

    struct THttpRequest {
        // First line
        EHttpMethod Method = EHttpMethod::kGet;
        std::string Path = "/";
        std::vector<std::pair<std::string, std::string>> Cgi;
        int HttpVersionMajor = 1;
        int HttpVersionMinor = 1;
        // Headers
        std::vector<std::pair<std::string, std::string>> Headers;
        // Body
        std::string Body;

        // Methods
        std::string Serialize() const;
    };

    struct THttpResponse {
        // First line
        int Status;
        int StatusMessage;
        int HttpVersionMajor;
        int HttpVersionMinor;
        // Headers
        std::vector<std::pair<std::string, std::string>> Headers;
        // Body
        std::string Body;

        // Methods
        static THttpResponse FromString(const std::string& s);
    };
} // namespace NAsync
