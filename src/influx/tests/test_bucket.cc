#include <gtest/gtest.h>

#include "config.hh"

#include <influx/bucket.hh>

class BucketTest: public ::testing::Test {
protected:
    influx::Influx db = influx::test::db();
    influx::Bucket bucket = db.CreateBucket("bucket-" + influx::test::nowstring(), 1h);

    void TearDown() override
    {
        db.DeleteBucket(bucket);
    }
};

TEST_F(BucketTest, should_accept_measurements)
{
    bucket << (influx::Measurement("m") << influx::Field{"field1", 42});
    EXPECT_EQ(bucket.BufferedMeasurementsCount(), 1);
}

TEST_F(BucketTest, should_allow_flusing_measurements)
{
    bucket << (influx::Measurement("m") << influx::Field{"field1", 42});
    bucket.Flush();
}