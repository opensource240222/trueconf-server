#pragma once

#include <string>
#include <cstdint>

extern std::string GetOSInfoString(void);
extern std::string GetCPUArchitectureString(void);
extern std::string GetFriendlyOSNameString(void);
extern std::string GetCPUInfoString(unsigned int cpu_number = 0);
extern unsigned int GetCPUsCount(unsigned int *active_processors = nullptr);
extern bool GetPhysicalMemoryInfo(uint64_t &total_bytes, uint64_t &available_bytes, unsigned int &memory_load);
extern std::string GetComputerNameString(void);
