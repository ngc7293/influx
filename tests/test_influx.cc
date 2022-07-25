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

TEST_F(InfluxTest, should_allow_infinite_retention_buckets)
{
    auto bucket = db.CreateBucket("bucket" + influx::test::nowstring(), 0s);
    EXPECT_NE(bucket.id(), "");
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
    std::vector<std::pair<std::string, bool>> expected = {
        {"_monitoring", true},
        {"_tasks", true},
        {"test-bucket-a", false},
        {"test-bucket-b", false}
    };

    auto bucket_a = db.CreateBucket("test-bucket-a", 1h);
    auto bucket_b = db.CreateBucket("test-bucket-b", 1h);

    auto buckets = db.ListBuckets();
    for (const auto& expect: expected) {
        auto it = std::find_if(buckets.begin(), buckets.end(), [&](const auto& bucket) {
            return expect.first == bucket.name() && expect.second == bucket.is_system_bucket();
        });

        EXPECT_NE(it, buckets.end()) << "Did not find expected bucket '" << expect.first << "'";
    }

    // If you fail here, check that your token is an all-access token.
    EXPECT_EQ(buckets.size(), expected.size());
}

TEST_F(InfluxTest, should_list_all_buckets_with_paging)
{
    std::vector<std::pair<std::string, bool>> expected = {
        {"_monitoring", true},
        {"_tasks", true},
        {"test-bucket-a", false},
        {"test-bucket-b", false}
    };

    auto bucket_a = db.CreateBucket("test-bucket-a", 1h);
    auto bucket_b = db.CreateBucket("test-bucket-b", 1h);

    {
        auto buckets = db.ListBuckets(2);
        EXPECT_EQ(buckets[0].name(), "_monitoring");
        EXPECT_EQ(buckets[1].name(), "_tasks");
        EXPECT_EQ(buckets.size(), 2);
    }
    {
        auto buckets = db.ListBuckets(2, 2);
        EXPECT_EQ(buckets[0].name(), "test-bucket-a");
        EXPECT_EQ(buckets[1].name(), "test-bucket-b");
        EXPECT_EQ(buckets.size(), 2);
    }
}

TEST_F(InfluxTest, should_throw_if_invalid_limit_passed)
{
    try {
        auto bucket = db.ListBuckets(0, 0);
        EXPECT_TRUE(false);
    } catch (influx::InfluxError& e) {
        EXPECT_STREQ(e.what(), "Limit must be within range [1, 100]");
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        auto bucket = db.ListBuckets(101, 0);
        EXPECT_TRUE(false);
    } catch (influx::InfluxError& e) {
        EXPECT_STREQ(e.what(), "Limit must be within range [1, 100]");
    } catch (...) {
        EXPECT_TRUE(false);
    }

    try {
        auto bucket = db.ListBuckets(std::numeric_limits<std::size_t>::max(), 0);
        EXPECT_TRUE(false);
    } catch (influx::InfluxError& e) {
        EXPECT_STREQ(e.what(), "Limit must be within range [1, 100]");
    } catch (...) {
        EXPECT_TRUE(false);
    }
}

TEST_F(InfluxTest, should_get_single_bucket_by_id)
{
    auto name = influx::test::nowstring();
    auto id = db.CreateBucket(name, 1h).id();

    auto bucket = db.GetBucketById(id);
    EXPECT_EQ(bucket.id(), id);
    EXPECT_EQ(bucket.name(), name);
    EXPECT_FALSE(bucket.is_system_bucket());
}

TEST_F(InfluxTest, should_return_invalid_bucket_if_id_empty)
{
    EXPECT_FALSE(db.GetBucketById(""));
}

TEST_F(InfluxTest, should_get_single_bucket_by_name)
{
    {
        auto name = influx::test::nowstring();
        auto id = db.CreateBucket(name, 1h).id();

        auto bucket = db.GetBucketByName(name);
        EXPECT_EQ(bucket.id(), id);
        EXPECT_EQ(bucket.name(), name);
        EXPECT_FALSE(bucket.is_system_bucket());
    }
    {
        auto name = influx::test::nowstring();
        auto id = db.CreateBucket(name, 1h).id();

        auto bucket = db[name];
        EXPECT_EQ(bucket.id(), id);
        EXPECT_EQ(bucket.name(), name);
        EXPECT_FALSE(bucket.is_system_bucket());
    }
}


TEST_F(InfluxTest, should_return_invalid_bucket_if_name_empty)
{
    EXPECT_FALSE(db.GetBucketByName(""));
}

TEST_F(InfluxTest, should_return_parsed_query)
{
    auto name = influx::test::nowstring();
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
}

TEST(InitInfluxTest, should_throw_if_invalid_token_passed)
{
    const auto c = influx::test::config();
    influx::Influx db(c["host"], c["org"], "invalid");

    try {
        db.CreateBucket(influx::test::nowstring(), 1h);
    } catch (influx::InfluxRemoteError& e) {
        EXPECT_EQ(e.statusCode(), 401);
    } catch (...) {
        EXPECT_TRUE(false);
    }

}