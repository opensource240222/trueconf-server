#include "std-generic/cpplib/TimeUtils.h"

#include <gtest/gtest.h>

namespace tu_test {

TEST(TimeUtils, UnixTimeToWindowsTime)
{
	EXPECT_EQ(116444736000000000, tu::UnixSecondsToWindowsTicks(std::chrono::system_clock::time_point())); // Start of Unix epoch
	EXPECT_EQ(94969899510000000, tu::UnixSecondsToWindowsTicks(std::chrono::system_clock::time_point(std::chrono::seconds(-(1ll << 31) - 1)))); // int32_t underflow
	EXPECT_EQ(137919572480000000, tu::UnixSecondsToWindowsTicks(std::chrono::system_clock::time_point(std::chrono::seconds(1ll << 31)))); // int32_t overflow (Year 2038 problem)
	EXPECT_EQ(159394408960000000, tu::UnixSecondsToWindowsTicks(std::chrono::system_clock::time_point(std::chrono::seconds(1ll << 32)))); // uint32_t overflow
}

TEST(TimeUtils, WindowsTimeToUnixTime)
{
	EXPECT_EQ(std::chrono::system_clock::time_point(), tu::WindowsTickToUnixSeconds(116444736000000000)); // Start of Unix epoch
	EXPECT_EQ(std::chrono::system_clock::time_point(std::chrono::seconds(-(1ll << 31) - 1)), tu::WindowsTickToUnixSeconds(94969899510000000)); // int32_t underflow
	EXPECT_EQ(std::chrono::system_clock::time_point(std::chrono::seconds(1ll << 31)), tu::WindowsTickToUnixSeconds(137919572480000000)); // int32_t overflow (Year 2038 problem)
	EXPECT_EQ(std::chrono::system_clock::time_point(std::chrono::seconds(1ll << 32)), tu::WindowsTickToUnixSeconds(159394408960000000)); // uint32_t overflow
}

TEST(TimeUtils, StringConvertions)
{
	auto now = time(nullptr);

	char timeStr[128] = { 0 };
	tu::TimeToNStr(std::chrono::system_clock::from_time_t(now), timeStr, 128);

	auto timeRes = tu::NStrToTimeT(timeStr);
	EXPECT_EQ(now, timeRes);
}

}
