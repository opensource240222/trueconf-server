#include "std-generic/cpplib/VS_Container.h"

#include <gtest/gtest.h>

namespace vs_container_test {

namespace {
const std::string test_key_1{"test_key_1"};
const std::string test_key_2{"test_key_2"};
const std::string test_key_3{"test_key_3"};
const std::string test_key_4{"test_key_4"};
const std::string test_key_5{"test_key_5"};

const bool b_test{true};
const int32_t i32_test{42};
const int64_t i64_test{1'000'000'000'000};
const double d_test{3.14159};
const string_view s_test{"Test string"};
const char bin_test[]{"VGVzdCBzdHJpbmc="};
} // anonymous namespace

TEST(VS_ContainerTest, DeletionOfOneValue)
{
	/* One value in container */

	VS_Container test;

	// bool
	EXPECT_TRUE(test.AddValue(test_key_1, b_test));
	EXPECT_TRUE(test.DeleteValue(test_key_1));
	ASSERT_TRUE(test.IsEmpty());
	// int32_t
	EXPECT_TRUE(test.AddValue(test_key_1, i32_test));
	EXPECT_TRUE(test.DeleteValue(test_key_1));
	ASSERT_TRUE(test.IsEmpty());
	// int64_t
	EXPECT_TRUE(test.AddValueI64(test_key_1, i64_test));
	EXPECT_TRUE(test.DeleteValue(test_key_1));
	ASSERT_TRUE(test.IsEmpty());
	// double
	EXPECT_TRUE(test.AddValue(test_key_1, d_test));
	EXPECT_TRUE(test.DeleteValue(test_key_1));
	ASSERT_TRUE(test.IsEmpty());
	// string
	EXPECT_TRUE(test.AddValue(test_key_1, s_test));
	EXPECT_TRUE(test.DeleteValue(test_key_1));
	ASSERT_TRUE(test.IsEmpty());
	// bin data
	EXPECT_TRUE(test.AddValue(test_key_1, bin_test, sizeof(bin_test)));
	EXPECT_TRUE(test.DeleteValue(test_key_1));
	ASSERT_TRUE(test.IsEmpty());
}

TEST(VS_ContainerTest, DeletionOfSeveralValues)
{
	/* Several values in container */

	VS_Container test;

	char buffer[1024] = {};
	size_t buff_size{sizeof(buffer)};

	// add values of different types in container
	EXPECT_TRUE(test.AddValue(test_key_1, b_test));
	EXPECT_TRUE(test.AddValueI32(test_key_2, i32_test));
	EXPECT_TRUE(test.AddValue(test_key_3, bin_test, sizeof(bin_test)));
	EXPECT_TRUE(test.AddValue(test_key_4, s_test));
	EXPECT_TRUE(test.AddValueI64(test_key_5, i64_test));
	ASSERT_TRUE(test.IsValid());
	ASSERT_EQ(test.Count(), 5u);
	// delete 1st value from container
	EXPECT_TRUE(test.DeleteValue(test_key_1));
	ASSERT_TRUE(test.IsValid());
	ASSERT_EQ(test.Count(), 4u);
	// check data serialization after 1st value deletion
	EXPECT_TRUE(test.Serialize(buffer, buff_size));
	test.Clear();
	EXPECT_TRUE(test.Deserialize(buffer, buff_size));
	ASSERT_TRUE(test.IsValid());
	ASSERT_EQ(test.Count(), 4u);
	// check deleted (1st) value
	bool b_val{false};
	ASSERT_FALSE(test.GetValue(test_key_1, b_val));
	// check next value (2nd) after deleted
	int32_t i32_val{0};
	EXPECT_TRUE(test.GetValue(test_key_2, i32_val));
	EXPECT_EQ(i32_test, i32_val);
	// check last (5th) value
	int64_t i64_val{0};
	EXPECT_TRUE(test.GetValue(test_key_5, i64_val));
	EXPECT_EQ(i64_test, i64_val);
	// delete last (5th) value
	EXPECT_TRUE(test.DeleteValue(test_key_5));
	ASSERT_TRUE(test.IsValid());
	ASSERT_EQ(test.Count(), 3u);
	// check data serialization after last (5th) value deletion
	buff_size = sizeof(buffer);
	EXPECT_TRUE(test.Serialize(buffer, buff_size));
	test.Clear();
	EXPECT_TRUE(test.Deserialize(buffer, buff_size));
	ASSERT_TRUE(test.IsValid());
	ASSERT_EQ(test.Count(), 3u);
	// check deleted (5th) value
	i64_val = 0;
	ASSERT_FALSE(test.GetValue(test_key_5, i64_val));
	// check prevous (4th) value after deleted
	std::string s_val;
	EXPECT_TRUE(test.GetValue(test_key_4, s_val));
	EXPECT_EQ(s_test, s_val);
	// delete middle (3rd) value
	EXPECT_TRUE(test.DeleteValue(test_key_3));
	ASSERT_TRUE(test.IsValid());
	ASSERT_EQ(test.Count(), 2u);
	// check data serialization after 3rd value deletion
	buff_size = sizeof(buffer);
	EXPECT_TRUE(test.Serialize(buffer, buff_size));
	test.Clear();
	EXPECT_TRUE(test.Deserialize(buffer, buff_size));
	ASSERT_TRUE(test.IsValid());
	ASSERT_EQ(test.Count(), 2u);
	// check deleted (3rd) value
	char bin_val[128];
	size_t bin_val_size{128};
	ASSERT_FALSE(test.GetValue(test_key_3, static_cast<void*>(bin_val), bin_val_size));
	// check next (4th) value after deleted
	s_val = "";
	EXPECT_TRUE(test.GetValue(test_key_4, s_val));
	EXPECT_EQ(s_test, s_val);
	// check previous (2nd) value after deleted
	i32_val = 0;
	EXPECT_TRUE(test.GetValue(test_key_2, i32_val));
	EXPECT_EQ(i32_test, i32_val);
}

} // namespace vs_container_test
