#ifndef INFLUX_TYPES_HH_
#define INFLUX_TYPES_HH_

#include <chrono>
#include <exception>
#include <string>
#include <variant>

#include <cstdint>

namespace influx {
    using FieldValue = std::variant<double, std::int64_t, std::uint64_t, std::string, bool>;
    using Clock = std::chrono::high_resolution_clock;
    using Timestamp = Clock::time_point;

    class InfluxError: public std::exception {
    };

    class InfluxRemoteError: public InfluxError {
    public:
        InfluxRemoteError(int statusCode)
            : statusCode_(statusCode)
        {
        }

        int statusCode() const { return statusCode_; }

    private:
        int statusCode_;
    };
}

#endif