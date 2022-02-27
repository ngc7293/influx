#include <iostream>

#include <gtest/gtest.h>

#include <influx/bucket.hh>
#include <influx/influx.hh>

#include "config.hh"

namespace std::chrono {
    bool operator==(const influx::Timestamp& lhs, const std::chrono::nanoseconds& rhs)
    {
        return lhs == influx::Timestamp() + std::chrono::duration_cast<std::chrono::system_clock::duration>(rhs);
    }

    std::ostream& operator<<(std::ostream& os, const influx::Timestamp& rhs)
    {
        return (os << std::chrono::duration_cast<std::chrono::nanoseconds>(rhs.time_since_epoch()).count() << "ns");
    }
}

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

    EXPECT_EQ(tables[0][0].start, 1645897891687426831ns);
    EXPECT_EQ(tables[0][0].stop,  1645897911687426831ns);
    EXPECT_EQ(tables[0][0].time,  1645897896590333377ns);

    EXPECT_EQ(tables[0][0].tags.at("domain"), "1");
    EXPECT_EQ(tables[1][0].tags.at("client"), "2");
}
