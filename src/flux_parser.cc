#include <iostream>
#include <sstream>

#include <cassert>
#include <cstring>
#include <ctime>

#include <influx/flux_parser.hh>

namespace influx {

namespace {
    const char * const ANNOTATION_DATATYPE = "#datatype";

    struct Column {
        std::string name;
        std::string type;
    };

    Timestamp parseRFC3339(const std::string& str)
    {
        struct std::tm then_tm{0};
        std::memset(&then_tm, 0, sizeof(struct tm));

        strptime(str.c_str(), "%FT%T", &then_tm);
        time_t then = mktime(&then_tm) - timezone;

        auto cur = str.find_first_of('.');
        std::string dec = "0" + str.substr(cur, str.size() - cur - 1);
        return Clock::from_time_t(then) + std::chrono::nanoseconds(static_cast<std::uint64_t>(std::stod(dec) * 10e8));
    }
}

struct FluxParser::Priv {

};

FluxParser::FluxParser()
    : d_(new Priv)
{
}

FluxParser::~FluxParser()
{
}


std::vector<FluxTable> FluxParser::parse(const std::string& body)
{
    // int currentTable = -1;
    std::stringstream ssbody(body);

    std::vector<FluxTable> tables;

    std::vector<Column> columns;
    FluxTable table;
    bool new_table = false;

    for (std::string line; std::getline(ssbody, line);) {
        if (line.ends_with("\r")) {
            line = line.substr(0, line.size() - 1);
        }

        if (line.empty()) {
            continue;
        } else if (line.starts_with(ANNOTATION_DATATYPE)) {
            if (!table.empty()) {
                tables.emplace_back(std::move(table));
            }

            columns.clear();
            table.clear();
            new_table = true;

            std::stringstream ssline(std::move(line));
            for (std::string token = ""; std::getline(ssline, token, ',');) {
                if (token.empty() || token == ANNOTATION_DATATYPE) {
                    continue;
                }

                columns.emplace_back("", token);
            }
        } else if (line.starts_with("#")) {
            continue;
        } else if (new_table) {
            std::stringstream ssline(std::move(line));
            std::size_t i = 0;

            for (std::string token = ""; std::getline(ssline, token, ','); i++) {
                if (token.empty()) {
                    continue;
                }

                columns[i - 1].name = std::move(token);
            }
            new_table = false;
        } else {
            std::stringstream ssline(std::move(line));
            std::size_t i = 0;

            FluxRecord record;

            for (std::string token = ""; std::getline(ssline, token, ','); i++) {
                if (token.empty()) {
                    continue;
                }

                if (i > columns.size()) {
                    assert(false);
                }

                if (columns[i - 1].name == "result") {
                    record.name = token;
                } else if (columns[i - 1].name == "table") {
                    continue;
                } else if (columns[i - 1].name == "_start") {
                    record.start = parseRFC3339(token);
                } else if (columns[i - 1].name == "_stop") {
                    record.stop = parseRFC3339(token);
                } else if (columns[i - 1].name == "_time") {
                    record.time = parseRFC3339(token);
                } else if (columns[i - 1].name == "_value") {
                    if (columns[i - 1].type == "double") {
                        record.value = std::stod(token);
                    } else if (columns[i - 1].type == "boolean") {
                        record.value = (token == "true");
                    } else if (columns[i - 1].type == "unsignedLong") {
                        record.value = static_cast<std::uint64_t>(std::stoull(token));
                    } else if (columns[i - 1].type == "long") {
                        record.value = static_cast<std::int64_t>(std::stoll(token));
                    } else {
                        record.value = token;
                    }
                } else if (columns[i - 1].name == "_field") {
                    record.field = token;
                } else if (columns[i - 1].name == "_measurement") {
                    record.field = token;
                } else {
                    record.tags[columns[i - 1].name] = token;
                }
            }

            table.emplace_back(std::move(record));
        }
    }

    if (!table.empty()) {
        tables.push_back(std::move(table));
    }

    return tables;
}

}
