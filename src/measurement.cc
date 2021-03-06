#include <iomanip>

#include <influx/measurement.hh>

namespace {
    bool Contains(char needle, const char* haystack)
    {
        const char* current = haystack;
        while (*current != '\0') {
            if (needle == *current) {
                return true;
            }
            current++;
        }

        return false;
    }

    std::string Escape(const std::string& str, const char* chars)
    {
        std::string escaped;
        escaped.reserve(str.length());

        for (size_t i = 0; i < str.length(); ++i) {
            if (Contains(str[i], chars)) {
                escaped.push_back('\\');
            }
            escaped.push_back(str[i]);
        }
        return escaped;
    }

    struct FieldValuePrinter {
        std::ostream& os;

        void operator()(double value)             { os << value; }
        void operator()(std::int64_t value)       { os << value << "i"; }
        void operator()(std::uint64_t value)      { os << value << "u"; }
        void operator()(const std::string& value) { os << "\"" << Escape(value, "\"\\") << "\""; }
        void operator()(bool value)               { os << (value ? "true" : "false"); }
    };
}

namespace influx {

Tag::Tag(const std::string& key, const std::string& value)
    : key(key)
    , value(value)
{
    if (key.length() == 0) {
        throw InvalidMeasurementError("Tag keys cannot be empty");
    }

    if (key[0] == '_') {
        throw InvalidMeasurementError("Tag keys cannot being with '_'");
    }
}

Field::Field(const std::string& key, const FieldValue& value)
    : key(key)
    , value(value)
{
    if (key.length() == 0) {
        throw InvalidMeasurementError("Field keys cannot be empty");
    }

    if (key[0] == '_') {
        throw InvalidMeasurementError("Field keys cannot being with '_'");
    }
}

bool operator==(const Tag& lhs, const Tag& rhs)
{
    return lhs.key == rhs.key && lhs.value == rhs.value;
}

bool operator!=(const Tag& lhs, const Tag& rhs)
{
    return !(lhs == rhs);
}

bool operator<(const Tag& lhs, const Tag& rhs)
{
    return lhs.key.compare(rhs.key) < 0;
}

bool operator==(const Field& lhs, const Field& rhs)
{
    return lhs.key == rhs.key && lhs.value == rhs.value;
}

bool operator!=(const Field& lhs, const Field& rhs)
{
    return !(lhs == rhs);
}

bool operator<(const Field& lhs, const Field& rhs)
{
    return lhs.key.compare(rhs.key) < 0;
}

Measurement::Measurement(const std::string& name, Timestamp timestamp)
    : name_(name)
    , timestamp_(timestamp)
{
}

Measurement::Measurement(const std::string& name, const std::set<Tag>& tags, const std::set<Field>& fields, Timestamp timestamp)
    : name_(name)
    , tags_(tags)
    , fields_(fields)
    , timestamp_(timestamp)
{
}

bool Measurement::operator==(const Measurement& other) const
{
    return name_ == other.name_ 
        && timestamp_ == other.timestamp_
        && tags_ == other.tags_
        && fields_ == other.fields_;
}

bool Measurement::operator!=(const Measurement& other) const
{
    return !(*this == other);
}

 void Measurement::AddTag(const Tag& tag)
 {
     tags_.insert(tag);
 }

 void Measurement::AddField(const Field& field)
 {
     fields_.insert(field);
 }

 void Measurement::SetTimestamp(const Timestamp& timestamp)
 {
     timestamp_ = timestamp;
 }

std::string Measurement::name() const
{
    return name_;
} 

const std::set<Tag>& Measurement::tags() const
{
    return tags_;
}

const std::set<Field>& Measurement::fields() const
{
    return fields_;
}

Timestamp Measurement::timestamp() const
{
    return timestamp_;
}

std::ostream& operator<<(std::ostream& os, const influx::Measurement& measurement)
{
    if (measurement.fields().empty()) {
        throw influx::InvalidMeasurementError("Cannot serialize empty Measurement");
    }

    os << Escape(measurement.name(), " ,");

    for (const influx::Tag& tag: measurement.tags()) {
        os << "," << Escape(tag.key, " =,") << "=" << Escape(tag.value, " =,");
    }

    bool first = true;
    FieldValuePrinter printer{os};
    for (const influx::Field& field: measurement.fields()) {
        os << (first ? " " : ",");
        os << Escape(field.key, " =,") << "=";
        std::visit(printer, field.value);
        first = false;
    }

    os << " " << std::chrono::duration_cast<std::chrono::nanoseconds>(measurement.timestamp().time_since_epoch()).count();
    return os;
}

} // namespace

influx::Measurement& operator<<(influx::Measurement& measurement, const influx::Tag& tag)
{
    measurement.AddTag(tag);
    return measurement;
}

influx::Measurement& operator<<(influx::Measurement& measurement, const influx::Field& field)
{
    measurement.AddField(field);
    return measurement;
}

influx::Measurement& operator<<(influx::Measurement& measurement, const influx::Timestamp& timestamp)
{
    measurement.SetTimestamp(timestamp);
    return measurement;   
}


influx::Measurement operator<<(influx::Measurement&& measurement, const influx::Tag& tag)
{
    return std::move(measurement << tag);
}

influx::Measurement operator<<(influx::Measurement&& measurement, const influx::Field& field)
{
    return std::move(measurement << field);
}

influx::Measurement operator<<(influx::Measurement&& measurement, const influx::Timestamp& timestamp)
{
    return std::move(measurement << timestamp);
}
