#include "ProtectionLib/HardwareKey.h"

#include <gtest/gtest.h>

namespace hwkey_test {

TEST(HWKey, HWKey_Stability)
{
	char hw_key_1[protection::HWKeyLength + 1];
	char hw_key_2[protection::HWKeyLength + 1];

	auto result = protection::ReadHWKey(hw_key_1);
	ASSERT_EQ(result, 0u); // We should never fail unless someboby is trying to hack/analyze us.
	result = protection::ReadHWKey(hw_key_2);
	ASSERT_EQ(result, 0u); // We should never fail unless someboby is trying to hack/analyze us.

	EXPECT_STREQ(hw_key_1, hw_key_2);
}

#if defined(__linux__)

TEST(HWKey, HWInfo_RequiredParameters)
{
	// Check that parameters which we require currently are present in HWInfo.

	protection::HWInfo hw;
	const auto result = protection::ReadHWInfo(hw);
	ASSERT_EQ(result, 0u);

	EXPECT_EQ(hw.used_sources.cpu, 1u);
	EXPECT_NE(hw.cpu_n_cores, 0u);
	EXPECT_NE(hw.cpu_brand_string[0], '\0');

	EXPECT_EQ(hw.used_sources.memory, 1u);
	EXPECT_NE(hw.memory_size, 0u);
}

TEST(HWKey, GetHWInfoSources)
{
	// Check that GetHWInfoSources can extract HWInfoSourceFlags from hardware key string.

	protection::HWInfo hw;
	auto result = protection::ReadHWInfo(hw);
	ASSERT_EQ(result, 0u);

	char hw_key[protection::HWKeyLength + 1];
	protection::HWInfoToKey(hw, hw_key);

	const auto hw_src = protection::GetHWInfoSources(hw_key);
	EXPECT_EQ(hw.used_sources.raw_value, hw_src.raw_value);
}

TEST(HWKey, CalculateHWInfoSourcesChecksum)
{
	protection::HWInfoSourceFlags hw_src {0};
	hw_src.cpu = 1;
	hw_src.memory = 1;
	hw_src.timezone = 1;
	hw_src.os_release = 1;
	hw_src.os_release_id = 1;
	hw_src.fs_id = 1;
	hw_src.n_disks = 2;
	hw_src.n_disk_wwids = 1;

	EXPECT_EQ(protection::CalculateHWInfoSourcesChecksum(hw_src), 0x9u);
}

#endif

}
