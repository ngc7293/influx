#include <gtest/gtest.h>

#include <influx/influx.hh>
#include <influx/bucket.hh>

TEST(Transport, performs_simple_get_correctly)
{
    influx::InfluxDB db("http://localhost:8086", "f16db33158c03f29", "6sZqHjOeRHv9iayt2NoUcFzCToeNSs9lQ7UTvwqcqvnjnyakTsqBIdCvZnt3DGqCYU_OW5Gq6yyxOl5OmaS6ew==");

    try {
        auto buckets = db.ListBuckets();
        EXPECT_EQ(buckets.size(), 3);
        EXPECT_EQ(buckets[2].name(), "testbucket");
    } catch (influx::InfluxRemoteError e) {
        std::cerr << "Error status " << e.statusCode() << std::endl;
        EXPECT_TRUE(false);
    } catch (...) {
        EXPECT_TRUE(false);
    }
}