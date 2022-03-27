#ifndef DOCKER_HH_
#define DOCKER_HH_

#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <curl/curl.h>

#include <nlohmann/json.hpp>

namespace docker {

enum class Protocol { TCP, UDP };

struct Port {
    Protocol protocol;
    std::uint16_t port;
};

struct Response {
    int code;
    nlohmann::json content;
};

class Docker {
public:
    Docker()
        : curl_(curl_easy_init())
    {
    }

    ~Docker()
    {
        curl_easy_cleanup(curl_);
    }

    std::optional<std::string> Launch(const std::string& image, const std::vector<Port>& exposedPorts, const std::unordered_map<std::string, std::string>& env)
    {
        nlohmann::json body = {
            {"Image", image},
        };

        for (const Port& port: exposedPorts) {
            std::string id = std::to_string(port.port) + std::string(port.protocol == Protocol::TCP ? "/tcp" : "/udp");
            body["ExposedPorts"][id] = nlohmann::json::object();
        }

        for (const auto& var: env) {
            body["Env"].push_back(var.first + "=" + var.second);
        }

        auto [code, content] = Post("/containers/create", body);
        if (code == 201 && content.count("Id")) {

            auto id = content["Id"].get<std::string>();
            if (Post("/containers/" + id + "/start").code == 204) {
                return id;
            }
        }

        return {};
    }

    void Stop(const std::string& id)
    {
        Post("/containers/" + id + "/stop");
    }

    void Remove(const std::string& id)
    {
        Delete("/containers/" + id);
    }

    Response Get(const std::string& endpoint)
    {
        auto [code, content] = CurlRequest(Verb::GET, endpoint);
        return {code, content.empty() ? nlohmann::json::object() : nlohmann::json::parse(content)};
    }

    Response Post(const std::string& endpoint, const nlohmann::json& body = {})
    {
        auto [code, content] = CurlRequest(Verb::POST, endpoint, body.dump());
        return {code, content.empty() ? nlohmann::json::object() : nlohmann::json::parse(content)};
    }

    Response Delete(const std::string& endpoint, const nlohmann::json& body = {})
    {
        auto [code, content] = CurlRequest(Verb::DELETE, endpoint, body.dump());
        return {code, content.empty() ? nlohmann::json::object() : nlohmann::json::parse(content)};
    }

    void SetLogOutput(std::ostream* os)
    {
        log_ = os;
    }

private:
    enum class Verb { GET, POST, DELETE };

    std::pair<int, std::string> CurlRequest(Verb verb, const std::string& endpoint, const std::string& body = "")
    {
        std::stringstream in, out;
        out.str(body);

        const char* verbStr = "GET";
        struct curl_slist *headers = curl_slist_append(nullptr, "Content-Type: application/json");

        curl_easy_reset(curl_);
        curl_easy_setopt(curl_, CURLOPT_UNIX_SOCKET_PATH, "/var/run/docker.sock");
        curl_easy_setopt(curl_, CURLOPT_URL, ("http:/v1.41" + endpoint).c_str());
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

        if (verb == Verb::POST) {
            curl_easy_setopt(curl_, CURLOPT_POST, 1);
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, body.length());
            verbStr = "POST";
        } else if (verb == Verb::DELETE) {
            curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, body.length());
            verbStr = "DELETE";
        }

        curl_easy_setopt(curl_, CURLOPT_READFUNCTION, CurlReadCallback);
        curl_easy_setopt(curl_, CURLOPT_READDATA, (void*)&out);
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (void*)&in);

        int code = -1;
        std::string response;
        CURLcode ret = curl_easy_perform(curl_);
        curl_slist_free_all(headers);

        if (ret == CURLE_OK) {
            curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &code);
            response = in.str();
            response.erase(response.find_last_not_of("\r\n") + 1);
        } else {
            response = "{\"curl_error\": " + std::to_string(ret) + "}";
        }

        if (log_) {
            *log_ << verbStr << " " << endpoint << " = [" << code << "] " << response << std::endl;
        }

        return {code, std::move(response)};
    }

    static std::size_t CurlReadCallback(char *buffer, std::size_t size, std::size_t nitems, void *userdata)
    {
        std::stringstream& ss = *(static_cast<std::stringstream*>(userdata));
        ss.get(buffer, size * nitems);
        return ss.gcount();
    }

    static std::size_t CurlWriteCallback(const char *ptr, std::size_t size, std::size_t nmemb, void *userdata)
    {
        std::stringstream& ss = *(static_cast<std::stringstream*>(userdata));
        ss.write(ptr, size * nmemb);
        return size * nmemb;
    }

private:
    CURL* curl_ = nullptr;
    std::ostream* log_ = nullptr;
};

}

#endif