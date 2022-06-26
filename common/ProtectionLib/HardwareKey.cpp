#include "ProtectionLib/HardwareKey.h"
#include "ProtectionLib/Lib.h"
#include "ProtectionLib/MD5.h"
#include "ProtectionLib/Protection.h"
#include "ProtectionLib/SHA1.h"

#if defined(_WIN32)
#	include <Windows.h>
#elif defined(__linux__)
#	include "ProtectionLib/LinuxSyscall.h"
#	include "ProtectionLib/LinuxTypes.h"

#	include <cpuid.h>
#	include <linux/errno.h> // errno values (E*)
#	include <linux/fcntl.h> // O_RDONLY, O_PATH, O_DIRECTORY, O_CLOEXEC,
#	include <linux/magic.h> // PROC_SUPER_MAGIC, SYSFS_MAGIC, BTRFS_SUPER_MAGIC
#	include <sys/stat.h> // S_ISREG
#	include <sys/syscall.h> // Syscall numbers (SYS_*)
#endif

#include <assert.h>
#include <limits.h> // INT_MIN

#if defined(_WIN32)
#	pragma comment(lib, "Advapi32.lib")
#endif

#if !defined(__has_feature)
#	define __has_feature(x) 0
#endif
#if __has_feature(memory_sanitizer)
extern "C" void __msan_unpoison(const volatile void* a, size_t size);
#endif

#define HWKEY_VERBOSE_LOGS 0
#if HWKEY_VERBOSE_LOGS
#	include <errno.h>
#	include <stdio.h>
#endif

namespace protection {

// Errors in this file consist of three parts:
// 1. type - identifies a type of error (required file missing, syscall failure, etc.);
// 2. context - identifies logical location in the code where the error happened (extraction of parameter A, etc.);
// 3. value of errno - accompanies syscall failure errors.

#define HWRESULT(type, context, sys_errno) ( \
	(static_cast<uint32_t>(type) & 0xff) | \
	(static_cast<uint32_t>(context) & 0xff) << 8 | \
	(static_cast<uint32_t>(sys_errno) & 0xffff) << 16 \
	)

#define HWRESULT_WITH_CONTEXT(error, context) ( \
	(static_cast<uint32_t>(error) & 0xffff00ff) | (static_cast<uint32_t>(context) & 0xff) << 8 \
	)

// Special value used to indicate that operation was unsuccessful.
#define HWRESULT_FALSE 0x000000ff

// To make life of crackers harder we encrypt errors before putting them into logs.
// To simplify this process we return errors to the outside already encrypted.
// To decrypt error x, calculate: (x ^ c_hwerr_xor_mask * c_hwsf_factor) * c_hwerr_factor_inv ^ c_hwsf_xor_mask
static const uint32_t c_hwerr_xor_mask = 0x2b15dd05;
static const uint32_t c_hwerr_factor = 2207599u;
static const uint32_t c_hwerr_factor_inv = 2396032911u;
static_assert(c_hwerr_factor % 2 == 1, "c_hwerr_factor must be odd");
static_assert(c_hwerr_factor * c_hwerr_factor_inv == 1, "c_hwerr_factor_inv should be a modular multiplicative inverse of c_hwerr_factor");
#define HWRESULT_ENCRYPT(x) (((x) ^ c_hwerr_xor_mask) * c_hwerr_factor ^ (c_hwerr_xor_mask * c_hwerr_factor));
#define HWRESULT_DECRYPT(x) (((x) ^ (c_hwerr_xor_mask * c_hwerr_factor)) * c_hwerr_factor_inv ^ c_hwerr_xor_mask);

SECURE_FUNC_INTERNAL
static unsigned FormatU32(uint32_t x, char buffer[10])
{
	const unsigned len =
		x < 10         ? 1 :
		x < 100        ? 2 :
		x < 1000       ? 3 :
		x < 10000      ? 4 :
		x < 100000     ? 5 :
		x < 1000000    ? 6 :
		x < 10000000   ? 7 :
		x < 100000000  ? 8 :
		x < 1000000000 ? 9 :
		10;
	// Fill decimal digits from the least significant to the most significant.
	buffer += len;
	do
	{
		*--buffer = '0' + x % 10;
		x /= 10;
	} while (x);
	return len;
}

SECURE_FUNC
uint32_t ReadHWKey(char hw_key[HWKeyLength + 1])
{
	HWInfo hw;
	memset(&hw, 0, sizeof(hw));
	const auto result = ReadHWInfo(hw);
	if (result == 0)
		HWInfoToKey(hw, hw_key);
	return result;
}

SECURE_FUNC
bool CheckSaltedHWKey(const char hw_key[HWKeyLength], const char* salt, const unsigned char expected_md5[16])
{
	MD5 md5;
	md5.Update(hw_key, HWKeyLength);
	md5.Update(salt, strlen(salt));
	md5.Final();
	unsigned char result_md5[16];
	md5.GetBytes(result_md5);
	return memcmp(result_md5, expected_md5, 16) == 0;
}

SECURE_FUNC
bool CheckSaltedHWKey(const char hw_key[HWKeyLength], const char* salt, const char expected_md5[32])
{
	unsigned char raw_expected_md5[16];
	for (unsigned i = 0; i < 16; ++i)
	{
		unsigned char x = 0;

		const char hi = expected_md5[i*2];
		if (hi >= '0' && hi <= '9')
			x |= (hi - '0') << 4;
		else if (hi >= 'A' && hi <= 'F')
			x |= (hi - 'A' + 10) << 4;
		else if (hi >= 'a' && hi <= 'f')
			x |= (hi - 'a' + 10) << 4;
		else
			return false;

		const char lo = expected_md5[i*2+1];
		if (lo >= '0' && lo <= '9')
			x |= lo - '0';
		else if (lo >= 'A' && lo <= 'F')
			x |= lo - 'A' + 10;
		else if (lo >= 'a' && lo <= 'f')
			x |= lo - 'a' + 10;
		else
			return false;

		raw_expected_md5[i] = x;
	}
	return CheckSaltedHWKey(hw_key, salt, raw_expected_md5);
}

SECURE_FUNC
void GetSaltedHWKey(const char hw_key[HWKeyLength], const char* salt, char result_md5[32])
{
	MD5 md5;
	md5.Update(hw_key, HWKeyLength);
	md5.Update(salt, strlen(salt));
	md5.Final();
	md5.GetString(result_md5);
}

#if defined(_WIN32)

SECURE_FUNC_INTERNAL
static unsigned FormatI32(int32_t x, char buffer[11])
{
	if (x == INT_MIN)
	{
		// Special case for INT_MIX because we can't negate that value.
		buffer[0]  = '-';
		buffer[1]  = '2';
		buffer[2]  = '1';
		buffer[3]  = '4';
		buffer[4]  = '7';
		buffer[5]  = '4';
		buffer[6]  = '8';
		buffer[7]  = '3';
		buffer[8]  = '6';
		buffer[9]  = '4';
		buffer[10] = '8';
		return 11;
	}

	if (x < 0)
	{
		*buffer = '-';
		return FormatU32(-x, buffer + 1) + 1;
	}
	else
		return FormatU32(x, buffer);
}

SECURE_FUNC
uint32_t ReadHWInfo(HWInfo& hw)
{
	char dir[MAX_PATH];
	if (GetWindowsDirectory(dir, MAX_PATH) > 0)
	{
		if (((dir[0] >= 'A' && dir[0] <= 'Z')
		  || (dir[0] >= 'a' && dir[0] <= 'z'))
		 && dir[1] == ':'
		 && dir[2] == '\\')
		{
			dir[3] = '\0';
			DWORD vsn;
			DWORD unused_1;
			DWORD unused_2;
			GetVolumeInformation(dir, NULL, 0, &vsn, &unused_1, &unused_2, NULL, 0);
			hw.system_drive_vsn = vsn;
		}
	}

	HKEY win_key = 0;
	RegOpenKeyExA(HKEY_LOCAL_MACHINE, SECURE_STRING("Software\\Microsoft\\Windows NT\\CurrentVersion"), 0, KEY_READ | KEY_WOW64_64KEY, &win_key);
	DWORD reg_size = sizeof(hw.windows_product_id);
	if (RegQueryValueExA(win_key, SECURE_STRING("ProductId"), 0, 0, reinterpret_cast<LPBYTE>(&hw.windows_product_id), &reg_size) != ERROR_SUCCESS)
		memset(hw.windows_product_id, 0, sizeof(hw.windows_product_id));
	reg_size = sizeof(hw.windows_install_date);
	if (RegQueryValueExA(win_key, SECURE_STRING("InstallDate"), 0, 0, reinterpret_cast<LPBYTE>(&hw.windows_install_date), &reg_size) != ERROR_SUCCESS)
		hw.windows_install_date = 0;
	RegCloseKey(win_key);

	hw.arm_hwkey = VS_ArmReadHardwareKey();

	return 0;
}

SECURE_FUNC
void HWInfoToKey(const HWInfo& hw, char hw_key[HWKeyLength + 1])
{
	constexpr size_t int_str_length = 1/*minus sign*/ + 10;
	char key[int_str_length * 3 + sizeof(hw.windows_product_id) + 1];
	char* p = key;

	if (hw.system_drive_vsn)
		p += FormatI32(hw.system_drive_vsn, p);
	const auto product_id_len = strlen(hw.windows_product_id);
	memcpy(p, hw.windows_product_id, product_id_len);
	p += product_id_len;
	if (hw.windows_install_date)
		p += FormatI32(hw.windows_install_date, p);
	p += FormatI32(hw.arm_hwkey, p);
	*p = '\0';

	MD5 md5;
	md5.Update(key, strlen(key));
	md5.Final();
	md5.GetString(hw_key);
}

#elif defined(__linux__)

#if HWKEY_VERBOSE_LOGS
#	define PERROR(fn, id, sys_err) do { errno = sys_err; fprintf(stderr, "%s: %s(%s): %m\n", __func__, fn, id); } while (0)
#else
#	define PERROR(fn, id, sys_err) ((void)0)
#endif

// For some syscalls we want to use 64-bit versions on i386 to avoid EOVERFLOW errors.
// So we select appropriate structures and syscall numbers for each architecture.
#if __i386__

using struct_stat = struct linux_stat64;
using struct_statfs = struct linux_statfs64;
static constexpr auto sys_stat = SYS_stat64;
static constexpr auto sys_fstat = SYS_fstat64;
static constexpr auto sys_fstatat = SYS_fstatat64;
static constexpr auto sys_statfs = SYS_statfs64;
static constexpr auto sys_fstatfs = SYS_fstatfs64;
#define STATFS_ARGS(path, buf) path, sizeof(linux_statfs64), buf
#define FSTATFS_ARGS(fd, buf) fd, sizeof(linux_statfs64), buf

#elif __x86_64__

using struct_stat = struct linux_stat;
using struct_statfs = struct linux_statfs;
static constexpr auto sys_stat = SYS_stat;
static constexpr auto sys_fstat = SYS_fstat;
static constexpr auto sys_fstatat = SYS_newfstatat;
static constexpr auto sys_statfs = SYS_statfs;
static constexpr auto sys_fstatfs = SYS_fstatfs;
#define STATFS_ARGS(path, buf) path, buf
#define FSTATFS_ARGS(fd, buf) fd, buf

#else
#	error Unknown architecture
#endif

// We use our own definition for a type that can store device IDs.
// Definitions of dev_t in libc and the kernel are not appropriate because in
// both kernel and libc device IDs are stores in way not compatible with
// structures used in syscalls.
using device_id_t = uint32_t;

using HWResult = uint32_t;

enum class HWError : uint8_t
{
	buffer_overflow = 1,
	debugger_found,
	invalid_disk_size,
	invalid_memory_size,
	invalid_mount_device,
	invalid_mountinfo_format,
	no_btrfs_fs,
	no_cpuid_function_01,
	no_cpuid_function_07,
	no_cpuid_function_80000001,
	no_cpuid_function_80000004,
	no_disk_size_file,
	no_filesystem_type,
	path_forgery,
	syscall_fstat_fail,
	syscall_fstatat_fail,
	syscall_fstatfs_fail,
	syscall_getdents64_fail,
	syscall_open_fail,
	syscall_openat_fail,
	syscall_read_fail,
	syscall_stat_fail,
	syscall_statfs_fail,
	syscall_sysinfo_fail,
	too_many_disks,
	wrong_filesystem_type,
};

enum class HWContext : uint8_t
{
	blockdev_device = 1, // "device/" is sysfs node for a block device
	blockdev_partition, // "partition" is sysfs node for a block device
	blockdev_size, // "size" is sysfs node for a block device
	blockdev_slaves, // "slaves/" is sysfs node for a block device
	blockdev_slaves_entry, // "slaves/*" is sysfs node for a block device
	blockdev_wwid, // "wwid" is sysfs node for a block device
	btrfs, // "/sys/fs/btrfs/"
	btrfs_device_dev, // "/sys/fs/btrfs/*/devices/*/dev"
	btrfs_devices, // "/sys/fs/btrfs/*/devices/" (main pass)
	btrfs_devices_entry, // "/sys/fs/btrfs/*/devices/*"
	btrfs_devices_search, // "/sys/fs/btrfs/*/devices/" (search pass)
	btrfs_entry, // "/sys/fs/btrfs/*"
	disk_model, // "model" is sysfs node for a disk device
	disk_vendor, // "vendor" is sysfs node for a disk device
	disk_wwid, // "wwid" is sysfs node for a disk device
	etc_machine_id, // "/etc/machine-id"
	etc_os_release, // "/etc/os-release"
	etc_timezone, // "/etc/timezone"
	mount_device, // Device name extracted from "/proc/self/mountinfo"
	partition_parent, // ".." in sysfs node for a partition block device
	proc_self_mountinfo, // "/proc/self/mountinfo"
	procfs, // "/proc/"
	root, // "/"
	sysfs, // "/sys/"
	sysfs_device_id_link, // "/sys/dev/block/MAJOR:MINOR"
	usr_lib_os_release, // "/usr/lib/os-release"
};

// RAII wrapper over a file descriptor.
namespace {
struct FD
{
	int value;

	SECURE_FUNC_INLINE
	FD(int value_ = -1) : value(value_) {}
	SECURE_FUNC_INLINE_END

	FD(const FD&) = delete;
	FD& operator=(const FD&) = delete;

	SECURE_FUNC_INLINE
	FD(FD&& x) noexcept
		: value(x.value)
	{
		x.value = -1;
	}
	SECURE_FUNC_INLINE_END

	SECURE_FUNC_INLINE
	FD& operator=(FD&& x) noexcept
	{
		if (this == &x)
			return *this;

		if (value != -1)
		{
			DECLARE_SECURE_SYSCALL_VARS;
			SECURE_SYSCALL(SYS_close, value);
			if (sys_err)
				PERROR("close", "", sys_err);
		}
		value = x.value;
		x.value = -1;
		return *this;
	}
	SECURE_FUNC_INLINE_END

	SECURE_FUNC_INLINE
	~FD()
	{
		if (value == -1)
			return;

		DECLARE_SECURE_SYSCALL_VARS;
		SECURE_SYSCALL(SYS_close, value);
		if (sys_err)
			PERROR("close", "", sys_err);
	}
	SECURE_FUNC_INLINE_END
};
}

template <size_t N>
SECURE_FUNC_INLINE
static void SetBuffer(char (&buffer)[N], const void* data, size_t size)
{
	memcpy(buffer, data, size <= N ? size : N);
}
SECURE_FUNC_INLINE_END

template <class T, size_t N>
SECURE_FUNC_INLINE
static constexpr size_t ArraySize(T (&)[N]) { return N; }
SECURE_FUNC_INLINE_END

// Parses unsigned decimal integer, advances 'p' to the first character that is not a decimal digit.
SECURE_FUNC_INTERNAL
static uint64_t ParseUnsigned(const char*& p, const char* p_end)
{
	uint64_t x = 0;
	while (p < p_end)
	{
		if (*p >= '0' && *p <= '9')
			x = x * 10 + (*p++ - '0');
		else
			break;
	}
	return x;
}

// Formats Linux (block/char) device ID into usual format: "major:minor".
SECURE_FUNC_INTERNAL
static unsigned FormatDeviceID(device_id_t dev, char buffer[10])
{
	auto p = buffer;
	p += FormatU32((dev >> 8) & 0xffff, p);
	*p++ = ':';
	p += FormatU32(dev & 0xff, p);
	*p = '\0';
	return p - buffer;
}

// Checks that 'fd' refers to an inode that resides on a specified device ('device_id').
SECURE_FUNC_INTERNAL
static HWResult VerifyDeviceID(int fd, device_id_t device_id)
{
	DECLARE_SECURE_SYSCALL_VARS;

	struct_stat st;
	SECURE_SYSCALL(sys_fstat, fd, &st);
	if (sys_err)
	{
		PERROR("fstat", "", sys_err);
		return HWRESULT(HWError::syscall_fstat_fail, 0, sys_err);
	}
#if __has_feature(memory_sanitizer)
	__msan_unpoison(&st, sizeof(st));
#endif
	if (st.st_dev != device_id)
	{
#if HWKEY_VERBOSE_LOGS
		fprintf(stderr, "File/directory %i is located on a wrong device: %04xh != %04xh (expected)\n", fd, static_cast<unsigned>(st.st_dev), static_cast<unsigned>(device_id));
#endif
		return HWRESULT(HWError::path_forgery, 0, 0);
	}
	return 0;
}

// Reads whole file in the supplied buffer. Sets 'size' to the size of read data.
// Returns 0 when it reads file successfully.
// Returns HWRESULT_FALSE when the file is missing or is not readable.
// Optionally, if expected_device_id != 0, verifies that the file resides on a correct device.
SECURE_FUNC_INTERNAL
static HWResult ReadFile(int dir_fd, const char* name, void* buffer, size_t& size, device_id_t expected_device_id = 0)
{
	assert(name);
	assert(dir_fd != AT_FDCWD && "Don't use paths relative to the current directory.");

	DECLARE_SECURE_SYSCALL_VARS;

	SECURE_SYSCALL(SYS_openat, dir_fd, name, O_RDONLY | O_CLOEXEC);
	if (sys_err)
	{
		if (sys_err == ENOENT)
			return HWRESULT_FALSE;
		PERROR("openat", name, sys_err);
		return HWRESULT(HWError::syscall_openat_fail, 0, sys_err);
	}
	FD fd(sys_ret);

	if (expected_device_id != 0)
	{
		const auto result = VerifyDeviceID(fd.value, expected_device_id);
		if (result != 0)
			return result;
	}

	const size_t orig_size = size;
	while (size > 0)
	{
		SECURE_SYSCALL(SYS_read, fd.value, buffer, size);
		if (sys_err)
		{
			PERROR("read", name, sys_err);
			size = orig_size - size;
			// If we got an error right away treat the file as missing. This happens with some files in sysfs.
			if (size == 0)
				return HWRESULT_FALSE;
			return HWRESULT(HWError::syscall_read_fail, 0, sys_err);
		}
		assert(sys_ret >= 0);
		const auto read_result = static_cast<size_t>(sys_ret);
#if __has_feature(memory_sanitizer)
		__msan_unpoison(buffer, read_result);
#endif
		if (read_result == 0)
			break;

		buffer = static_cast<char*>(buffer) + read_result;
		size -= read_result;
	}

	size = orig_size - size;
	return 0;
}

// Opens a root of a special filesystem (procfs, sysfs) and validates the file descriptor.
SECURE_FUNC_INTERNAL
static HWResult OpenSpecialFS(const char* name, uint32_t fs_type, FD& fd, device_id_t& device_id)
{
	DECLARE_SECURE_SYSCALL_VARS;

	SECURE_SYSCALL(SYS_open, name, O_PATH | O_DIRECTORY | O_CLOEXEC);
	if (sys_err)
	{
		PERROR("open", name, sys_err);
		return HWRESULT(HWError::syscall_open_fail, 0, sys_err);
	}
	fd = FD(sys_ret);

	struct_statfs sfs;
	SECURE_SYSCALL(sys_fstatfs, FSTATFS_ARGS(fd.value, &sfs));
	if (sys_err)
	{
		PERROR("fstatfs", name, sys_err);
		return HWRESULT(HWError::syscall_fstatfs_fail, 0, sys_err);
	}
#if __has_feature(memory_sanitizer)
	__msan_unpoison(&sfs, sizeof(sfs));
#endif
	if (sfs.f_type != fs_type)
		return HWRESULT(HWError::wrong_filesystem_type, 0, 0);

	struct_stat st;
	SECURE_SYSCALL(sys_fstat, fd.value, &st);
	if (sys_err)
	{
		PERROR("fstat", name, sys_err);
		return HWRESULT(HWError::syscall_fstat_fail, 0, sys_err);
	}
#if __has_feature(memory_sanitizer)
	__msan_unpoison(&st, sizeof(st));
#endif
	device_id = st.st_dev;

	return 0;
}

SECURE_FUNC_INTERNAL
static HWResult ReadHWInfo_CPU(HWInfo& hw)
{
	uint32_t eax, ebx, ecx, edx;

	__cpuid(0, eax, ebx, ecx, edx);
	const unsigned max_leaf = eax;
	if (max_leaf < 1)
		return HWRESULT(HWError::no_cpuid_function_01, 0, 0);

	// Read CPU family/model/stepping, number of cores and feature flags
	__cpuid(1, eax, ebx, ecx, edx);
	const uint32_t family_id = (eax >> 8) & 0xf;
	hw.cpu_family = family_id;
	if (family_id == 0x0f)
		hw.cpu_family += (eax >> 20) & 0xff;
	hw.cpu_model = (eax >> 4) & 0xf;
	if (family_id == 0x6 || family_id == 0xf)
		hw.cpu_model |= (eax >> 12) & 0xf0;
	hw.cpu_stepping = (eax) & 0xf;
	hw.cpu_n_cores = (ebx >> 16) & 0xff;
	hw.cpuid_leaf_01_ecx = ecx;
	hw.cpuid_leaf_01_edx = edx;
	const bool hypervisor_present = (ecx & 0x80000000) != 0;

	if (max_leaf < 7)
		return HWRESULT(HWError::no_cpuid_function_07, 0, 0);

	// Read structured extended feature flags
	__cpuid_count(7, 0, eax, ebx, ecx, edx);
	hw.cpuid_leaf_07_0_ebx = ebx;
	hw.cpuid_leaf_07_0_ecx = ecx;
	hw.cpuid_leaf_07_0_edx = 0;

	if (hypervisor_present)
	{
		__cpuid(0x40000000, eax, ebx, ecx, edx);
		*reinterpret_cast<uint32_t*>(&hw.hypervisor_id[0]) = ebx;
		*reinterpret_cast<uint32_t*>(&hw.hypervisor_id[4]) = ecx;
		*reinterpret_cast<uint32_t*>(&hw.hypervisor_id[8]) = edx;

		hw.used_sources.hypervisor = 1;
	}

	__cpuid(0x80000000, eax, ebx, ecx, edx);
	const unsigned max_ext_leaf = eax;
	if (max_ext_leaf < 0x80000001)
		return HWRESULT(HWError::no_cpuid_function_80000001, 0, 0);

	// Read extended feature flags
	__cpuid(0x80000001, eax, ebx, ecx, edx);
	hw.cpuid_leaf_80000001_ecx = ecx;
	hw.cpuid_leaf_80000001_edx = edx;

	if (max_ext_leaf < 0x80000004)
		return HWRESULT(HWError::no_cpuid_function_80000004, 0, 0);

	// Read processor brand string
	__cpuid(0x80000002, eax, ebx, ecx, edx);
	*reinterpret_cast<uint32_t*>(&hw.cpu_brand_string[ 0]) = eax;
	*reinterpret_cast<uint32_t*>(&hw.cpu_brand_string[ 4]) = ebx;
	*reinterpret_cast<uint32_t*>(&hw.cpu_brand_string[ 8]) = ecx;
	*reinterpret_cast<uint32_t*>(&hw.cpu_brand_string[12]) = edx;
	__cpuid(0x80000003, eax, ebx, ecx, edx);
	*reinterpret_cast<uint32_t*>(&hw.cpu_brand_string[16]) = eax;
	*reinterpret_cast<uint32_t*>(&hw.cpu_brand_string[20]) = ebx;
	*reinterpret_cast<uint32_t*>(&hw.cpu_brand_string[24]) = ecx;
	*reinterpret_cast<uint32_t*>(&hw.cpu_brand_string[28]) = edx;
	__cpuid(0x80000004, eax, ebx, ecx, edx);
	*reinterpret_cast<uint32_t*>(&hw.cpu_brand_string[32]) = eax;
	*reinterpret_cast<uint32_t*>(&hw.cpu_brand_string[36]) = ebx;
	*reinterpret_cast<uint32_t*>(&hw.cpu_brand_string[40]) = ecx;
	*reinterpret_cast<uint32_t*>(&hw.cpu_brand_string[44]) = edx;

	// Read TLB/cache/prefetch information (only on Intel)
	__cpuid(2, eax, ebx, ecx, edx);
	if ((eax & 0x80000000) == 0)
	{
		hw.cpuid_leaf_02_descriptors[ 0] = (eax >> 24) & 0xff;
		hw.cpuid_leaf_02_descriptors[ 1] = (eax >> 16) & 0xff;
		hw.cpuid_leaf_02_descriptors[ 2] = (eax >>  8) & 0xff;
		hw.cpuid_leaf_02_descriptors[ 3] = 0; // Least significant byte of eax always contains 0x01 and must be ignored.
	}
	if ((ebx & 0x80000000) == 0)
	{
		hw.cpuid_leaf_02_descriptors[ 4] = (ebx >> 24) & 0xff;
		hw.cpuid_leaf_02_descriptors[ 5] = (ebx >> 16) & 0xff;
		hw.cpuid_leaf_02_descriptors[ 6] = (ebx >>  8) & 0xff;
		hw.cpuid_leaf_02_descriptors[ 7] = (ebx      ) & 0xff;
	}
	if ((ecx & 0x80000000) == 0)
	{
		hw.cpuid_leaf_02_descriptors[ 8] = (ecx >> 24) & 0xff;
		hw.cpuid_leaf_02_descriptors[ 9] = (ecx >> 16) & 0xff;
		hw.cpuid_leaf_02_descriptors[10] = (ecx >>  8) & 0xff;
		hw.cpuid_leaf_02_descriptors[11] = (ecx      ) & 0xff;
	}
	if ((edx & 0x80000000) == 0)
	{
		hw.cpuid_leaf_02_descriptors[12] = (edx >> 24) & 0xff;
		hw.cpuid_leaf_02_descriptors[13] = (edx >> 16) & 0xff;
		hw.cpuid_leaf_02_descriptors[14] = (edx >>  8) & 0xff;
		hw.cpuid_leaf_02_descriptors[15] = (edx      ) & 0xff;
	}
	// Sort descriptors to not depend on their order.
	for (unsigned i = 0; i < ArraySize(hw.cpuid_leaf_02_descriptors); ++i)
		for (unsigned j = i + 1; j < ArraySize(hw.cpuid_leaf_02_descriptors); ++j)
			if (hw.cpuid_leaf_02_descriptors[j] > hw.cpuid_leaf_02_descriptors[i])
			{
				const auto tmp = hw.cpuid_leaf_02_descriptors[j];
				hw.cpuid_leaf_02_descriptors[j] = hw.cpuid_leaf_02_descriptors[i];
				hw.cpuid_leaf_02_descriptors[i] = tmp;
			}

	hw.used_sources.cpu = 1;
	return 0;
}

SECURE_FUNC_INTERNAL
static HWResult ReadHWInfo_Memory(HWInfo& hw)
{
	DECLARE_SECURE_SYSCALL_VARS;

	linux_sysinfo si;
	SECURE_SYSCALL(SYS_sysinfo, &si);
	if (sys_err)
	{
		PERROR("sysinfo", "", sys_err);
		return HWRESULT(HWError::syscall_sysinfo_fail, 0, sys_err);
	}
#if __has_feature(memory_sanitizer)
	__msan_unpoison(&si, sizeof(si));
#endif

	if (si.totalram == 0)
		return HWRESULT(HWError::invalid_memory_size, 0, 0);

	const uint64_t ram_size = static_cast<uint64_t>(si.totalram) * si.mem_unit;
	const uint64_t swap_size = static_cast<uint64_t>(si.totalswap) * si.mem_unit;

	// Round up to 512MiB
	constexpr uint64_t _512MiB = (1 << 29);
	const uint64_t ram_size_rounded = (ram_size + _512MiB - 1) & ~(_512MiB - 1);

	hw.memory_size = ram_size_rounded;
	hw.swap_size = swap_size;
	hw.used_sources.memory = 1;
	return 0;
}

SECURE_FUNC_INTERNAL
static HWResult ReadHWInfo_Timezone(HWInfo& hw)
{
	char buffer[sizeof(hw.tz_name)];
	size_t size = sizeof(buffer);
	const auto result = ReadFile(0, SECURE_STRING("/etc/timezone"), buffer, size);
	if (result != 0)
	{
		if (result != HWRESULT_FALSE)
			return HWRESULT_WITH_CONTEXT(result, HWContext::etc_timezone);
		return 0; // File is missing, that is ok.
	}
	assert(size <= sizeof(hw.tz_name));

	// Take the first line. This file should contain only one line anyway, this
	// is a defence against unforeseen future format changes.
	size_t len = 0;
	for ( ; len < size; ++len)
		if (buffer[len] == '\n' || buffer[len] == '\r')
			break;

	SetBuffer(hw.tz_name, buffer, len);
	hw.used_sources.timezone = 1;
	return 0;
}

// Parses value of an option from os-release file.
// Retuns pointer to the start of the value or nullptr is parsing fails.
// Advances 'p' past the end of the option value.
SECURE_FUNC_INTERNAL
static const char* ParseOSReleaseOptionValue(const char*& p, const char* p_end)
{
	if (p >= p_end)
		return nullptr;

	if (*p == '\'' || *p == '"')
	{
		const char quote = *p++;

		// Parse until next unescaped quote.
		const auto result = p;
		bool escaped = false;
		for (; p < p_end; ++p)
		{
			if (*p == quote && !escaped)
			{
				++p; // Skip the quote.
				return result;
			}
			if (*p == '\n' || *p == '\r')
				return nullptr; // Value can't span multiple lines.

			escaped = *p == '\\' && !escaped;
		}
		return nullptr;
	}
	else
	{
		// Parse until end of the line.
		const auto result = p;
		for (; p < p_end; ++p)
			if (*p == '\n' || *p == '\r')
				return result;
		return nullptr;
	}
}

SECURE_FUNC_INTERNAL
static HWResult ReadHWInfo_OSRelease(HWInfo& hw)
{
	char buffer[1024];
	size_t size = sizeof(buffer);
	auto result = ReadFile(0, SECURE_STRING("/etc/os-release"), buffer, size);
	if (result != 0)
	{
		if (result != HWRESULT_FALSE)
			return HWRESULT_WITH_CONTEXT(result, HWContext::etc_os_release);
		size = sizeof(buffer);
		result = ReadFile(0, SECURE_STRING("/usr/lib/os-release"), buffer, size);
		if (result != 0)
		{
			if (result != HWRESULT_FALSE)
				return HWRESULT_WITH_CONTEXT(result, HWContext::usr_lib_os_release);
			return 0; // Both files are missing, that is ok.
		}
	}
	hw.used_sources.os_release = 1;

	// Extract values of ID= and VERSION_ID= from the file.
	const char* p = buffer;
	const char* const p_end = buffer + size;
	while (p < p_end)
	{
		// Non-const non-static to force initialization in the code
		char id_str[] = "ID=";
		char version_id_str[] = "VERSION_ID=";

		constexpr auto id_str_len = sizeof(id_str)-1;
		constexpr auto version_id_str_len = sizeof(version_id_str)-1;

		if (p + id_str_len < p_end && memcmp(p, id_str, id_str_len) == 0)
		{
			p += id_str_len;
			if (const auto value = ParseOSReleaseOptionValue(p, p_end))
			{
				assert(p >= value);
				const size_t length = p - value;
				SetBuffer(hw.os_release_id, value, length);
				hw.used_sources.os_release_id = 1;
			}
		}
		else if (p + version_id_str_len < p_end && memcmp(p, version_id_str, version_id_str_len) == 0)
		{
			p += version_id_str_len;
			if (const auto value = ParseOSReleaseOptionValue(p, p_end))
			{
				assert(p >= value);
				const size_t length = p - value;
				SetBuffer(hw.os_release_version_id, value, length);
				hw.used_sources.os_release_version_id = 1;
			}
		}
		// Skip to the end of line.
		for (; p < p_end; ++p)
			if (*p == '\n' || *p == '\r')
				break;
		// Skip to the next non-empty line.
		for (; p < p_end; ++p)
			if (*p != '\n' && *p != '\r')
				break;
	}
	return 0;
}

SECURE_FUNC_INTERNAL
static HWResult ReadHWInfo_MachineID(HWInfo& hw)
{
	char buffer[48];
	size_t size = sizeof(buffer);
	const auto result = ReadFile(0, SECURE_STRING("/etc/machine-id"), buffer, size);
	if (result != 0)
	{
		if (result != HWRESULT_FALSE)
			return HWRESULT_WITH_CONTEXT(result, HWContext::etc_machine_id);
		return 0; // File is missing, that is ok.
	}

	// Validate contents, machine-id should be a 32 byte hex string followed by a new line (also shouldn't be all zeroes).
	// We don't want to include invalid data in the hardware key because invalid machine-id is likely to be corrected in the future.
	if (size < 32)
		return 0; // Ignore invalid machine-id
	unsigned zeros = false;
	for (unsigned i = 0; i < 32; ++i)
	{
		const char c = buffer[i];
		const bool is_digit = c >= '0' && c <= '9';
		const bool is_hex_upper = c >= 'A' && c <= 'F';
		const bool is_hex_lower = c >= 'a' && c <= 'f';
		if (!is_digit && !is_hex_lower && !is_hex_upper)
			return 0; // Ignore invalid machine-id

		zeros += c == '0';
	}
	if (zeros == 32)
		return 0; // Ignore invalid machine-id
	if (size > 32 && buffer[32] != '\r' && buffer[32] != '\n')
		return 0; // Ignore invalid machine-id

	memcpy(hw.machine_id, buffer, sizeof(hw.machine_id));
	hw.used_sources.machine_id = 1;
	return 0;
}

SECURE_FUNC_INTERNAL
static HWResult ReadHWInfo_Filesystem(HWInfo& hw)
{
	char root_str[] = "/";

	DECLARE_SECURE_SYSCALL_VARS;

	struct_statfs sfs;
	SECURE_SYSCALL(sys_statfs, STATFS_ARGS(root_str, &sfs));
	if (sys_err)
	{
		PERROR("statfs", root_str, sys_err);
		return HWRESULT(HWError::syscall_statfs_fail, 0, sys_err);
	}
#if __has_feature(memory_sanitizer)
	__msan_unpoison(&sfs, sizeof(sfs));
#endif

	if (sfs.f_type == 0)
		return HWRESULT(HWError::no_filesystem_type, 0, 0);
	hw.fs_type = sfs.f_type;

	hw.fs_id = 0; // In case f_fsid is smaller than 8 bytes.
	static_assert(sizeof(sfs.f_fsid) <= sizeof(uint64_t), "Unexpected size of f_fsid field");
	memcpy(&hw.fs_id, &sfs.f_fsid, sizeof(sfs.f_fsid));
	if (hw.fs_id != 0)
		hw.used_sources.fs_id = 1;

	return 0;
}

static HWResult ReadBlockDeviceInfo(HWInfo::Disk*& disk, const HWInfo::Disk* disk_end, int fd, int sysfs_fd, device_id_t sysfs_device_id);

// Reads identifying information for a block device that represents a hardware disk device.
// Returns HWRESULT_FALSE if the block device is not associated with a hardware device.
SECURE_FUNC_INTERNAL
static HWResult TryReadDiskDeviceInfo(HWInfo::Disk& disk, int block_fd, device_id_t sysfs_device_id)
{
	DECLARE_SECURE_SYSCALL_VARS;

	// Non-const non-static to force initialization in the code
	char device_str[] = "device";
	char size_str[] = "size";
	char wwid_str[] = "wwid";
	char vendor_str[] = "vendor";
	char model_str[] = "model";

	// Check if the block device is associated with a hardware disk device.
	SECURE_SYSCALL(SYS_openat, block_fd, device_str, O_PATH | O_DIRECTORY | O_CLOEXEC);
	if (sys_err)
	{
		if (sys_err == ENOENT)
			return HWRESULT_FALSE;
		PERROR("openat", device_str, sys_err);
		return HWRESULT(HWError::syscall_openat_fail, HWContext::blockdev_device, sys_err);
	}
	FD device_fd(sys_ret);

	auto result = VerifyDeviceID(device_fd.value, sysfs_device_id);
	if (result != 0)
		return HWRESULT_WITH_CONTEXT(result, HWContext::blockdev_device);

#if HWKEY_VERBOSE_LOGS
	fprintf(stderr, "Block device %i is a hardware disk.\n", block_fd);
#endif

	char buffer[128];
	static_assert(sizeof(buffer) >= sizeof(HWInfo::Disk::wwid), "");
	static_assert(sizeof(buffer) >= sizeof(HWInfo::Disk::vendor), "");
	static_assert(sizeof(buffer) >= sizeof(HWInfo::Disk::model), "");
	size_t size;

	// Read size of the block device
	size = sizeof(buffer);
	result = ReadFile(block_fd, size_str, buffer, size, sysfs_device_id);
	if (result != 0)
	{
		if (result != HWRESULT_FALSE)
			return HWRESULT_WITH_CONTEXT(result, HWContext::blockdev_size);
		return HWRESULT(HWError::no_disk_size_file, 0, 0);
	}

	const char* p = buffer;
	disk.size = ParseUnsigned(p, p + sizeof(buffer)) * 512;
	if (disk.size == 0)
		return HWRESULT(HWError::invalid_disk_size, 0, 0);

	// Try to read wwid of the disk device
	size = sizeof(buffer);
	result = ReadFile(device_fd.value, wwid_str, buffer, size, sysfs_device_id);
	if (result == 0)
	{
		SetBuffer(disk.wwid, buffer, size);
		disk.is_wwid = 1;

		disk.is_filled = 1;
		return 0;
	}
	else if (result != HWRESULT_FALSE)
		return HWRESULT_WITH_CONTEXT(result, HWContext::disk_wwid);

	// For NVMe devices wwid is stored in the block device node instead
	size = sizeof(buffer);
	result = ReadFile(block_fd, wwid_str, buffer, size, sysfs_device_id);
	if (result == 0)
	{
		SetBuffer(disk.wwid, buffer, size);
		disk.is_wwid = 1;

		disk.is_filled = 1;
		return 0;
	}
	else if (result != HWRESULT_FALSE)
		return HWRESULT_WITH_CONTEXT(result, HWContext::blockdev_wwid);

	// If there is no wwid (or the read on it fails, that was observed for SCSI drives) then read disk vendor and model
	size = sizeof(buffer);
	result = ReadFile(device_fd.value, vendor_str, buffer, size, sysfs_device_id);
	if (result == 0)
		SetBuffer(disk.vendor, buffer, size);
	else if (result != HWRESULT_FALSE)
		return HWRESULT_WITH_CONTEXT(result, HWContext::disk_vendor);

	size = sizeof(buffer);
	result = ReadFile(device_fd.value, model_str, buffer, size, sysfs_device_id);
	if (result == 0)
		SetBuffer(disk.model, buffer, size);
	else if (result != HWRESULT_FALSE)
		return HWRESULT_WITH_CONTEXT(result, HWContext::disk_model);

	disk.is_filled = 1;
	return 0;
}

// Reads identifying information for a block device that represents a partition on another block device.
// Returns HWRESULT_FALSE if the block device is not associated with a partition.
SECURE_FUNC_INTERNAL
static HWResult TryReadPartitionInfo(HWInfo::Disk*& disk, const HWInfo::Disk* disk_end, int block_fd, int sysfs_fd, device_id_t sysfs_device_id)
{
	DECLARE_SECURE_SYSCALL_VARS;

	// Non-const non-static to force initialization in the code
	char partition_str[] = "partition";
	char dotdot_str[] = "..";

	// If block device is a partition then the file "partition" must be present.
	struct_stat partition_st;
	SECURE_SYSCALL(sys_fstatat, block_fd, partition_str, &partition_st, AT_SYMLINK_NOFOLLOW);
	if (sys_err)
	{
		if (sys_err == ENOENT)
			return HWRESULT_FALSE;
		PERROR("fstatat64", partition_str, sys_err);
		return HWRESULT(HWError::syscall_fstatat_fail, HWContext::blockdev_partition, sys_err);
	}
#if __has_feature(memory_sanitizer)
	__msan_unpoison(&partition_st, sizeof(partition_st));
#endif

	if (!S_ISREG(partition_st.st_mode))
		return HWRESULT_FALSE;

#if HWKEY_VERBOSE_LOGS
	fprintf(stderr, "Block device %i is a partition.\n", block_fd);
#endif

	SECURE_SYSCALL(SYS_openat, block_fd, dotdot_str, O_PATH | O_DIRECTORY | O_CLOEXEC);
	if (sys_err)
	{
		PERROR("openat", dotdot_str, sys_err);
		return HWRESULT(HWError::syscall_openat_fail, HWContext::partition_parent, sys_err);
	}
	FD fd(sys_ret);

	// Note: Not calling VerifyDeviceID because we can't leave sysfs by going to the parent directory from here.

#if HWKEY_VERBOSE_LOGS
	fprintf(stderr, "Opening parent block device: %i\n", fd.value);
#endif
	return ReadBlockDeviceInfo(disk, disk_end, fd.value, sysfs_fd, sysfs_device_id);
}

// Reads identifying information for a virtual block device that consists of several "slave" block devices.
// This handles device mapper block devices (lvm2, luks, ...) and mdraid arrays.
// Returns HWRESULT_FALSE if the block device doesn't have "slave" block devices.
SECURE_FUNC_INTERNAL
static HWResult TryReadCompositeBlockDeviceInfo(HWInfo::Disk*& disk, const HWInfo::Disk* disk_end, int block_fd, int sysfs_fd, device_id_t sysfs_device_id)
{
	DECLARE_SECURE_SYSCALL_VARS;

	// Non-const non-static to force initialization in the code
	char slaves_str[] = "slaves";

	// Links to the underlying block devices are stored in the "slaves" directory.
	SECURE_SYSCALL(SYS_openat, block_fd, slaves_str, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
	if (sys_err)
	{
		if (sys_err == ENOENT)
			return HWRESULT_FALSE;
		PERROR("openat", slaves_str, sys_err);
		return HWRESULT(HWError::syscall_openat_fail, HWContext::blockdev_slaves, sys_err);
	}
	FD slaves_fd(sys_ret);

	auto result = VerifyDeviceID(slaves_fd.value, sysfs_device_id);
	if (result != 0)
		return HWRESULT_WITH_CONTEXT(result, HWContext::blockdev_slaves);

	bool found_slaves = false;
	while (true)
	{
		char dir_buffer[2048];
		SECURE_SYSCALL(SYS_getdents64, slaves_fd.value, &dir_buffer, sizeof(dir_buffer));
		if (sys_err)
		{
			PERROR("getdents64", slaves_str, sys_err);
			return HWRESULT(HWError::syscall_getdents64_fail, HWContext::blockdev_slaves, sys_err);
		}
		assert(sys_ret >= 0);
		const auto size = static_cast<size_t>(sys_ret);
#if __has_feature(memory_sanitizer)
		__msan_unpoison(dir_buffer, size);
#endif
		if (size == 0)
			break;

		const linux_dirent64* dir = nullptr;
		for (size_t offset = 0; offset < size; offset += dir->d_reclen)
		{
			dir = reinterpret_cast<const linux_dirent64*>(dir_buffer + offset);
			if (dir->d_name[0] == '.' && dir->d_name[1] == '\0')
				continue; // Skip "."
			if (dir->d_name[0] == '.' && dir->d_name[1] == '.' && dir->d_name[2] == '\0')
				continue; // Skip ".."
			if (dir->d_type != linux_DT_DIR && dir->d_type != linux_DT_LNK && dir->d_type != linux_DT_UNKNOWN)
				continue; // Skip non-directories

			SECURE_SYSCALL(SYS_openat, slaves_fd.value, dir->d_name, O_PATH | O_DIRECTORY | O_CLOEXEC);
			if (sys_err)
			{
				if (sys_err == ENOTDIR)
					continue; // Skip non-directories
				if (sys_err == ENOENT)
					continue; // Directory entry might be gone by now, that is ok.
				PERROR("openat", dir->d_name, sys_err);
				return HWRESULT(HWError::syscall_openat_fail, HWContext::blockdev_slaves_entry, sys_err);
			}
			FD fd(sys_ret);

			result = VerifyDeviceID(fd.value, sysfs_device_id);
			if (result != 0)
				return HWRESULT_WITH_CONTEXT(result, HWContext::blockdev_slaves_entry);

#if HWKEY_VERBOSE_LOGS
			if (!found_slaves)
				fprintf(stderr, "Block device %i is a dm/md composite device.\n", block_fd);
			fprintf(stderr, "Opening slave block device: %i (%s)\n", fd.value, dir->d_name);
#endif

			result = ReadBlockDeviceInfo(disk, disk_end, fd.value, sysfs_fd, sysfs_device_id);
			if (result != 0)
				return result;
			if (result != HWRESULT_FALSE)
				found_slaves = true;
		}
	}
	if (!found_slaves)
		return HWRESULT_FALSE;
	return 0;
}

SECURE_FUNC_INTERNAL
static HWResult ReadBlockDeviceInfo(HWInfo::Disk*& disk, const HWInfo::Disk* disk_end, int fd, int sysfs_fd, device_id_t sysfs_device_id)
{
	// Check that we didn't run out disk info slots.
	if (disk == disk_end)
		return HWRESULT(HWError::too_many_disks, 0, 0);

	HWResult result;
	if (HWRESULT_FALSE != (result = TryReadDiskDeviceInfo(*disk, fd, sysfs_device_id)))
	{
		if (result == 0)
			++disk;
		return result;
	}
	if (HWRESULT_FALSE != (result = TryReadPartitionInfo(disk, disk_end, fd, sysfs_fd, sysfs_device_id)))
		return result;
	if (HWRESULT_FALSE != (result = TryReadCompositeBlockDeviceInfo(disk, disk_end, fd, sysfs_fd, sysfs_device_id)))
		return result;
	return HWRESULT_FALSE;
}

// Parses one line from /proc/self/mountinfo, see 'man 5 proc' for format description.
// Optional fields (7th field) are not split, they are returned as one field.
// Modifies passed string - inserts null bytes between fields.
// Returns the number of fields.
SECURE_FUNC_INTERNAL
static unsigned ParseMountinfoRecord(char* line, const char* fields[11])
{
	unsigned n_fields = 0;

	bool in_field = false; // True when we the last character was part of the field.
	for (char* p = line; *p; ++p)
	{
		if (in_field && *p == ' ')
		{
			// End of a field.
			*p = '\0';
			in_field = false;
		}
		else if (!in_field && *p != ' ')
		{
			// Start of a field.
			if (n_fields >= 11)
				return n_fields; // There are more fields than we expected, ignore the rest.

			fields[n_fields++] = p;
			in_field = true;

			if (n_fields == 7)
			{
				// We are at the position in which optional fields may appear.
				if (p[0] == '-' && (p[1] == ' ' || !p[1]))
				{
					// There is a separator right away, so no optional fields.
					fields[6] = nullptr;
				}
				else
				{
					// Skip to the separator field: " - "
					++p; // Skip first iteration, we know that p[0] != ' '
					for ( ; *p; ++p)
						if (p[0] == ' ' && p[1] == '-' && (p[2] == ' ' || !p[2]))
							break;
					if (!*p)
						return n_fields;
					*p++ = '\0';
				}

				// Continue parsing the separator as usual to avoid duplication of logic.
				fields[7] = p;
				++n_fields;
			}
		}
	}
	return n_fields;
}

// Finds name of the block device that was mounted on the given mount point and returns id of that device in 'device_id'.
SECURE_FUNC_INTERNAL
static HWResult GetDeviceByMountpoint(const char* mount_point, device_id_t& device_id, int procfs_fd, device_id_t procfs_device_id)
{
	DECLARE_SECURE_SYSCALL_VARS;

	SECURE_SYSCALL(SYS_openat, procfs_fd, SECURE_STRING("self/mountinfo"), O_RDONLY | O_CLOEXEC);
	if (sys_err)
	{
		PERROR("openat", "/proc/self/mountinfo", sys_err);
		return HWRESULT(HWError::syscall_openat_fail, HWContext::proc_self_mountinfo, sys_err);
	}
	FD mountinfo_fd(sys_ret);

	auto result = VerifyDeviceID(mountinfo_fd.value, procfs_device_id);
	if (result != 0)
		return HWRESULT_WITH_CONTEXT(result, HWContext::proc_self_mountinfo);

	// Read /proc/self/mountinfo and find a record describing filesystem mounted on mount_point.
	// We can't use ReadFile because /proc/self/mountinfo may be arbitrarily large.
	// Instead we read the file in small chunks and parse it line by line.
	char buffer[2048];
	size_t data_size = 0;
	while (true)
	{
		assert(data_size <= sizeof(buffer));
		if (data_size >= sizeof(buffer))
			return HWRESULT(HWError::buffer_overflow, HWContext::proc_self_mountinfo, 0);

		SECURE_SYSCALL(SYS_read, mountinfo_fd.value, buffer + data_size, sizeof(buffer) - data_size);
		if (sys_err)
		{
			PERROR("read", "/proc/self/mountinfo", sys_err);
			return HWRESULT(HWError::syscall_read_fail, HWContext::proc_self_mountinfo, sys_err);
		}
		assert(sys_ret >= 0);
		const auto read_result = static_cast<size_t>(sys_ret);
#if __has_feature(memory_sanitizer)
		__msan_unpoison(buffer, read_result);
#endif
		if (read_result == 0)
			break;
		data_size += static_cast<size_t>(read_result);

		// Parse all full lines that are now in the buffer.
		auto p = buffer;
		const auto p_end = buffer + data_size;
		while (p < p_end)
		{
			auto line_start = p;

			// Skip to the end of line.
			for (; p < p_end; ++p)
				if (*p == '\n' || *p == '\r')
					break;
			if (p >= p_end)
			{
				p = line_start; // Reset to beginning of unparsed data.
				break;
			}

			*p++ = '\0';
#if HWKEY_VERBOSE_LOGS
			fprintf(stderr, "Parsing mointinfo line: '%s'\n", line_start);
#endif
			// Skip to the next non-empty line.
			for (; p < p_end; ++p)
				if (*p != '\n' && *p != '\r')
					break;

			const char* fields[11];
			const auto n_fields = ParseMountinfoRecord(line_start, fields);

			if (n_fields < 5)
				continue; // Not enough fields to determine if this is the record we want, this is ok.
			if (strcmp(fields[4], mount_point) != 0)
				continue; // This is not the record for our filesystem.
			if (n_fields < 10)
				return HWRESULT(HWError::invalid_mountinfo_format, 0, 0); // Mount source field is missing, this shouldn't happen (for filesystems we are interested in).

			const auto device_name = fields[9];
#if HWKEY_VERBOSE_LOGS
			fprintf(stderr, "Device mounted on '%s' is '%s'.\n", mount_point, device_name);
#endif

			// Get ID of the device used to mount the filesystem.
			struct_stat st;
			SECURE_SYSCALL(sys_stat, device_name, &st);
			if (sys_err)
			{
				PERROR("stat", device_name, sys_err);
				return HWRESULT(HWError::syscall_stat_fail, HWContext::mount_device, sys_err);
			}
#if __has_feature(memory_sanitizer)
			__msan_unpoison(&st, sizeof(st));
#endif

			if (!S_ISBLK(st.st_mode))
				return HWRESULT(HWError::invalid_mount_device, 0, 0);

#if HWKEY_VERBOSE_LOGS
			fprintf(stderr, "ID of device '%s' is %04xh\n", device_name, static_cast<unsigned>(st.st_rdev));
#endif
			device_id = st.st_rdev;
			return 0;
		}

		// At this point 'p' points to the start of non parsed data.
		data_size = p_end - p;
		if (data_size > 0)
			memmove(buffer, p, data_size);
	}

	return HWRESULT_FALSE;
}

SECURE_FUNC_INTERNAL
static HWResult CheckIfBTRFSHasDevice(int fs_fd, const char* device_id_str, device_id_t sysfs_device_id)
{
	DECLARE_SECURE_SYSCALL_VARS;

	// Non-const non-static to force initialization in the code
	char devices_str[] = "devices";

	SECURE_SYSCALL(SYS_openat, fs_fd, devices_str, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
	if (sys_err)
	{
		if (sys_err == ENOENT || sys_err == ENOTDIR)
			return HWRESULT_FALSE;
		PERROR("openat", devices_str, sys_err);
		return HWRESULT(HWError::syscall_openat_fail, HWContext::btrfs_devices_search, sys_err);
	}
	FD devices_fd(sys_ret);

	auto result = VerifyDeviceID(devices_fd.value, sysfs_device_id);
	if (result != 0)
		return HWRESULT_WITH_CONTEXT(result, HWContext::btrfs_devices_search);

	// Iterate "devices/" directory and try to find a link that points to a block device with device ID ("dev" file) equal to device_id_str.
	while (true)
	{
		char dir_buffer[2048];
		SECURE_SYSCALL(SYS_getdents64, devices_fd.value, &dir_buffer, sizeof(dir_buffer));
		if (sys_err)
		{
			PERROR("getdents64", devices_str, sys_err);
			return HWRESULT(HWError::syscall_getdents64_fail, HWContext::btrfs_devices_search, sys_err);
		}
		assert(sys_ret >= 0);
		const auto size = static_cast<size_t>(sys_ret);
#if __has_feature(memory_sanitizer)
		__msan_unpoison(dir_buffer, size);
#endif
		if (size == 0)
			break;

		const linux_dirent64* dir = nullptr;
		for (size_t offset = 0; offset < size; offset += dir->d_reclen)
		{
			dir = reinterpret_cast<const linux_dirent64*>(dir_buffer + offset);
			if (dir->d_name[0] == '.' && dir->d_name[1] == '\0')
				continue; // Skip "."
			if (dir->d_name[0] == '.' && dir->d_name[1] == '.' && dir->d_name[2] == '\0')
				continue; // Skip ".."
			if (dir->d_type != linux_DT_DIR && dir->d_type != linux_DT_LNK && dir->d_type != linux_DT_UNKNOWN)
				continue; // Skip non-directories

			// Construct path to file "dev" inside current directory entry.
			const auto device_name_len = strlen(dir->d_name);
			char dev_file_name[512];
			if (device_name_len + sizeof("/dev") > sizeof(dev_file_name))
				return HWRESULT(HWError::buffer_overflow, HWContext::btrfs_devices_search, 0);

			memcpy(dev_file_name, dir->d_name, device_name_len);
			dev_file_name[device_name_len+0] = '/';
			dev_file_name[device_name_len+1] = 'd';
			dev_file_name[device_name_len+2] = 'e';
			dev_file_name[device_name_len+3] = 'v';
			dev_file_name[device_name_len+4] = '\0';

			// Read and parse the "dev" file
			char dev_file_buffer[16];
			size_t dev_file_size = sizeof(dev_file_buffer) - 1; // Reserve space for null-terminator
			result = ReadFile(devices_fd.value, dev_file_name, dev_file_buffer, dev_file_size, sysfs_device_id);
			if (result != 0)
			{
				if (result != HWRESULT_FALSE)
					return HWRESULT_WITH_CONTEXT(result, HWContext::btrfs_device_dev);
				continue; // There is no "dev" file in this entry (or entry is not even a directory), this is ok.
			}

			dev_file_buffer[dev_file_size] = '\0';

			// Put null-terminator at the end of line
			for (auto p = dev_file_buffer; *p; ++p)
				if (*p == '\n' || *p == '\r')
				{
					*p = '\0';
					break;
				}

			if (strcmp(dev_file_buffer, device_id_str) == 0)
				return 0;
		}
	}
	return HWRESULT_FALSE;
}

SECURE_FUNC_INTERNAL
static HWResult ReadDisksInfo_BTRFS(HWInfo::Disk*& disk, const HWInfo::Disk* disk_end, const char* path, int sysfs_fd, int procfs_fd, device_id_t sysfs_device_id, device_id_t procfs_device_id)
{
	// TODO: Find mount point for the filesystem on which 'path' is stored.
	// This is not trivial and is not needed currently (we only call this function with path == "/").
	// Approximate algorithm would be:
	// 1. Get device id of 'path'.
	// 2. Remove last part from 'path'.
	// 3. If 'path' is a symlink then do readlink(path), if result is an absolute path then replace 'path' with the result, otherwise append result to 'path' and simplify it (remove ".." and ".").
	// 4. Get device id of 'path', if it differs from the previous one then previous value of 'path' is the mount point.
	// 5. If 'path' is not "/" go to 2.
	assert(path[0] == '/' && path[1] == '\0');
	const auto mount_point = path;

	device_id_t mount_device_id;
	auto result = GetDeviceByMountpoint(mount_point, mount_device_id, procfs_fd, procfs_device_id);
	if (result != 0)
		return result;

	char mount_device_id_str[10];
	FormatDeviceID(mount_device_id, mount_device_id_str);

	DECLARE_SECURE_SYSCALL_VARS;

	// Open sysfs node with information about all mounted btrfs filesystems
	SECURE_SYSCALL(SYS_openat, sysfs_fd, SECURE_STRING("fs/btrfs"), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
	if (sys_err)
	{
		if (sys_err == ENOENT)
			return HWRESULT_FALSE;
		PERROR("openat", "/sys/fs/btrfs", sys_err);
		return HWRESULT(HWError::syscall_openat_fail, HWContext::btrfs, sys_err);
	}
	FD btrfs_fd(sys_ret);

	result = VerifyDeviceID(btrfs_fd.value, sysfs_device_id);
	if (result != 0)
		return HWRESULT_WITH_CONTEXT(result, HWContext::btrfs);

	// Find filesystem that uses a device with device ID equal to mount_device_id.
	FD fs_fd;
	while (true)
	{
		char dir_buffer[2048];
		SECURE_SYSCALL(SYS_getdents64, btrfs_fd.value, &dir_buffer, sizeof(dir_buffer));
		if (sys_err)
		{
			PERROR("getdents64", "/sys/fs/btrfs", sys_err);
			return HWRESULT(HWError::syscall_getdents64_fail, HWContext::btrfs, sys_err);
		}
		assert(sys_ret >= 0);
		const auto size = static_cast<size_t>(sys_ret);
#if __has_feature(memory_sanitizer)
		__msan_unpoison(dir_buffer, size);
#endif
		if (size == 0)
			break;

		const linux_dirent64* dir = nullptr;
		for (size_t offset = 0; offset < size; offset += dir->d_reclen)
		{
			dir = reinterpret_cast<const linux_dirent64*>(dir_buffer + offset);
			if (dir->d_name[0] == '.' && dir->d_name[1] == '\0')
				continue; // Skip "."
			if (dir->d_name[0] == '.' && dir->d_name[1] == '.' && dir->d_name[2] == '\0')
				continue; // Skip ".."
			if (dir->d_type != linux_DT_DIR && dir->d_type != linux_DT_LNK && dir->d_type != linux_DT_UNKNOWN)
				continue; // Skip non-directories

			SECURE_SYSCALL(SYS_openat, btrfs_fd.value, dir->d_name, O_PATH | O_DIRECTORY | O_CLOEXEC);
			if (sys_err)
			{
				if (sys_err == ENOTDIR)
					continue; // Skip non-directories
				if (sys_err == ENOENT)
					continue; // Directory entry might be gone by now, that is ok.
				PERROR("openat", dir->d_name, sys_err);
				return HWRESULT(HWError::syscall_openat_fail, HWContext::btrfs_entry, sys_err);
			}
			fs_fd = FD(sys_ret);

			result = VerifyDeviceID(fs_fd.value, sysfs_device_id);
			if (result != 0)
				return HWRESULT_WITH_CONTEXT(result, HWContext::btrfs_entry);

#if HWKEY_VERBOSE_LOGS
			fprintf(stderr, "Checking btrfs instance %s\n", dir->d_name);
#endif

			result = CheckIfBTRFSHasDevice(fs_fd.value, mount_device_id_str, sysfs_device_id);
			if (result == HWRESULT_FALSE)
				continue;
			if (result == 0)
			{
#if HWKEY_VERBOSE_LOGS
				fprintf(stderr, "Our btrfs intstance is %s\n", dir->d_name);
#endif
				break;
			}
		}
	}

	if (fs_fd.value == -1)
		return HWRESULT(HWError::no_btrfs_fs, 0, 0);

	// Non-const non-static to force initialization in the code
	char devices_str[] = "devices";

	SECURE_SYSCALL(SYS_openat, fs_fd.value, devices_str, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
	if (sys_err)
	{
		PERROR("openat", devices_str, sys_err);
		return HWRESULT(HWError::syscall_openat_fail, HWContext::btrfs_devices, sys_err);
	}
	FD devices_fd(sys_ret);

	result = VerifyDeviceID(devices_fd.value, sysfs_device_id);
	if (result != 0)
		return HWRESULT_WITH_CONTEXT(result, HWContext::btrfs_devices);

	// Iterate "devices/" directory of the filesystem we have found and collect information on all its devices.
	while (true)
	{
		char dir_buffer[2048];
		SECURE_SYSCALL(SYS_getdents64, devices_fd.value, &dir_buffer, sizeof(dir_buffer));
		if (sys_err)
		{
			PERROR("getdents64", devices_str, sys_err);
			return HWRESULT(HWError::syscall_getdents64_fail, HWContext::btrfs_devices, sys_err);
		}
		assert(sys_ret >= 0);
		const auto size = static_cast<size_t>(sys_ret);
#if __has_feature(memory_sanitizer)
		__msan_unpoison(dir_buffer, size);
#endif
		if (size == 0)
			break;

		const linux_dirent64* dir = nullptr;
		for (size_t offset = 0; offset < size; offset += dir->d_reclen)
		{
			dir = reinterpret_cast<const linux_dirent64*>(dir_buffer + offset);
			if (dir->d_name[0] == '.' && dir->d_name[1] == '\0')
				continue; // Skip "."
			if (dir->d_name[0] == '.' && dir->d_name[1] == '.' && dir->d_name[2] == '\0')
				continue; // Skip ".."
			if (dir->d_type != linux_DT_DIR && dir->d_type != linux_DT_LNK && dir->d_type != linux_DT_UNKNOWN)
				continue; // Skip non-directories

			SECURE_SYSCALL(SYS_openat, devices_fd.value, dir->d_name, O_PATH | O_DIRECTORY | O_CLOEXEC);
			if (sys_err)
			{
				if (sys_err == ENOTDIR)
					continue; // Skip non-directories
				if (sys_err == ENOENT)
					continue; // Directory entry might be gone by now, that is ok.
				PERROR("openat", dir->d_name, sys_err);
				return HWRESULT(HWError::syscall_openat_fail, HWContext::btrfs_devices_entry, sys_err);
			}
			FD fd(sys_ret);

			result = VerifyDeviceID(fd.value, sysfs_device_id);
			if (result != 0)
				return HWRESULT_WITH_CONTEXT(result, HWContext::btrfs_devices_entry);

#if HWKEY_VERBOSE_LOGS
			fprintf(stderr, "Opening btrfs block device: %i (%s)\n", fd.value, dir->d_name);
#endif
			result = ReadBlockDeviceInfo(disk, disk_end, fd.value, sysfs_fd, sysfs_device_id);
			if (result != 0 && result != HWRESULT_FALSE)
				return result;
		}
	}

	return 0;
}

SECURE_FUNC_INTERNAL
static HWResult ReadDisksInfo_Default(HWInfo::Disk*& disk, const HWInfo::Disk* disk_end, const char* path, int sysfs_fd, device_id_t sysfs_device_id)
{
	DECLARE_SECURE_SYSCALL_VARS;

	// Get device ID of the path.
	struct_stat path_st;
	SECURE_SYSCALL(sys_stat, path, &path_st);
	if (sys_err)
	{
		PERROR("stat", path, sys_err);
		return HWRESULT(HWError::syscall_stat_fail, HWContext::root, sys_err);
	}
#if __has_feature(memory_sanitizer)
	__msan_unpoison(&path_st, sizeof(path_st));
#endif
#if HWKEY_VERBOSE_LOGS
	fprintf(stderr, "Path '%s' is on device %04xh\n", path, static_cast<unsigned>(path_st.st_dev));
#endif

	// Format device ID into the form used in sysfs dev/block/ node.
	char dev_node_path[] = "dev/block/xxxxx:xxx";
	FormatDeviceID(path_st.st_dev, dev_node_path + sizeof("dev/block"));
#if HWKEY_VERBOSE_LOGS
	fprintf(stderr, "Will start at /sys/%s\n", dev_node_path);
#endif

	// Open sysfs node for the block device which contains our filesystem.
	SECURE_SYSCALL(SYS_openat, sysfs_fd, dev_node_path, O_PATH | O_DIRECTORY | O_CLOEXEC);
	if (sys_err)
	{
		if (sys_err == ENOENT)
			return 0;
		PERROR("openat", dev_node_path, sys_err);
		return HWRESULT(HWError::syscall_openat_fail, HWContext::sysfs_device_id_link, sys_err);
	}
	FD fd(sys_ret);

	auto result = VerifyDeviceID(fd.value, sysfs_device_id);
	if (result != 0)
		return HWRESULT_WITH_CONTEXT(result, HWContext::sysfs_device_id_link);

#if HWKEY_VERBOSE_LOGS
	fprintf(stderr, "Opening root block device: %i (/sys/%s)\n", fd.value, dev_node_path);
#endif

	result = ReadBlockDeviceInfo(disk, disk_end, fd.value, sysfs_fd, sysfs_device_id);
	if (result != 0 && result != HWRESULT_FALSE)
		return result;
	return 0;
}

SECURE_FUNC_INTERNAL
static HWResult ReadHWInfo_Disks(HWInfo& hw)
{
	DECLARE_SECURE_SYSCALL_VARS;

	// Non-const non-static to force initialization in the code
	char root_str[] = "/";

	// Get type of the filesystem.
	struct_statfs sfs;
	SECURE_SYSCALL(sys_statfs, STATFS_ARGS(root_str, &sfs));
	if (sys_err)
	{
		PERROR("statfs", root_str, sys_err);
		return HWRESULT(HWError::syscall_statfs_fail, HWContext::root, sys_err);
	}
#if __has_feature(memory_sanitizer)
	__msan_unpoison(&sfs, sizeof(sfs));
#endif

	HWResult result;

	// Open special filesytems, they are needed for the following steps.
	// We are not using SECURE_STRING here because we check the fs type and thus we are not susceptible to mount point forgery.
	FD sysfs_fd;
	device_id_t sysfs_device_id = 0;
	result = OpenSpecialFS("/sys", SYSFS_MAGIC, sysfs_fd, sysfs_device_id);
	if (result != 0)
		return HWRESULT_WITH_CONTEXT(result, HWContext::sysfs);

	FD procfs_fd;
	device_id_t procfs_device_id = 0;
	result = OpenSpecialFS("/proc", PROC_SUPER_MAGIC, procfs_fd, procfs_device_id);
	if (result != 0)
		return HWRESULT_WITH_CONTEXT(result, HWContext::procfs);

	// Don't save disk info directly into HWInfo.
	// Instead reserve space on the stack for 64 disk info records, read the data there and then save 8 "largest" disks.
	// This allows to get info from same disks and in the same order even if there are more than 8 disk in the root filesytem.
	// Hopefully nobody has more than 64 disks in the root filesystem...
	HWInfo::Disk disks[64];
	memset(disks, 0, sizeof(disks));
	auto p_disk = disks;
	const auto p_disk_end = p_disk + ArraySize(disks);

	if (sfs.f_type == BTRFS_SUPER_MAGIC)
		result = ReadDisksInfo_BTRFS(p_disk, p_disk_end, root_str, sysfs_fd.value, procfs_fd.value, sysfs_device_id, procfs_device_id);
	else
		result = ReadDisksInfo_Default(p_disk, p_disk_end, root_str, sysfs_fd.value, sysfs_device_id);
	assert(result != HWRESULT_FALSE && "This value shouldn't be returned from ReadHWInfo_Disks functions");
	if (result != 0)
		return result;

	const unsigned n_disks = p_disk - disks;

	// Copy disk info records into HWInfo::disk while sorting them.
	for (unsigned i = 0; i < ArraySize(hw.disk); ++i)
	{
		unsigned max_j = 0; // Index of the next disk info to copy.
		for (unsigned j = 1; j < n_disks; ++j)
		{
			if (!disks[j].is_filled)
				continue; // Skip already copied disk info.

			if (!disks[max_j].is_filled)
			{
				max_j = j; // If we have no maximum yet then use this disk info.
				continue;
			}

			auto diff = memcmp(disks[j].wwid, disks[max_j].wwid, sizeof(disks[j].wwid));
			if (diff != 0)
			{
				max_j = diff < 0 ? j : max_j;
				continue;
			}
		}

		if (!disks[max_j].is_filled)
		{
			assert(i == n_disks);
			break; // We ran out of disk info records to copy.
		}

		hw.disk[i] = disks[max_j];
		disks[max_j].is_filled = 0; // Mark disk info as not filled to skip it on the next iteration.

		hw.used_sources.n_disks++;
		if (hw.disk[i].is_wwid)
			hw.used_sources.n_disk_wwids++;
	}

	return 0;
}

SECURE_FUNC
uint32_t ReadHWInfo(HWInfo& hw)
{
#if defined(ENABLE_VMPROTECT_BUILD)
	if (VMProtectIsDebuggerPresent(true))
		return HWRESULT_ENCRYPT(HWRESULT(HWError::debugger_found, 0, 0));
#endif

	uint32_t result;
	if (0 != (result = ReadHWInfo_CPU(hw)))
		return HWRESULT_ENCRYPT(result);
	if (0 != (result = ReadHWInfo_Memory(hw)))
		return HWRESULT_ENCRYPT(result);
	if (0 != (result = ReadHWInfo_Timezone(hw)))
		return HWRESULT_ENCRYPT(result);
	if (0 != (result = ReadHWInfo_OSRelease(hw)))
		return HWRESULT_ENCRYPT(result);
	if (0 != (result = ReadHWInfo_MachineID(hw)))
		return HWRESULT_ENCRYPT(result);
	if (0 != (result = ReadHWInfo_Filesystem(hw)))
		return HWRESULT_ENCRYPT(result);
	if (0 != (result = ReadHWInfo_Disks(hw)))
		return HWRESULT_ENCRYPT(result);
	hw.used_sources.checksum = CalculateHWInfoSourcesChecksum(hw.used_sources);
	return 0;
}

// These constants are used to encrypt/decrypt HWInfoSourceFlags in hardware key value.
static const uint32_t c_hwsf_xor_mask = 0xda7c439a;
static const uint32_t c_hwsf_factor = 15195235u;
static const uint32_t c_hwsf_factor_inv = 202474827u;
static_assert(c_hwsf_factor % 2 == 1, "c_hwsf_factor must be odd");
static_assert(c_hwsf_factor * c_hwsf_factor_inv == 1, "c_hwsf_factor_inv should be a modular multiplicative inverse of c_hwsf_factor");

SECURE_FUNC
void HWInfoToKey(const HWInfo& hw, char hw_key[HWKeyLength + 1])
{
	SHA1 sha;
	sha.Update(&hw, sizeof(HWInfo) - sizeof(HWInfoSourceFlags));
	sha.Final();
	sha.GetString(hw_key);

	// Encrypt HWInfoSourceFlags
	const uint32_t x = (hw.used_sources.raw_value ^ c_hwsf_xor_mask) * c_hwsf_factor;

	// Encode the encrypted value in hex and append to hardware key.
	// We use non-const non-static array to force its initialization in the code.
	// This is less efficient but we need to avoid global static data because it is not protected unlike the code.
	char hex_char[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	hw_key[40] = hex_char[(x >> 28) & 0x0f];
	hw_key[41] = hex_char[(x >> 24) & 0x0f];
	hw_key[42] = hex_char[(x >> 20) & 0x0f];
	hw_key[43] = hex_char[(x >> 16) & 0x0f];
	hw_key[44] = hex_char[(x >> 12) & 0x0f];
	hw_key[45] = hex_char[(x >>  8) & 0x0f];
	hw_key[46] = hex_char[(x >>  4) & 0x0f];
	hw_key[47] = hex_char[(x      ) & 0x0f];
	hw_key[48] = '\0';
}

SECURE_FUNC
HWInfoSourceFlags GetHWInfoSources(char hw_key[HWKeyLength])
{
	uint32_t x = 0;

	for (unsigned off = 40; off < 48; ++off)
	{
		const char c = hw_key[off];
		if (c >= '0' && c <= '9')
			x = (x << 4) | (c - '0');
		else if (c >= 'A' && c <= 'F')
			x = (x << 4) | (c - 'A' + 10);
		else if (c >= 'a' && c <= 'f')
			x = (x << 4) | (c - 'a' + 10);
	}

	HWInfoSourceFlags result;
	result.raw_value = x * c_hwsf_factor_inv ^ c_hwsf_xor_mask;
	return result;
}

SECURE_FUNC
unsigned CalculateHWInfoSourcesChecksum(HWInfoSourceFlags hw_src)
{
	// We use CRC-4-ITU (polynomial: x^4 + x^1 + x^0, no initial/final xor).
	// We calculate it over lower 28 bits of HWInfoSourceFlags (all non-checksum bits).

	uint32_t x = hw_src.raw_value << 4; // Make room for the result (and mask out checksum bits in the process).
	uint32_t mask = 0x13 << (32-5); // CRC polynomial left alligned.
	while (x & 0xfffffff0)
	{
		if ((x ^ mask) < x) // Check if MSB is the same in x and mask.
			x ^= mask;
		mask >>= 1;
	}

	// CRC-4 value is in 4 lower bits of x.
	return x & 0xf;
}

#undef PERROR
#undef STATFS_ARGS
#undef FSTATFS_ARGS

#endif

#undef HWRESULT
#undef HWRESULT_WITH_CONTEXT
#undef HWRESULT_FALSE
#undef HWRESULT_ENCRYPT
#undef HWRESULT_DECRYPT

}
