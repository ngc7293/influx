#ifndef INFLUX__TEST__TESTCONFIG_HH_
#define INFLUX__TEST__TESTCONFIG_HH_

#include <fstream>
#include <filesystem>

#include <nlohmann/json.hpp>
#include <influx/influx.hh>

using namespace std::chrono_literals;

namespace influx::test {

inline const nlohmann::json config()
{
    std::ifstream ifs;
    nlohmann::json c;
    
    if (!std::filesystem::exists("test.config.json")) {
        std::cerr << "Could not find 'test.config.json' in " << std::filesystem::current_path() << std::endl;
        exit(-1);
    }

    ifs.open("test.config.json");
    ifs >> c;

    if (!c.count("host") && !c["host"].is_string()) {
        std::cerr << "Invalid test config: missing 'host' field" << std::endl;
    }

    if (!c.count("org") && !c["org"].is_string()) {
        std::cerr << "Invalid test config: missing 'org' field" << std::endl;
    }

    if (!c.count("token") && !c["token"].is_string()) {
        std::cerr << "Invalid test config: missing 'token' field" << std::endl;
    }

    return c;
}

inline influx::Influx db()
{
    const auto c = config();
    return Influx(c["host"], c["org"], c["token"]);
}

inline std::string nowstring()
{
    return std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
}

} // namespace

#endif
