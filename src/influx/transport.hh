#ifndef INFLUX_TRANSPORT_HH_
#define INFLUX_TRANSPORT_HH_

#include <memory>
#include <string>
#include <vector>
#include <utility>

namespace influx::transport {

class HttpSession {
public:
    HttpSession(const std::string& domain, const std::string& org);
    ~HttpSession();

    void Post(
        const std::string& path,
        const std::vector<std::pair<std::string, std::string>> headers,
        const std::string& body
    );

private:
    struct Priv;
    std::unique_ptr<Priv> d_;
};

} // namespace

#endif