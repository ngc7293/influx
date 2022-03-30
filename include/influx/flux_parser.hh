#ifndef INFLUX__FLUX_PARSER_HH_
#define INFLUX__FLUX_PARSER_HH_

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
    std::vector<FluxTable> parse(const std::string& body);
};

} // namespace

#endif
