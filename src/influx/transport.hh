#ifndef INFLUX_TRANSPORT_HH_
#define INFLUX_TRANSPORT_HH_

#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <influx/types.hh>

namespace influx::transport {

enum class Verb {
    GET,
    POST,
    PUT,
    DELETE
};

struct HttpResponse {
    int status;
    std::string body;
};

class HttpSession {
public:
    HttpSession(const std::string& host, const std::string& token);
    ~HttpSession();

    HttpResponse Get(
        const std::string& url,
        const std::vector<std::pair<std::string, std::string>> headers
    );

    HttpResponse Post(
        const std::string& url,
        const std::vector<std::pair<std::string, std::string>> headers,
        const std::string& body
    );

private:
    HttpResponse Perform(
        const Verb verb,
        const std::string& url,
        const std::vector<std::pair<std::string, std::string>> headers,
        const std::string& body
    );

private:
    struct Priv;
    std::unique_ptr<Priv> d_;
};

} // namespace

#endif