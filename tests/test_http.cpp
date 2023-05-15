#include <http/message.hpp>

#include <gtest/gtest.h>

using namespace NAsync;

TEST(Http, SerializeRequest) {
    {
        THttpRequest req;
        std::string expectedString = "GET / HTTP/1.1\r\n"
                                     "Content-Length: 0\r\n"
                                     "\r\n";
        ASSERT_EQ(req.Serialize(), expectedString);
    }

    {
        THttpRequest req;
        req.Method = EHttpMethod::kPost;
        req.Path = "/custompath";
        req.Cgi.emplace_back("cgi", "cgi_val");
        req.Cgi.emplace_back("cgi2", "cgi_val2");
        req.Headers.emplace_back("header", "header_val");
        req.Body = "123";

        std::string expectedString = "POST /custompath?cgi=cgi_val&cgi2=cgi_val2 HTTP/1.1\r\n"
                                     "header: header_val\r\n"
                                     "Content-Length: 3\r\n"
                                     "\r\n"
                                     "123\r\n"
                                     "\r\n";
                                     
        ASSERT_EQ(req.Serialize(), expectedString);
    }
}

