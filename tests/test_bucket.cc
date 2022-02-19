#include <gtest/gtest.h>

#include "config.hh"

#include <influx/bucket.hh>

class BucketTest: public ::testing::Test {
protected:
    influx::Influx db = influx::test::db();
    influx::Bucket bucket = db.CreateBucket("bucket-" + influx::test::nowstring(), 1h);

    void TearDown() override
    {
        auto buckets = db.ListBuckets();
        for (auto& bucket: buckets) {
            if (!bucket.is_system_bucket()) {
                db.DeleteBucket(bucket);
            }
        }
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

TEST_F(BucketTest, should_be_null_by_default)
{
    influx::Bucket first;
    EXPECT_FALSE(first);
}

TEST_F(BucketTest, should_be_reassignable)
{
    influx::Bucket first;
    influx::Bucket second;
    EXPECT_EQ(first.name(), "");
    EXPECT_EQ(second.name(), "");

    first = db.CreateBucket("bucket-other", 1h);
    EXPECT_EQ(first.name(), "bucket-other");
    EXPECT_EQ(second.name(), "");

    second = std::move(first);
    EXPECT_EQ(first.name(), "");
    EXPECT_EQ(second.name(), "bucket-other");
}

TEST_F(BucketTest, should_throw_if_writing_to_null_bucket)
{
    influx::Bucket invalid;

    try {
        invalid << (influx::Measurement("m") << influx::Field{"field1", 42});
        EXPECT_TRUE(false);
    } catch (influx::NullBucketError) {
        // Success
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        std::vector<influx::Measurement> measurements = {
            (influx::Measurement("m") << influx::Field{"field1", 42}),
            (influx::Measurement("m") << influx::Field{"field1", 42})
        };
        invalid.Write(measurements);
        EXPECT_TRUE(false);
    } catch (influx::NullBucketError) {
        // Success
    } catch (...) {
        EXPECT_TRUE(false);
    }
}

TEST_F(BucketTest, should_throw_if_flusing_null_bucket)
{
    influx::Bucket invalid;

    try {
        invalid.Flush();
        EXPECT_TRUE(false);
    } catch (influx::NullBucketError) {
        // Success
    } catch (...) {
        EXPECT_TRUE(false);
    }
}

TEST_F(BucketTest, should_be_comparable)
{
    auto first = db.CreateBucket("bucket-a", 1h);
    auto second = db.CreateBucket("bucket-b", 1h);
    auto third = first;

    EXPECT_EQ(influx::Bucket(), influx::Bucket());
    EXPECT_EQ(first, third);
    EXPECT_NE(first, second);
}
