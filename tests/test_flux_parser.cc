#include <iostream>

#include <gtest/gtest.h>

#include <influx/bucket.hh>
#include <influx/influx.hh>

#include "config.hh"

TEST(FluxParserTest, should_parse_query_static)
{
    auto tables = influx::FluxParser().parse(R"~~(
#datatype,string,long,dateTime:RFC3339,dateTime:RFC3339,dateTime:RFC3339,double,string,string,string
,result,table,_start,_stop,_time,_value,_field,_measurement,domain
,acqui,0,2022-02-26T17:51:31.687426831Z,2022-02-26T17:51:51.687426831Z,2022-02-26T17:51:36.590333377Z,20,x,acquisition,1
,acqui,0,2022-02-26T17:51:31.687426831Z,2022-02-26T17:51:51.687426831Z,2022-02-26T17:51:41.590333377Z,10,x,acquisition,1

#datatype,string,long,dateTime:RFC3339,dateTime:RFC3339,dateTime:RFC3339,long,string,string,string,string
,result,table,_start,_stop,_time,_value,_field,_measurement,client,domain
,acqui,1,2022-02-26T17:51:31.687426831Z,2022-02-26T17:51:51.687426831Z,2022-02-26T17:51:46.590333377Z,30,x,fizzbuzz,2,1
)~~");

    ASSERT_EQ(tables.size(), 2);
    ASSERT_EQ(tables[0].size(), 2);
    ASSERT_EQ(tables[1].size(), 1);

    EXPECT_TRUE(std::holds_alternative<double>(tables[0][0].value));
    EXPECT_EQ(std::get<double>(tables[0][0].value), 20.0);

    EXPECT_TRUE(std::holds_alternative<double>(tables[0][1].value));
    EXPECT_EQ(std::get<double>(tables[0][1].value), 10.0);

    EXPECT_TRUE(std::holds_alternative<std::int64_t>(tables[1][0].value));
    EXPECT_EQ(std::get<std::int64_t>(tables[1][0].value), 30);

    EXPECT_EQ(tables[0][0].start, influx::Timestamp() + 1645897891687426831ns);
    EXPECT_EQ(tables[0][0].stop, influx::Timestamp() + 1645897911687426831ns);
    EXPECT_EQ(tables[0][0].time, influx::Timestamp() + 1645897896590333377ns);

    EXPECT_EQ(tables[0][0].tags.at("domain"), "1");
    EXPECT_EQ(tables[1][0].tags.at("client"), "2");
}

TEST(FluxParserTest, should_parse_query_dynamic)
{
    auto name = influx::test::nowstring();
    auto db = influx::test::db();
    auto bucket = db.CreateBucket(name, 1h);
    auto now = influx::Clock::now() - std::chrono::seconds(15);

    bucket
        << (influx::Measurement("acquisition", now +  0s) << influx::Field("x", 20.0) << influx::Field("y", 21.0) << influx::Field("z", 22.0) << influx::Tag("domain", "1"))
        << (influx::Measurement("acquisition", now +  5s) << influx::Field("x", 10.0) << influx::Field("y", 11.0) << influx::Field("z", 12.0) << influx::Tag("domain", "1"))
        << (influx::Measurement("fizzbuzz",    now + 10s) << influx::Field("x", 30)   << influx::Field("y", 31.0) << influx::Field("z", 32.0) << influx::Tag("domain", "1") << influx::Tag("client", "2"));
    bucket.Flush();

    auto tables = db.Query(R"~(
        from(bucket: ")~" + name + R"~(")
            |> range(start: -20s)
            |> filter(fn: (r) => r._field == "x")
            |> yield(name: "acqui")
    )~");

    ASSERT_EQ(tables[0].size(), 2);
    EXPECT_TRUE(std::holds_alternative<double>(tables[0][0].value));
    EXPECT_EQ(std::get<double>(tables[0][0].value), 20.0);
    EXPECT_TRUE(std::holds_alternative<double>(tables[0][1].value));
    EXPECT_EQ(std::get<double>(tables[0][1].value), 10.0);

    ASSERT_EQ(tables.size(), 2);
    ASSERT_EQ(tables[1].size(), 1);
    EXPECT_TRUE(std::holds_alternative<std::int64_t>(tables[1][0].value));
    EXPECT_EQ(std::get<std::int64_t>(tables[1][0].value), 30);

    db.DeleteBucket(bucket);
}