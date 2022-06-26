#include "sysinfo.h"
#include <sstream>
#include <iostream>

#include <cstring>

#include <windows.h>

// Getting system information is really tricky.
// Links about gathering OS info:
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724833(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/dn481241(v=vs.85).aspx
// https://www.medo64.com/2013/10/xp-compatible-manifest/

std::string GetCPUArchitectureString(void)
{
	SYSTEM_INFO sysinfo;
	std::stringstream out;
	memset(&sysinfo, 0, sizeof(sysinfo));
	GetNativeSystemInfo((LPSYSTEM_INFO)&sysinfo);

	switch (sysinfo.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_INTEL:
		out << "x86";
		break;
	case PROCESSOR_ARCHITECTURE_AMD64:
		out << "x64";
		break;
	case PROCESSOR_ARCHITECTURE_ARM:
		out << "ARM";
		break;
	case PROCESSOR_ARCHITECTURE_IA64:
		out << "IA64";
		break;
	default:
		out << "<Unknown: " << sysinfo.wProcessorArchitecture << ">";
		break;
	}

	return out.str();
}

std::string GetOSInfoString(void)
{
	OSVERSIONINFOEXA info;
	std::string result;
	std::stringstream out;

	memset(&info, 0, sizeof(info));
	info.dwOSVersionInfoSize = sizeof(info);

	if (!GetVersionExA((LPOSVERSIONINFOA)&info))
		return "";

	out << "Windows NT " << info.dwMajorVersion << "." << info.dwMinorVersion << " " <<
		(info.wProductType == VER_NT_WORKSTATION ? "Workstation" : "Server") << " ";

	/*if (info.wServicePackMajor != 0 || info.wServicePackMinor != 0)
	{
		out << "SP " << info.wServicePackMajor << "." << info.wServicePackMinor << " ";
	}*/
	if (*info.szCSDVersion)
		out << info.szCSDVersion << " ";

	out << "(Build " << info.dwBuildNumber << ")";
	out << " for " << GetCPUArchitectureString();

	result = out.str();

	return result;
}

std::string GetFriendlyOSNameString(void)
{
	SYSTEM_INFO sysinfo;
	OSVERSIONINFOEXA osinfo;
	std::stringstream out;

	memset(&osinfo, 0, sizeof(osinfo));
	osinfo.dwOSVersionInfoSize = sizeof(osinfo);

	if (!GetVersionExA((LPOSVERSIONINFOA)&osinfo))
		return "";

	memset(&sysinfo, 0, sizeof(sysinfo));
	GetNativeSystemInfo((LPSYSTEM_INFO)&sysinfo);

	out << "Windows ";
	switch (osinfo.dwMajorVersion)
	{
	case 10:

		if (osinfo.dwMinorVersion == 0)
		{
			if (osinfo.wProductType == VER_NT_WORKSTATION)
			{
				out << "10";
			}
			else
			{
				out << "Server 2016";
			}
		}
		else
		{
			return GetOSInfoString(); // unknown Windows 10 flavour
		}

		break;
	case 6:
		switch (osinfo.dwMinorVersion)
		{
		case 3:
			if (osinfo.wProductType == VER_NT_WORKSTATION)
			{
				out << "8.1";
			}
			else
			{
				out << "Server 2012 R2";
			}

			break;
		case 2:
			if (osinfo.wProductType == VER_NT_WORKSTATION)
			{
				out << "8";
			}
			else
			{
				out << "Server 2012";
			}

			break;
		case 1:
			if (osinfo.wProductType == VER_NT_WORKSTATION)
			{
				out << "7";
			}
			else
			{
				out << "Server 2008 R2";
			}
			break;
		case 0:
			if (osinfo.wProductType == VER_NT_WORKSTATION)
			{
				out << "Vista";
			}
			else
			{
				out << "Server 2008";
			}

			break;
		}
		break;
	case 5:
		switch (osinfo.dwMinorVersion)
		{
		case 2:
			if (GetSystemMetrics(SM_SERVERR2) != 0)
			{
				out << "Server 2003 R2";
			}
			else if (osinfo.wSuiteMask & VER_SUITE_WH_SERVER)
			{
				out << "Home Server";
			}
			else if (GetSystemMetrics(SM_SERVERR2) == 0)
			{
				out << "Server 2003";
			}
			else if ((osinfo.wProductType == VER_NT_WORKSTATION) && (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64))
			{
				out << "Windows XP x64 Edition";
			}
			break;
		case 1:
			out << "XP";
			break;
		case 0:
			out << "2000";
			break;
		}
		break;
	default:
	{
		return GetOSInfoString();
	}
	break;
	}

	if (*osinfo.szCSDVersion)
	{
		out << " " << osinfo.szCSDVersion;
	}

	return out.str();
}


std::string GetCPUInfoString(unsigned int cpu_number)
{
	HKEY rkey = nullptr;
	std::stringstream reg_path;
	char cpu_info[1024] = { 0 };
	DWORD type = REG_NONE;
	DWORD data_size = sizeof(cpu_info);

	reg_path << "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\" << cpu_number;
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, reg_path.str().c_str(), NULL, KEY_READ, &rkey) != ERROR_SUCCESS)
		return "";

	if (RegQueryValueExA(rkey, "ProcessorNameString", nullptr, &type, (LPBYTE)&cpu_info[0], &data_size) != ERROR_SUCCESS || type != REG_SZ)
	{
		RegCloseKey(rkey);
		return "";
	}

	return &cpu_info[0];
}

unsigned int GetCPUsCount(unsigned int *active_processors)
{
	SYSTEM_INFO sysinfo;

	memset(&sysinfo, 0, sizeof(sysinfo));
	GetNativeSystemInfo(&sysinfo);

	if (active_processors != nullptr)
	{
		unsigned int count = 0, i = 0;
		auto mask = sysinfo.dwActiveProcessorMask;

		while (i < sizeof(mask) * 8)
		{
			if (mask & (1 << i))
			{
				count++;
			}
			i++;
		}

		*active_processors = count;
	}

	return sysinfo.dwNumberOfProcessors;
}


bool GetPhysicalMemoryInfo(uint64_t &total_bytes, uint64_t &available_bytes, unsigned int &memory_load)
{
	MEMORYSTATUSEX mstat;
	memset(&mstat, 0, sizeof(mstat));

	mstat.dwLength = sizeof(mstat);
	if (!GlobalMemoryStatusEx(&mstat))
	{
		return false;
	}

	total_bytes = mstat.ullTotalPhys;
	available_bytes = mstat.ullAvailPhys;
	memory_load = mstat.dwMemoryLoad;

	return true;
}

std::string GetComputerNameString(void)
{
	char buf[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
	DWORD size = sizeof(buf);
	if (!GetComputerNameA(buf, &size))
	{
		return "";
	}

	return &buf[0];
}
