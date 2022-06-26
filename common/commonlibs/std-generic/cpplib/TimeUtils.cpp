#include "TimeUtils.h"
#include "std-generic/clib/vs_time.h"

#include <cassert>
#include <cstring>
#include <sstream>

#ifdef _WIN32
#define timezone _timezone
#endif

namespace tu {

using win_time_tick = std::chrono::duration<int64_t, std::ratio<1, 10000000>>;
static const win_time_tick c_win_to_unix_time_diff(116444736000000000);

int64_t UnixSecondsToWindowsTicks(const std::chrono::system_clock::time_point tp)
{
	return (std::chrono::duration_cast<win_time_tick>(tp.time_since_epoch()) + c_win_to_unix_time_diff).count();
}

std::chrono::system_clock::time_point WindowsTickToUnixSeconds(const int64_t windowsTicks)
{
	if (windowsTicks <= 0) return std::chrono::system_clock::time_point();
	return std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(win_time_tick(windowsTicks) - c_win_to_unix_time_diff));
}

static const char c_NEUTRAL_TIME_FMT[] = "%Y/%m/%d %H.%M.%S GMT";
static const char c_RUS_TIMEFORMAT[] = "%d.%m.%Y %H:%M:%S";
static const char c_GLOBAL_TIME_FMT[] = "%Y %m/%d %H:%M:%S GMT";
static const char c_LOCAL_TIME_FMT[] =  "%Y %m/%d %H:%M:%S";
static const char c_ISO8601_Z_TIME_FMT[] = "%Y-%m-%dT%H:%M:%SZ";
static const char c_LF_FORMAT[] = "%d/%m/%Y:%H:%M:%S %z";

size_t TimeToString(const std::chrono::system_clock::time_point tp, const  char* format, char* OUT_buffer, const size_t buff_size, const bool need_local) {
	if (!OUT_buffer || !format) return 0;
	const auto unix_tp = std::chrono::system_clock::to_time_t(tp);
	tm tp_tm;
	if ((need_local ? localtime_r(&unix_tp, &tp_tm) : gmtime_r(&unix_tp, &tp_tm)) == nullptr)
		return 0;
	return std::strftime(OUT_buffer, buff_size, format, &tp_tm);
}

std::string TimeToString(const std::chrono::system_clock::time_point tp, const char* format, const bool need_local) {
	assert(format != nullptr);
	std::string result;
	const auto unix_tp = std::chrono::system_clock::to_time_t(tp);
	tm tp_tm;
	if ((need_local ? localtime_r(&unix_tp, &tp_tm) : gmtime_r(&unix_tp, &tp_tm)) == nullptr)
		return result;
	result.resize(std::strlen(format) + 48);
	const auto length = std::strftime(&result[0], result.size(), format, &tp_tm);
	result.resize(length);
	return result;
}

size_t TimeToNStr(const std::chrono::system_clock::time_point tp, char* OUT_buffer, const size_t buff_size) {
	return TimeToString(tp, c_NEUTRAL_TIME_FMT, OUT_buffer, buff_size);
}
size_t TimeToRuStr(const std::chrono::system_clock::time_point tp, char* OUT_buffer, const size_t buff_size) {
	return TimeToString(tp, c_RUS_TIMEFORMAT, OUT_buffer, buff_size);
}
size_t TimeToGStr(const std::chrono::system_clock::time_point tp, char* OUT_buffer, const size_t buff_size) {
	return TimeToString(tp, c_GLOBAL_TIME_FMT, OUT_buffer, buff_size);
}
size_t TimeToLStr(const std::chrono::system_clock::time_point tp, char* OUT_buffer, const size_t buff_size) {
	return TimeToString(tp, c_LOCAL_TIME_FMT, OUT_buffer, buff_size,true);
}
size_t TimeToLFStr(const std::chrono::system_clock::time_point tp, char* OUT_buffer, const size_t buff_size) {
	return TimeToString(tp, c_LF_FORMAT, OUT_buffer, buff_size,true);
}
size_t TimeToStr_ISO8601_Z(const std::chrono::system_clock::time_point tp, char* OUT_buffer, const size_t buff_size) {
	return TimeToString(tp, c_ISO8601_Z_TIME_FMT, OUT_buffer, buff_size);
}

time_t TimeStringToUnixTime(const char *str, const char* format)
{
	if (!str || !format) return false;

	std::tm tm = {};
	unsigned chars_parsed = 0;
	if (sscanf(str, format, &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &chars_parsed) != 6 || chars_parsed != strlen(str))
		return false; // Parsing failed
	tm.tm_year -= 1900;
	tm.tm_mon -= 1;
	//tm.tm_mday -= 1;
	tm.tm_isdst = 0;	// do not use daylight savings time affection because timezone do not include it in result
	return std::mktime(&tm) - timezone;
}

time_t ISO8601_ZStrToTimeT(const char *str)
{
	return TimeStringToUnixTime(str, "%4d-%2d-%2dT%2d:%2d:%2dZ%n");
}

time_t NStrToTimeT(const char *str) {
	return TimeStringToUnixTime(str, "%4d/%2d/%2d %2d.%2d.%2d GMT%n");
}

}
