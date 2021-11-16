#include <sstream>

#include <gtest/gtest.h>

#include <influx/measurement.hh>

using namespace std::chrono_literals;

TEST(MeasurementTest, outputs_correctly_as_line_protocol)
{
    influx::Measurement m("m");

    m << influx::Tag{"key1", "value1"}
      << influx::Field{"field2", false}
      << influx::Tag{"key2", "value2"}
      << influx::Timestamp(1000ns)
      << influx::Field{"field1", 0.0}
      << influx::Field{"field3", 1}
      << influx::Field{"field4", 0ul};

    std::stringstream ss;
    ss << m;

    EXPECT_EQ(ss.str(), "m key1=value1,key2=value2 field1=0,field2=false,field3=1i,field4=0u 1000");
}

TEST(MeasurementTest, escapes_correctly)
{
    influx::Measurement m("my measurement", influx::Timestamp(1000ns));

    m << influx::Tag{"key=1", "value 1"}
      << influx::Field{"field2", R"("a" is different from "\")"}
      << influx::Tag{"key,2", "🚀"};

    std::stringstream ss;
    ss << m;

    EXPECT_EQ(ss.str(), R"(my\ measurement key\,2=🚀,key\=1=value\ 1 field2="\"a\" is different from \"\\\"" 1000)");
}

TEST(MeasurementTest, throws_is_trying_to_output_empty_measurement)
{
    try {
        std::stringstream ss;
        ss << influx::Measurement("m", influx::Timestamp(1000ns));
        EXPECT_FALSE(true);
    } catch (influx::EmptyMeasurementError& e) { }
}

TEST(MeasurementTest, throws_if_tag_or_field_key_is_empty)
{
    try {
        influx::Tag{"", "value"};
        EXPECT_FALSE(true);
    } catch (influx::EmptyKeyError& e) { }

    try {
        influx::Field{"", "value"};
        EXPECT_FALSE(true);
    } catch (influx::EmptyKeyError& e) { }
}

TEST(MeasurementTest, throws_if_tag_or_field_key_begins_with_underscore)
{
    try {
        influx::Tag{"_id", "value"};
        EXPECT_FALSE(true);
    } catch (influx::NamingRestrictionError& e) { }

    try {
        influx::Field{"_oola", "value"};
        EXPECT_FALSE(true);
    } catch (influx::NamingRestrictionError& e) { }
}