#ifndef INFLUX__CLIENT_HH_
#define INFLUX__CLIENT_HH_

#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <unordered_map>

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

class HttpClient {
public:
    HttpClient();
    HttpClient(HttpClient&& other);
    HttpClient(const HttpClient& other);
    HttpClient(const std::string& host, const std::string& org, const std::string& token);
    ~HttpClient();

    HttpResponse Get(
        const std::string& endpoint,
        const std::unordered_map<std::string, std::string>& headers = {}
    );

    HttpResponse Post(
        const std::string& endpoint,
        const std::string& body,
        const std::unordered_map<std::string, std::string>& headers = {}
    );

    HttpResponse Delete(
        const std::string& endpoint,
        const std::string& body = "",
        const std::unordered_map<std::string, std::string>& headers = {}
    );


private:
    HttpResponse Perform(
        const Verb verb,
        const std::string& endpoint,
        const std::string& body,
        const std::unordered_map<std::string, std::string>& headers
    );

private:
    struct Priv;
    std::unique_ptr<Priv> d_;
};

} // namespace

#endif
