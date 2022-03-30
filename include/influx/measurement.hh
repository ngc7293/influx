#ifndef INFLUX__MEASUREMENT_HH_
#define INFLUX__MEASUREMENT_HH_

#include <iostream>
#include <set>

#include <influx/types.hh>

namespace influx {

struct Tag {
    std::string key;
    std::string value;

    Tag() = delete;
    Tag(const std::string& key, const std::string& value);
};

struct Field {
    std::string key;
    FieldValue value;

    Field() = delete;
    Field(const std::string& key, const FieldValue& value);
};

bool operator==(const influx::Tag& lhs, const influx::Tag& rhs);
bool operator!=(const influx::Tag& lhs, const influx::Tag& rhs);
bool operator<(const influx::Tag& lhs, const influx::Tag& rhs);

bool operator==(const influx::Field& lhs, const influx::Field& rhs);
bool operator!=(const influx::Field& lhs, const influx::Field& rhs);
bool operator<(const influx::Field& lhs, const influx::Field& rhs);

class Measurement {
public:
    Measurement() = delete;
    Measurement(const std::string& name, Timestamp timestamp = Clock::now());
    Measurement(const std::string& name, const std::set<Tag>& tags, const std::set<Field>& fields, Timestamp timestamp = Clock::now());
    ~Measurement() = default;

    bool operator==(const Measurement& other) const;
    bool operator!=(const Measurement& other) const;

    void AddTag(const Tag& tag);
    void AddField(const Field& field);
    void SetTimestamp(const Timestamp& timestamp);

    std::string name() const;
    const std::set<Tag>& tags() const;
    const std::set<Field>& fields() const;
    Timestamp timestamp() const;

private:
    std::string name_;
    std::set<Tag> tags_;
    std::set<Field> fields_;
    Timestamp timestamp_;
};

class InvalidMeasurementError: public InfluxError {
public:
    InvalidMeasurementError(const char* what)
        : what_(what)
    {
    }

    const char* what() const throw() override { return what_; }

private:
    const char* what_ = "Invalid measurement";
};

::std::ostream& operator<<(::std::ostream& os, const Measurement& measurement);

} // namespace

/* Output measurment to ostream in line protocol format precision=ns */

influx::Measurement operator<<(influx::Measurement&& measurement, const influx::Tag& tag);
influx::Measurement operator<<(influx::Measurement&& measurement, const influx::Field& tag);
influx::Measurement operator<<(influx::Measurement&& measurement, const influx::Timestamp& tag);

influx::Measurement& operator<<(influx::Measurement& measurement, const influx::Tag& tag);
influx::Measurement& operator<<(influx::Measurement& measurement, const influx::Field& field);
influx::Measurement& operator<<(influx::Measurement& measurement, const influx::Timestamp& timestamp);

#endif
