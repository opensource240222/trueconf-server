#pragma once

#include <cstdint>

namespace protection {

#if defined(_WIN32)
static const unsigned HWKeyLength = 32;
#elif defined(__linux__)
static const unsigned HWKeyLength = 40/*SHA1*/ + 8/*encrypted HWInfoSourceFlags*/ ;
#endif

struct HWInfo;

// Writes hardware key into the supplied buffer.
// This is the function that generally should be used.
// On success returns 0, otherwise returns opaque error code (see implementation for how to interpret the value).
uint32_t ReadHWKey(char hw_key[HWKeyLength + 1]);

// Reads hardware information into HWInfo structure.
// On success returns 0, otherwise returns opaque error code (see implementation for how to interpret the value).
uint32_t ReadHWInfo(HWInfo& hw);

// Calculates hardware key corresponding to supplied hardware information.
void HWInfoToKey(const HWInfo& hw, char hw_key[HWKeyLength + 1]);

// Computes md5(concat(hw_key, salt)) == expected_md5
bool CheckSaltedHWKey(const char hw_key[HWKeyLength], const char* salt, const unsigned char expected_md5[16]);
bool CheckSaltedHWKey(const char hw_key[HWKeyLength], const char* salt, const char expected_md5[32]);

// Computes md5(concat(hw_key, salt))
void GetSaltedHWKey(const char hw_key[HWKeyLength], const char* salt, char result_md5[33]);

#if defined(__linux__)

union HWInfoSourceFlags
{
	uint32_t raw_value;
	struct {
		uint32_t cpu : 1; // Were we able to extract all CPU information? (currently always set)
		uint32_t hypervisor : 1; // Did cpuid reported presence of a hypervisor?
		uint32_t memory : 1; // Were we able get size of RAM? (currently always set)
		uint32_t timezone : 1; // Were we able to read /etc/timezone ?
		uint32_t os_release : 1; // Were we able to read /etc/os_release ?
		uint32_t os_release_id : 1; // Were we able to extract ID field from /etc/os-release ?
		uint32_t os_release_version_id : 1; // Were we able to extract VERSION_ID field from /etc/os-release ?
		uint32_t machine_id : 1; // Were we able to read /etc/machine-id ?
		uint32_t fs_id : 1; // Was f_fsid field returned by statfs() non zero?
		uint32_t n_disks : 4; // Number of disks related to root fs that we have found.
		uint32_t n_disk_wwids : 4; // Number of disks for which we were able to read 'wwid'.
		uint32_t reserved : 11;
		uint32_t checksum : 4; // CRC-4-ITU of all other bits
	};
};
static_assert(sizeof(HWInfoSourceFlags) == 4, "");

// Extracts and decrypts information about data sources used to calculate a hardware key.
HWInfoSourceFlags GetHWInfoSources(char hw_key[HWKeyLength]);

// Calculates checksum of HWInfoSourceFlags
unsigned CalculateHWInfoSourcesChecksum(HWInfoSourceFlags hw_src);

#endif

#if defined(_WIN32)

struct HWInfo
{
	uint32_t system_drive_vsn; // Volume Serial Number of the drive containig Windows installation directory
	char windows_product_id[65]; // HKLM/Software/Microsoft/Windows NT/CurrentVersion/ProductId
	uint32_t windows_install_date; // HKLM/Software/Microsoft/Windows NT/CurrentVersion/InstallDate
	uint32_t arm_hwkey; // Result of VS_ArmReadHardwareKey()
};

#elif defined(__linux__)

#pragma pack(push, 1)
struct HWInfo
{
	uint8_t cpu_family; // As reported by leaf 01h
	uint8_t cpu_model; // As reported by leaf 01h
	uint8_t cpu_stepping; // As reported by leaf 01h
	uint8_t cpu_n_cores; // As reported by leaf 01h
	char cpu_brand_string[48]; // As reported by leafs 80000002h-80000004h
	// Various feature bitfields
	uint32_t cpuid_leaf_01_ecx;
	uint32_t cpuid_leaf_01_edx;
	uint32_t cpuid_leaf_07_0_ebx;
	uint32_t cpuid_leaf_07_0_ecx;
	uint32_t cpuid_leaf_07_0_edx; // For now this field is reserved (and set to 0) because there are no feature flags defined for edx
	uint32_t cpuid_leaf_80000001_ecx;
	uint32_t cpuid_leaf_80000001_edx;
	// Following fields are valid only for Intel CPUs
	uint8_t cpuid_leaf_02_descriptors[16]; // Sorted TLB/cache/prefetch descriptors returned by leaf 02h

	char hypervisor_id[12]; // As reported by cpuid leaf 40000000h

	uint64_t memory_size; // Memory size in bytes, as reported by sysinfo() in totalmem field
	uint64_t swap_size; // Swap size in bytes, as reported by sysinfo() in totalswap field

	char tz_name[48]; // Contents of /etc/timezone

	char os_release_id[32]; // Field ID from /etc/os-release
	char os_release_version_id[16]; // Field VERSION_ID from /etc/os-release

	char machine_id[32]; // Contents of /etc/machine-id

	// Parameters of the filesystem containing ID of the filesystem containing the running binary, as reported by statfs() in
	uint32_t fs_type; // f_type field
	uint64_t fs_id; // f_fsid field

	// Parameters of block devices corresponding to disks that contain the filesystem above
	struct Disk
	{
		// Contents of the corresponding files in sysfs directory for the device
		uint64_t size : 56; // Drive size in bytes
		uint64_t reserved_1 : 6;
		uint64_t is_wwid : 1; // 1=use 'wwid' field, 0=use 'vendor' and 'model' fields
		uint64_t is_filled : 1; // 1=array member is used
		union {
			char wwid[128];
			struct {
				char vendor[64];
				char model[64];
			};
		};
	};
	Disk disk[8];

	HWInfoSourceFlags used_sources;
};
#pragma pack(pop)

#endif

}
