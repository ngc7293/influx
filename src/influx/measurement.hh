#ifndef INFLUX_MEASUREMENT_HH_
#define INFLUX_MEASUREMENT_HH_

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

bool operator<(const influx::Tag& lhs, const influx::Tag& rhs);
bool operator<(const influx::Field& lhs, const influx::Field& rhs);

class Measurement {
public:
    Measurement() = delete;
    Measurement(const std::string& name, Timestamp timestamp = Clock::now());
    Measurement(const std::string& name, const std::set<Tag>& tags, const std::set<Field>& fields, Timestamp timestamp = Clock::now());
    ~Measurement() = default;

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

class EmptyMeasurementError: public InfluxError {
    const char* what() const throw() override { return "Cannot serialize empty Measurement"; }
};

class EmptyKeyError: public InfluxError {
    const char* what() const throw() override { return "Field and Tag keys cannot be empty"; }
};

class NamingRestrictionError: public InfluxError {
    const char* what() const throw() override { return "Field and Tag keys cannot being with '_'"; }
};

} // namespace

/* Output measurment to ostream in line protocol format precision=ns */
std::ostream& operator<<(std::ostream& os, const influx::Measurement& measurement);

influx::Measurement& operator<<(influx::Measurement& measurement, const influx::Tag& tag);
influx::Measurement& operator<<(influx::Measurement& measurement, const influx::Field& field);
influx::Measurement& operator<<(influx::Measurement& measurement, const influx::Timestamp& timestamp);

#endif