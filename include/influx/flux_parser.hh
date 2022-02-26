#ifndef INFLUX_FLUXPARSER_HH_
#define INFLUX_FLUXPARSER_HH_

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include <influx/types.hh>

namespace influx {

struct FluxRecord {
    std::string name;
    Timestamp start;
    Timestamp stop;
    Timestamp time;
    FieldValue value;
    std::string field;
    std::string measurement;
    std::unordered_map<std::string, std::string> tags;
};

using FluxTable = std::vector<FluxRecord>;

class FluxParser {
public:
    FluxParser();
    ~FluxParser();

    std::vector<FluxTable> parse(const std::string& body);

private:
    struct Priv;
    std::unique_ptr<Priv> d_;
};

}

#endif