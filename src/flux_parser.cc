#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>

#include <cassert>
#include <cstring>
#include <ctime>

#if (defined _MSC_VER)
#include <windows.h>
#include <timezoneapi.h>
#endif

#include <influx/flux_parser.hh>

namespace influx {

namespace {
    const char * const ANNOTATION_DATATYPE = "#datatype";

    struct Column {
        std::string name;
        std::string type;
    };

    std::chrono::seconds tzbias()
    {
#if (defined _MSC_VER)
        TIME_ZONE_INFORMATION tzinfo;
        GetTimeZoneInformation(&tzinfo);
        return std::chrono::seconds(tzinfo.Bias * 60);
#else
        return std::chrono::seconds(timezone);
#endif
    }

    Timestamp parseRFC3339(const std::string& str)
    {
        std::istringstream ss(str);
        struct std::tm tm;
        std::memset(&tm, 0, sizeof(struct tm));

        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        time_t timestamp = mktime(&tm) - tzbias().count();

        double frac = 0;
        ss >> frac;
        auto nano = static_cast<std::uint64_t>(frac * 1e9);
        return Clock::from_time_t(timestamp) + std::chrono::duration_cast<Clock::duration>(std::chrono::nanoseconds(nano));
    }
}

std::vector<FluxTable> FluxParser::parse(const std::string& body)
{
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
                        // GCC and MSVC disagree on what a long long is
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
