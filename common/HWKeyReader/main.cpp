#include "ProtectionLib/HardwareKey.h"
#include "ProtectionLib/Lib.h"

#include <inttypes.h>
#include <stdio.h>

static const char* format_binary(uint32_t x)
{
	constexpr unsigned n_bits = sizeof(x) * 8;

	thread_local char buffer[n_bits + 1];
	for (unsigned i = 0; i < n_bits; ++i)
		buffer[i] = '0' + ((x >> (n_bits-1 - i)) & 1);
	buffer[n_bits] = '\0';
	return buffer;
}

int main()
{
	protection::HWInfo hw;
	protection::memset(&hw, 0, sizeof(hw));
	const auto result = protection::ReadHWInfo(hw);
	printf("ReadHWInfo: %u\n", static_cast<unsigned>(result));

#if defined(_WIN32)

	printf("System drive VSN: %08" PRIx32 "\n", hw.system_drive_vsn);
	printf("Windows product ID: %s\n", hw.windows_product_id);
	printf("Windows install date: %" PRIu32 "\n", hw.windows_install_date);
	printf("Armadillo key: %08" PRIx32 "\n", hw.arm_hwkey);

#elif defined(__linux__)

	printf("Sources:");
#	define print_source(name) if (hw.used_sources.name) printf(" " #name)
	print_source(cpu);
	print_source(hypervisor);
	print_source(memory);
	print_source(timezone);
	print_source(os_release);
	print_source(os_release_id);
	print_source(os_release_version_id);
	print_source(machine_id);
	print_source(fs_id);
#	undef print_source
	if (hw.used_sources.n_disks > 0) printf(" disks(%u)", hw.used_sources.n_disks);
	if (hw.used_sources.n_disk_wwids > 0) printf(" disk_wwids(%u)", hw.used_sources.n_disk_wwids);
	printf(" checksum(0x%x)\n", hw.used_sources.checksum);

#	define print_str(prefix, var) printf(prefix ": %.*s\n", static_cast<int>(sizeof(var)), var)
	puts("CPU:");
	printf("\tfamily: %02xh\n", hw.cpu_family);
	printf("\tmodel: %02xh\n", hw.cpu_model);
	printf("\tstepping: %02xh\n", hw.cpu_stepping);
	printf("\tcores: %u\n", hw.cpu_n_cores);
	print_str("\tbrand", hw.cpu_brand_string);
	puts("\tfeature flags:");
	printf("\t\tcpuid[01][ecx]: %s\n", format_binary(hw.cpuid_leaf_01_ecx));
	printf("\t\tcpuid[01][edx]: %s\n", format_binary(hw.cpuid_leaf_01_edx));
	printf("\t\tcpuid[07:0][ebx]: %s\n", format_binary(hw.cpuid_leaf_07_0_ebx));
	printf("\t\tcpuid[07:0][ecx]: %s\n", format_binary(hw.cpuid_leaf_07_0_ecx));
	printf("\t\tcpuid[07:0][edx]: %s\n", format_binary(hw.cpuid_leaf_07_0_edx));
	printf("\t\tcpuid[80000001][ecx]: %s\n", format_binary(hw.cpuid_leaf_80000001_ecx));
	printf("\t\tcpuid[80000001][edx]: %s\n", format_binary(hw.cpuid_leaf_80000001_edx));
	printf("\tIntel TLB/cache/prefetch descriptors:");
	for (const unsigned x : hw.cpuid_leaf_02_descriptors)
	{
		if (x == 0)
			break;
		printf(" %02x", x);
	}
	puts("");

	print_str("Hypervisor ID", hw.hypervisor_id);

	printf("Memory: %" PRIu64 " (%u MiB)\n", hw.memory_size, static_cast<unsigned>(hw.memory_size >> 20));
	printf("Swap: %" PRIu64 " (%u MiB)\n", hw.swap_size, static_cast<unsigned>(hw.swap_size >> 20));

	print_str("Timezeone", hw.tz_name);

	print_str("OS ID", hw.os_release_id);
	print_str("OS version ID", hw.os_release_version_id);

	print_str("Machine ID", hw.machine_id);

	printf("FS type magic: 0x%08" PRIx32 "\n", hw.fs_type);
	printf("FS ID: 0x%016" PRIx64 "\n", hw.fs_id);

	for (unsigned i = 0; i < sizeof(hw.disk)/sizeof(hw.disk[0]); ++i)
	{
		if (!hw.disk[i].is_filled)
			continue;

		printf("Disk #%u:\n", i);
		printf("\tsize: %" PRIu64 "\n", hw.disk[i].size);
		if (hw.disk[i].is_wwid)
			print_str("\twwid", hw.disk[i].wwid);
		else
		{
			print_str("\tvendor", hw.disk[i].vendor);
			print_str("\tmodel", hw.disk[i].model);
		}
	}

#endif

	char hw_key[protection::HWKeyLength + 1];
	protection::HWInfoToKey(hw, hw_key);
	printf("\nHardware key: %s\n", hw_key);

	return result == 0 ? 0 : 1;
}
