#include <gtest/gtest.h>

#include "config.hh"

#include <influx/bucket.hh>
#include <influx/influx.hh>
#include <influx/measurement.hh>

using namespace std::chrono_literals;

class InfluxTest: public ::testing::Test {
protected:
    influx::Influx db = influx::test::db();

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

TEST_F(InfluxTest, should_create_and_delete_buckets)
{
    auto bucket = db.CreateBucket("bucket" + influx::test::nowstring(), 2h);
    EXPECT_NE(bucket.id(), "");

    db.DeleteBucket(bucket);
    EXPECT_EQ(bucket.id(), "");
}

TEST_F(InfluxTest, should_prevent_creation_of_bucket_with_too_short_retention_policy)
{
    try {
        auto bucket = db.CreateBucket("bucket" + influx::test::nowstring(), 1s);
        EXPECT_TRUE(false);
    } catch (influx::InfluxError& e) {
        EXPECT_STREQ(e.what(), "Retention policy must be at least an hour");
    } catch (...) {
        EXPECT_TRUE(false);
    }
}

TEST_F(InfluxTest, should_list_all_buckets)
{
    std::vector<std::string> expected = {
        "test-bucket-a",
        "test-bucket-b"
    };

    auto bucket_a = db.CreateBucket("test-bucket-a", 1h);
    auto bucket_b = db.CreateBucket("test-bucket-b", 1h);

    auto buckets = db.ListBuckets();
    for (const auto& expect: expected) {
        auto it = std::find_if(buckets.begin(), buckets.end(), [&](const auto& bucket) {
            return expect == bucket.name();
        });

        EXPECT_NE(it, buckets.end()) << "Did not find expected bucket '" << expect << "'";
    }

    // If you fail here, check that your token is not an all-access token.
    EXPECT_EQ(buckets.size(), expected.size());
}

TEST_F(InfluxTest, should_get_single_bucket)
{
    {
        auto name = influx::test::nowstring();
        auto id = db.CreateBucket(name, 1h).id();

        auto bucket = db.GetBucket(id);
        EXPECT_EQ(bucket.id(), id);
        EXPECT_EQ(bucket.name(), name);
        EXPECT_FALSE(bucket.is_system_bucket());
    }
    {
        auto name = influx::test::nowstring();
        auto id = db.CreateBucket(name, 1h).id();

        auto bucket = db[id];
        EXPECT_EQ(bucket.id(), id);
        EXPECT_EQ(bucket.name(), name);
        EXPECT_FALSE(bucket.is_system_bucket());
    }
}

TEST_F(InfluxTest, should_query_data)
{
    auto bucket = db.CreateBucket("test-bucket-a", 1h);

    bucket << (influx::Measurement("m") << influx::Field{"val", 42});
    bucket.Flush();
    auto result = db.Query(R"~~(
        from(bucket: "test-bucket-a")
            |> range(start: -1m)
            |> filter(fn: (r) => r._field == "val")
            |> yield(name: "val")
    )~~");
}