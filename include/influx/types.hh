#ifndef INFLUX__TYPES_HH_
#define INFLUX__TYPES_HH_

#include <chrono>
#include <exception>
#include <string>
#include <variant>

#include <cstdint>

namespace influx {
    using FieldValue = std::variant<double, std::int64_t, std::uint64_t, std::string, bool>;
    using Clock = std::chrono::system_clock;
    using Timestamp = Clock::time_point;

    class InfluxError: public std::exception {
    public:
        InfluxError(const char* message = "")
            : message_(message)
        {
        }

        const char* what() const throw() override { return message_; }

    private:
        const char* message_;
    };

    class InfluxRemoteError: public InfluxError {
    public:
        InfluxRemoteError(int statusCode, std::string&& message)
            : statusCode_(statusCode)
            , message_(std::move(message))
        {
        }

        int statusCode() const { return statusCode_; }
        const char* what() const throw() override { return message_.c_str(); }

    private:
        int statusCode_;
        std::string message_;
    };
} // namespace

#endif
