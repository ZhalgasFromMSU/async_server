#include <http/message.hpp>

namespace NAsync {

    std::string THttpRequest::Serialize() const {
        std::string s;

#define METHOD_CASE(method, str) \
            case method: \
                s += str " "; \
                break;

        switch (Method) {
            METHOD_CASE(EHttpMethod::kGet, "GET")
            METHOD_CASE(EHttpMethod::kHead, "HEAD")
            METHOD_CASE(EHttpMethod::kPost, "POST")
            METHOD_CASE(EHttpMethod::kPut, "PUT")
            METHOD_CASE(EHttpMethod::kDelete, "DELETE")
            METHOD_CASE(EHttpMethod::kConnect, "CONNECT")
            METHOD_CASE(EHttpMethod::kOptions, "OPTIONS")
            METHOD_CASE(EHttpMethod::kTrace, "TRACE")
            METHOD_CASE(EHttpMethod::kPatch, "PATCH")
        }
#undef METHOD_CASE

        s += Path;
        if (!Cgi.empty()) {
            s += "?";
            std::string sep = "";
            for (const auto& [name, val] : Cgi) {
                s += sep + name + "=" + val;
                if (sep.empty()) {
                    sep = "&";
                }
            }
        }
        s += " ";
        s += "HTTP/" + std::to_string(HttpVersionMajor) + "." + std::to_string(HttpVersionMinor);
        s += "\r\n";

        for (const auto& [name, val] : Headers) {
            s += name + ": " + val + "\r\n";
        }
        s += "Content-Length: " + std::to_string(Body.size()) + "\r\n";
        s += "\r\n";

        if (!Body.empty()) {
            s += Body + "\r\n\r\n";
        }
        return s;
    }

} // namespace NAsync
