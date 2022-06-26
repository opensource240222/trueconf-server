#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace tu { /* time utils */

int64_t UnixSecondsToWindowsTicks(const std::chrono::system_clock::time_point tp);
std::chrono::system_clock::time_point WindowsTickToUnixSeconds(const int64_t windowsTicks);

size_t TimeToString(const std::chrono::system_clock::time_point tp, const char* format, char* OUT_buffer, const size_t buff_size, const bool need_local = false);
std::string TimeToString(const std::chrono::system_clock::time_point tp, const char* format, const bool need_local = false);

size_t TimeToNStr(const std::chrono::system_clock::time_point tp, char* OUT_buffer, const size_t buff_size);
size_t TimeToRuStr(const std::chrono::system_clock::time_point tp, char* OUT_buffer, const size_t buff_size);
size_t TimeToGStr(const std::chrono::system_clock::time_point tp, char* OUT_buffer, const size_t buff_size);
size_t TimeToLStr(const std::chrono::system_clock::time_point tp, char* OUT_buffer, const size_t buff_size);
size_t TimeToLFStr(const std::chrono::system_clock::time_point tp, char* OUT_buffer, const size_t buff_size);
size_t TimeToStr_ISO8601_Z(const std::chrono::system_clock::time_point tp, char* OUT_buffer, const size_t buff_size);

time_t ISO8601_ZStrToTimeT(const char* str);
time_t NStrToTimeT(const char *str);

}
