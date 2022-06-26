/**
 **************************************************************************
 * \file VS_Util.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief ancillary functions, implementation
 *
 * \b Project Standart Libraries
 * \author SMirnovK
 * \date 28.11.06
 *
 * $Revision: 23 $
 *
 * $History: VS_Utils.cpp $
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 5.07.12    Time: 13:53
 * Updated in $/VSNA/std/cpplib
 * - projects depens repaired
 *
 * *****************  Version 22  *****************
 * User: Mushakov     Date: 13.07.11   Time: 17:35
 * Updated in $/VSNA/std/cpplib
 * - Roaming
 *
 * *****************  Version 21  *****************
 * User: Mushakov     Date: 29.03.11   Time: 17:31
 * Updated in $/VSNA/std/cpplib
 *  - rebranding
 *  - arm_hw_key added to Server hw key
 *
 * *****************  Version 20  *****************
 * User: Ktrushnikov  Date: 28.01.11   Time: 22:25
 * Updated in $/VSNA/std/cpplib
 * VCS3.2
 * - VS_PresenceService::NetworkResolve() support for VCS (using
 * confRestrick with AS)
 * - common func VS_GetServerFromCallID() to VS_Utils
 *
 * *****************  Version 19  *****************
 * User: Mushakov     Date: 20.01.11   Time: 18:16
 * Updated in $/VSNA/std/cpplib
 *  - Subscriptions via vcs servers supported
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 12.08.09   Time: 16:54
 * Updated in $/VSNA/std/cpplib
 * - old hardware key for client
 *
 * *****************  Version 17  *****************
 * User: Mushakov     Date: 7.08.09    Time: 15:14
 * Updated in $/VSNA/std/cpplib
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 21.04.08   Time: 20:34
 * Updated in $/VSNA/std/cpplib
 * - new Application Id algorithm
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 17.03.08   Time: 15:11
 * Updated in $/VSNA/std/cpplib
 * - network settings for endpoints with broker format name
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 26.02.08   Time: 16:54
 * Updated in $/VSNA/std/cpplib
 * - new servers coonect shceme
 *
 * *****************  Version 13  *****************
 * User: Ktrushnikov  Date: 22.02.08   Time: 12:02
 * Updated in $/VSNA/std/cpplib
 * - GetUserNameA() added to support Win98
 * - func renamed
 *
 * *****************  Version 12  *****************
 * User: Ktrushnikov  Date: 22.02.08   Time: 11:41
 * Updated in $/VSNA/std/cpplib
 * - Use only GetUserNameExA()
 *
 * *****************  Version 11  *****************
 * User: Ktrushnikov  Date: 22.02.08   Time: 10:12
 * Updated in $/VSNA/std/cpplib
 * - use GetUserNameExA() instead of GerUserNameA()
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 20.02.08   Time: 19:34
 * Updated in $/VSNA/std/cpplib
 * - hardware key
 *
 * *****************  Version 9  *****************
 * User: Ktrushnikov  Date: 20.02.08   Time: 16:34
 * Updated in $/VSNA/std/cpplib
 * - new func added: VS_Utils::VS_GetServerType()
 * - SM:ManagerService:DDNS_UPDATE add only AS servers
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 31.01.08   Time: 22:08
 * Updated in $/VSNA/std/cpplib
 * - bugs
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 31.01.08   Time: 19:04
 * Updated in $/VSNA/std/cpplib
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 30.01.08   Time: 19:26
 * Updated in $/VSNA/std/cpplib
 * - VS_Utils: VS_GetUniqueID_LoginComputer() func added
 * - test for above func added at _TestStandardLibraries
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 16.01.08   Time: 15:15
 * Updated in $/VSNA/std/cpplib
 * - network settings repaired
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 29.11.07   Time: 18:42
 * Updated in $/VSNA/std/cpplib
 * - registry moved to NA in client
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 22.11.07   Time: 17:26
 * Updated in $/VSNA/std/cpplib
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 22.11.07   Time: 16:41
 * Updated in $/VSNA/std/cpplib
 * - repare client
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 5.02.07    Time: 21:40
 * Updated in $/VS2005/std/cpplib
 * - project configuration
 * - depricated functions warnings suppressed
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 28.11.06   Time: 16:44
 * Created in $/VS/std/cpplib
 * - more complex random generation functions
 *
 ****************************************************************************/
/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VS_Utils.h"
#include "VS_Protocol.h"
#include "VS_RegistryConst.h"
#include "VS_RegistryKey.h"
#include "VS_VideoLevelCaps.h"
#include "std/cpplib/md5.h"
#include "../cpplib/VS_WideStr.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/VS_Replace.h"
#include "std-generic/cpplib/utf8.h"

#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/split.hpp>

#include "VS_Cpu.h"
#if defined(_WIN32) // Not ported yet
#include "diskid.h"

#include <Windows.h>
#include <process.h>
#else
#include <fstream>
#include <sys/utsname.h>
#endif

#include <boost/regex.hpp>

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <thread>
#include <iomanip>
#include <boost/algorithm/string/predicate.hpp>

#if defined(_WIN32)
#	include <Windows.h>
#	include <io.h>
#	define close _close
#	define dup _dup
#	define dup2 _dup2
#	define fileno _fileno
#	define DEV_NULL "NUL"
#else
#	include <sys/time.h>
#	include <unistd.h>
#	define DEV_NULL "/dev/null"
#endif

#if defined(_WIN32)
#	include <Windows.h>
#elif _POSIX_VERSION >= 200112L
#	include <sys/time.h>
#endif

#if defined(_WIN32)
#	include <process.h>
#	define getpid _getpid
#elif _POSIX_VERSION >= 200112L
#	include <sys/types.h>
#	include <unistd.h>
#endif

// Registry functions
#if defined(_WIN32)
#	include <Windows.h>
#endif

// For GetExecutablePath
#if defined(_WIN32)
#	include <Windows.h>
#elif defined(__linux__)
#	include <limits.h>
#	include <unistd.h>
#endif

#if defined(_WIN32) // Not ported yet

bool VS_GetDiskSerialNumber(char* id, unsigned int size);
bool VS_GetUserName(char* id, unsigned long id_sz);
bool VS_GetWinId(char* id, unsigned long size);

/**
 **************************************************************************
 ****************************************************************************/
bool VS_ReadDiskandOsKeys(VS_SimpleStr &key)
{
	const unsigned long size = 1024;
	char buff[size] = {0};
	if (VS_GetDiskSerialNumber(buff, size))
		key+= buff;
	key+="\n";
	if (VS_GetDiskString(buff, size))
		key+= buff;
	key+="\n";
	if (VS_GetWinId(buff, size))
		key+= buff;
	return true;
}



bool VS_ReadClientHardwareKey(VS_SimpleStr &key)
{
	const unsigned long size = 1024;
	char buff[size] = {0};
	const char* AppId = "AppId";
	VS_RegistryKey rkey(true, "", false, true);
	if (rkey.GetValue(buff, size, VS_REG_STRING_VT, AppId) !=33) {
		VS_GenKeyByMD5(buff);
		rkey.SetString(buff, AppId);
	}
	MD5 md5;
	// init by AppId
	md5.Update(buff);
	// read disk Serial number
	if (VS_GetDiskSerialNumber(buff, size))
		md5.Update(buff);
	// raad application registry root
	const auto reg_root = VS_RegistryKey::GetDefaultRoot();
	md5.Update(reg_root);
	// read user
	if (VS_GetUserName(buff, size))
		md5.Update(buff);
	md5.Final();
	md5.GetString(buff);
	key = buff;
	return true;
}
#endif

std::array<unsigned char, 32> VS_GetPersistentKey()
{
#ifdef _WIN32
	MD5 md5;

	char directotyBuffer[1024];
	if (GetWindowsDirectory(directotyBuffer, 512)) {
		char drive[_MAX_FNAME];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		_splitpath( directotyBuffer, drive, dir, fname, ext);
		strcat(drive, "\\");

		DWORD VolumeSerialNumber = 0;		// volume serial number
		DWORD MaximumComponentLength;		// maximum file name length
		DWORD FileSystemFlags;				// file system options
		GetVolumeInformation(drive, 0, 0, &VolumeSerialNumber, &MaximumComponentLength, &FileSystemFlags, 0, 0);
		if (VolumeSerialNumber) {
			_itoa(VolumeSerialNumber, fname, 10);
			md5.Update(fname);
		}
	}
	HKEY win_key = 0;
	RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ|KEY_WOW64_64KEY, &win_key);
	void *buf(0);
	DWORD regSize = 0;
	RegQueryValueExA((HKEY)win_key, "ProductId", 0, 0, 0, &regSize);
	if (regSize)
	{
		buf = malloc(regSize);
		if (RegQueryValueExA((HKEY)win_key, "ProductId", 0, 0, (LPBYTE)buf, &regSize) == ERROR_SUCCESS)
		{
			md5.Update(static_cast<const char*>(buf));
			free(buf);
			buf = 0;
		}
	}
	regSize = 0;
	RegQueryValueExA((HKEY)win_key, "InstallDate", 0, 0, 0, &regSize);
	if (regSize)
	{
		buf = malloc(regSize);
		if (RegQueryValueExA((HKEY)win_key, "InstallDate", 0, 0, (LPBYTE)buf, &regSize) == ERROR_SUCCESS)
		{
			char install_date[256];
			_itoa(*(int*)buf, install_date, 10);
			md5.Update(install_date);
			free(buf);
		}
	}
	RegCloseKey(win_key);

	md5.Update("0"); // Previously result of VS_ArmReadHardwareKey()
	md5.Final();
	std::array<unsigned char, 32> result;
	md5.GetBytes(result.data());
	return result;
#else
	return { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	         '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', };
#endif // _WIN32
}

union VS_RandValues
{
	struct Params {
		unsigned long cnt;
		std::chrono::steady_clock::rep time_tick;
		size_t tid_hash;
		long pid;
		int rand;
	} params;
	unsigned char raw[64];
};

/**
**************************************************************************
****************************************************************************/
void VS_GenKeyByMD5(unsigned char key[16])
{
	static std::atomic<unsigned long> counter(0);

	VS_RandValues rndVal;
	memset(&rndVal, 0, sizeof(VS_RandValues));
	rndVal.params.cnt = counter.fetch_add(137);
	rndVal.params.time_tick = std::chrono::steady_clock::now().time_since_epoch().count();
	rndVal.params.tid_hash = std::hash<std::thread::id>()(std::this_thread::get_id());
	rndVal.params.pid = getpid();
	rndVal.params.rand = rand();

	MD5 md5;
	md5.Update(&rndVal, sizeof(VS_RandValues));
	md5.Final();
	md5.GetBytes(key);
}

/**
**************************************************************************
****************************************************************************/
void VS_GenKeyByMD5(char *key)
{
	unsigned char ukey[16];
	VS_GenKeyByMD5(ukey);
	VS_MD5ToString(ukey, key);
}

unsigned long long VS_GetTimeOfDayMs()
{
#if defined(_WIN32)
	const unsigned long long epoch = 116444736000000000ull;
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	ULARGE_INTEGER tmp;
	tmp.LowPart = ft.dwLowDateTime;
	tmp.HighPart = ft.dwHighDateTime;
	return (tmp.QuadPart - epoch) / 10000;
#else
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

bool VS_IsBrokerFormat(string_view ep)
{
	auto n = ep.find_last_of('#');
	return n != string_view::npos && n != 0;
}

VS_ServerTypes VS_GetServerType(string_view uid)
{
	if (uid.empty())
		return ST_UNKNOWN;

	auto pos = uid.find_first_of('#');
	if (pos == string_view::npos)
		return ST_UNKNOWN;

	uid.remove_prefix(++pos);	// skip '#'

	if (boost::iequals(uid, "as"))
		return ST_AS;
	else if (boost::iequals(uid, "bs"))
		return ST_BS;
	else if (boost::iequals(uid, "rs"))
		return ST_RS;
	else if (boost::iequals(uid, "sm"))
		return ST_SM;
	else if (boost::iequals(uid, "vcs"))
		return ST_VCS;
	else if (boost::iequals(uid, "regs"))
		return ST_REGS;
	else if (boost::iequals(uid, "tmp"))
		return ST_TMP;

	return ST_UNKNOWN;
}

string_view VS_RemoveServerType(string_view server_name)
{
	const auto pos = server_name.find_first_of('#');
	if (pos != string_view::npos)
		server_name.remove_suffix(server_name.length() - pos);
	return server_name;
}

std::string VS_GetDBConnectionString()
{
	std::string result;

	const auto& reg_backend = VS_RegistryKey::GetDefaultBackendConfiguration();
	if      (boost::starts_with(reg_backend, string_view("postgresql:")))
	{
		// Use the same DB connection string that registry backed uses, but with @blob=bytea option added.
		result = reg_backend;

		// Remove existing @blob option
		const auto blob_opt_start = result.find("@blob=");
		if (blob_opt_start != result.npos)
		{
			const auto blob_opt_end = result.find(';', blob_opt_start);
			assert(blob_opt_start > 0);
			result.erase(blob_opt_start, blob_opt_end - blob_opt_start + 1);
		}

		assert(!result.empty());
		if (result.back() != ';')
			result += ';';
		result += "@blob=bytea";
	}
	else if (boost::starts_with(reg_backend, string_view("registry:")))
	{
		// If we use the Windows registry backend then DB connection string must be stored in there.
		VS_RegistryKey(false, CONFIGURATION_KEY).GetString(result, DB_CONNECTIONSTRING_TAG);
	}

	return result;
}

static void VS_RestoreOutput(FILE* stream, std::ostream& cpp_stream, int original_fd)
{
	// If C stream was closed then dup2() won't be enough to restore its state.
	// Because of that we open /dev/null first (this shouldn't fail).
	auto ret = freopen(DEV_NULL, "a", stream);
	assert(ret);

	// Associate original file descriptor with the stream.
	dup2(original_fd, fileno(stream));

	// Reset C++ stream state, freopen() already did this for the C stream.
	cpp_stream.clear();
}

static bool VS_RedirectOutput(const char* file, FILE* stream, std::ostream& cpp_stream, int original_fd)
{
	// Open file and associate existing stream with it.
	if (!freopen(file, "a", stream))
	{
		// Open failed and stream is now closed, redirect to default destination.
		VS_RestoreOutput(stream, cpp_stream, original_fd);
		return false;
	}

	// Reset C++ stream state, freopen() already did this for the C stream.
	cpp_stream.clear();

	// Disable buffering.
	setbuf(stream, nullptr);

	return true;
}

bool VS_RedirectOutput(const char* file)
{
	// Duplicate file descriptors of original stdout and stderr.
	static const int stdout_fd = dup(fileno(stdout));
	static const int stderr_fd = dup(fileno(stderr));

	// Flush streams before doing anything.
	std::cout.flush();
	fflush(stdout);
	std::cerr.flush();
	fflush(stderr);

	if (file)
	{
		bool result = true;
		result = VS_RedirectOutput(file, stdout, std::cout, stdout_fd) && result;
		result = VS_RedirectOutput(file, stderr, std::cerr, stderr_fd) && result;
		return result;
	}
	else
	{
		VS_RestoreOutput(stdout, std::cout, stdout_fd);
		VS_RestoreOutput(stderr, std::cerr, stderr_fd);
		return true;
	}
}

std::string VS_GetExecutablePath()
{
#if defined(_WIN32)

	wchar_t buffer[MAX_PATH];
	const auto length = GetModuleFileNameW(NULL, buffer, MAX_PATH);
	if (length == 0 || length == MAX_PATH)
		return {};
	assert(length < MAX_PATH);
	return vs::UTF16toUTF8Convert(wstring_view(buffer, length));

#elif defined(__linux__)

	char buffer[PATH_MAX];
	const auto length = readlink("/proc/self/exe", buffer, PATH_MAX);
	if (length == -1 || length == PATH_MAX)
		return {};
	assert(length > 0);
	assert(length < PATH_MAX);
	assert(buffer[0] == '/');
	return { buffer, static_cast<size_t>(length) };

#else
#error Unknown platform
#endif
}

std::string VS_GetExecutableDirectory()
{
	std::string result = VS_GetExecutablePath();
	if (result.empty())
		return result;

#if defined(_WIN32)
	constexpr char path_separator = '\\';
#else
	constexpr char path_separator = '/';
#endif
	const auto sep_pos = result.rfind(path_separator);
	if (sep_pos != result.npos)
		result.resize(sep_pos + 1);
	else
		result.clear();
	return result;
}

#if defined(_WIN32) // Not ported yet

#define SECURITY_WIN32
#include <Security.h>
typedef BOOLEAN(WINAPI * GETUSERNAMEEX)(EXTENDED_NAME_FORMAT, LPTSTR, PULONG);

bool VS_GetUserName(char* id, unsigned long id_sz)
{
	HINSTANCE h = LoadLibrary("secur32.dll");
	bool ret = false;
	if (h) {
		GETUSERNAMEEX f = (GETUSERNAMEEX) GetProcAddress(h, "GetUserNameExA");
		if (f)
			ret = f((EXTENDED_NAME_FORMAT)NameSamCompatible, id, &id_sz)!=0;
		FreeLibrary(h);
	}
	if (!ret)
		ret = GetUserNameA(id, &id_sz)!=0;

	return ret;
}

bool VS_GetDiskSerialNumber(char* id, unsigned int size)
{
	if (!id || size < 16)
		return false;
	char buff[MAX_PATH] = {0};
	if (GetWindowsDirectory(buff, MAX_PATH)) {
		char drive[_MAX_FNAME], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
		_splitpath(buff, drive, dir, fname, ext);
		strcat(drive, "\\");

		DWORD VolumeSerialNumber = 0, MaximumComponentLength, FileSystemFlags;
		GetVolumeInformation(drive, 0, 0, &VolumeSerialNumber, &MaximumComponentLength, &FileSystemFlags, 0, 0);
		if (VolumeSerialNumber) {
			_itoa(VolumeSerialNumber, id, 10);
			return true;
		}
	}
	return false;
}

bool VS_GetWinId(char* id, unsigned long size)
{
	if (!id || size < 32)
		return false;

	std::string key;
	HKEY win_key;
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_QUERY_VALUE|KEY_WOW64_64KEY, &win_key) == ERROR_SUCCESS)
	{
		DWORD reg_size;
		DWORD reg_type;
		LONG ret;

		ret = RegQueryValueExA(win_key, "ProductId", 0, 0, 0, &reg_size);
		if ((ret == ERROR_SUCCESS || ret == ERROR_MORE_DATA) && reg_size > 0)
		{
			auto buffer = std::make_unique<char[]>(reg_size);
			if (RegQueryValueExA(win_key, "ProductId", 0, &reg_type, reinterpret_cast<LPBYTE>(buffer.get()), &reg_size) == ERROR_SUCCESS)
			{
				if (reg_type == REG_SZ || reg_type == REG_EXPAND_SZ || reg_type == REG_MULTI_SZ)
					key.append(buffer.get(), reg_size);
			}
		}
		ret = RegQueryValueExA(win_key, "InstallDate", 0, 0, 0, &reg_size);
		if ((ret == ERROR_SUCCESS || ret == ERROR_MORE_DATA) && reg_size > 0)
		{
			auto buffer = std::make_unique<char[]>(reg_size);
			if (RegQueryValueExA(win_key, "ProductId", 0, &reg_type, reinterpret_cast<LPBYTE>(buffer.get()), &reg_size) == ERROR_SUCCESS)
			{
				if ((reg_type == REG_DWORD || reg_type == REG_BINARY) && reg_size == sizeof(int))
				{
					char str_value[std::numeric_limits<int>::digits10 + 1 + 1] = {};
					sprintf(str_value, "%d", *reinterpret_cast<const int*>(buffer.get()));
					key += str_value;
				}
				else if (reg_type == REG_SZ || reg_type == REG_EXPAND_SZ || reg_type == REG_MULTI_SZ)
					key.append(buffer.get(), reg_size);
			}
		}
		RegCloseKey(win_key);
	}
	VS_ConvertToMD5(key, id);
	return true;
}


/********************************** Getting OS full name *****************************************/

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

#define BUFSIZE 256
#include <strsafe.h>

BOOL GetOsVersionRtl(RTL_OSVERSIONINFOEXW& OsVer)
{
	typedef LONG(WINAPI* tRtlGetVersion)(RTL_OSVERSIONINFOEXW*);

	//memset(&OsVer, 0, sizeof(RTL_OSVERSIONINFOEXW));
	//OsVer.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

	HMODULE h_NtDll = GetModuleHandleW(L"ntdll.dll");
	if (h_NtDll == NULL)
		return FALSE;
	tRtlGetVersion f_RtlGetVersion = (tRtlGetVersion)GetProcAddress(h_NtDll, "RtlGetVersion");
	if (!f_RtlGetVersion)
		return FALSE; // This will never happen (all processes load ntdll.dll)

	LONG Status = f_RtlGetVersion(&OsVer);
	return Status == 0;
}

BOOL GetOSDisplayString(LPTSTR pszOS)
{
#if defined UNICODE
#error "GetOSDisplayString() Will not work for UNICODE"
#endif
   OSVERSIONINFOEXW osvi;
   SYSTEM_INFO si;
   PGNSI pGNSI;
   PGPI pGPI;
   DWORD dwType;

   ZeroMemory(&si, sizeof(SYSTEM_INFO));
   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));

   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

   if (GetOsVersionRtl(osvi) || GetVersionExW((OSVERSIONINFOW *)&osvi)) {
	   // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
	   pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
	   if(NULL != pGNSI)
		   pGNSI(&si);
	   else GetSystemInfo(&si);

	   if ( VER_PLATFORM_WIN32_NT==osvi.dwPlatformId && osvi.dwMajorVersion > 4 ) {
		   StringCchCopy(pszOS, BUFSIZE, TEXT("Microsoft "));

		   // Test for the specific product.
		   if ( osvi.dwMajorVersion >= 6 ) {

			   if (osvi.dwMajorVersion == 10) {
				   if (osvi.dwMinorVersion == 0) {
					   if (osvi.wProductType == VER_NT_WORKSTATION)
						   StringCchCat(pszOS, BUFSIZE, TEXT("Windows 10 "));
					   else {
						   StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server "));
						   if (osvi.dwBuildNumber < 17763)
							   StringCchCat(pszOS, BUFSIZE, TEXT("2016 "));
						   else
							   StringCchCat(pszOS, BUFSIZE, TEXT("2019 "));
					   }
				   }
			   }
			   else if (osvi.dwMajorVersion == 6) {

				   if (osvi.dwMinorVersion == 0) {
					   if (osvi.wProductType == VER_NT_WORKSTATION)
						   StringCchCat(pszOS, BUFSIZE, TEXT("Windows Vista "));
					   else
						   StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2008 "));
				   }

				   if (osvi.dwMinorVersion == 1) {
					   if (osvi.wProductType == VER_NT_WORKSTATION)
						   StringCchCat(pszOS, BUFSIZE, TEXT("Windows 7 "));
					   else
						   StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2008 R2 "));
				   }

				   if (osvi.dwMinorVersion == 2) {
					   if (osvi.wProductType == VER_NT_WORKSTATION)
						   StringCchCat(pszOS, BUFSIZE, TEXT("Windows 8 "));
					   else
						   StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2012 "));
				   }

				   if (osvi.dwMinorVersion == 3) {
					   if (osvi.wProductType == VER_NT_WORKSTATION)
						   StringCchCat(pszOS, BUFSIZE, TEXT("Windows 8.1 "));
					   else
						   StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2012 R2 "));
				   }
			   }

			   pGPI = (PGPI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
			   pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);

#define PRODUCT_MOBILE_ENTERPRISE			0x00000085 // Windows 10 Mobile Enterprise
#define PRODUCT_IOTUAPCOMMERCIAL			0x00000083 // Windows 10 IoT Core Commercial

#define PLNAME(a, b) case (a): StringCchCat(pszOS, BUFSIZE, TEXT(b)); break;
			   switch (dwType)
			   {
					PLNAME(PRODUCT_BUSINESS, "Business")
					PLNAME(PRODUCT_BUSINESS_N, "Business N")
					PLNAME(PRODUCT_CLUSTER_SERVER, "HPC Edition")
					PLNAME(PRODUCT_CLUSTER_SERVER_V, "Server Hyper Core V")
					PLNAME(PRODUCT_CORE, "Home")
					PLNAME(PRODUCT_CORE_COUNTRYSPECIFIC, "Home China")
					PLNAME(PRODUCT_CORE_N, "Home N")
					PLNAME(PRODUCT_CORE_SINGLELANGUAGE, "Home Single Language")
					PLNAME(PRODUCT_DATACENTER_EVALUATION_SERVER, "Server Datacenter(evaluation installation)")
					PLNAME(PRODUCT_DATACENTER_A_SERVER_CORE, "Server Datacenter, Semi - Annual Channel(core installation)")
					PLNAME(PRODUCT_STANDARD_A_SERVER_CORE, "Server Standard, Semi - Annual Channel(core installation)")
					PLNAME(PRODUCT_DATACENTER_SERVER, "Server Datacenter(full installation)")
					PLNAME(PRODUCT_DATACENTER_SERVER_CORE, "Server Datacenter(core installation)")
					PLNAME(PRODUCT_DATACENTER_SERVER_CORE_V, "Server Datacenter without Hyper - V(core installation)")
					PLNAME(PRODUCT_DATACENTER_SERVER_V, "Server Datacenter without Hyper - V(full installation)")
					PLNAME(PRODUCT_EDUCATION, "Education")
					PLNAME(PRODUCT_EDUCATION_N, "Education N")
					PLNAME(PRODUCT_ENTERPRISE, "Enterprise")
					PLNAME(PRODUCT_ENTERPRISE_E, "Enterprise E")
					PLNAME(PRODUCT_ENTERPRISE_EVALUATION, "Enterprise Evaluation")
					PLNAME(PRODUCT_ENTERPRISE_N, "Enterprise N")
					PLNAME(PRODUCT_ENTERPRISE_N_EVALUATION, "Enterprise N Evaluation")
					PLNAME(PRODUCT_ENTERPRISE_S, "Enterprise 2015 LTSB")
					PLNAME(PRODUCT_ENTERPRISE_S_EVALUATION, "Enterprise 2015 LTSB Evaluation")
					PLNAME(PRODUCT_ENTERPRISE_S_N, "Enterprise 2015 LTSB N")
					PLNAME(PRODUCT_ENTERPRISE_S_N_EVALUATION, "Enterprise 2015 LTSB N Evaluation")
					PLNAME(PRODUCT_ENTERPRISE_SERVER, "Server Enterprise(full installation)")
					PLNAME(PRODUCT_ENTERPRISE_SERVER_CORE, "Server Enterprise(core installation)")
					PLNAME(PRODUCT_ENTERPRISE_SERVER_CORE_V, "Server Enterprise without Hyper - V(core installation)")
					PLNAME(PRODUCT_ENTERPRISE_SERVER_IA64, "Server Enterprise for Itanium - based Systems")
					PLNAME(PRODUCT_ENTERPRISE_SERVER_V, "Server Enterprise without Hyper - V(full installation)")
					PLNAME(PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL, "Essential Server Solution Additional")
					PLNAME(PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC, "Essential Server Solution Additional SVC")
					PLNAME(PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT, "Essential Server Solution Management")
					PLNAME(PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC, "Essential Server Solution Management SVC")
					PLNAME(PRODUCT_HOME_BASIC, "Home Basic")
					PLNAME(PRODUCT_HOME_BASIC_E, "Not supported")
					PLNAME(PRODUCT_HOME_BASIC_N, "Home Basic N")
					PLNAME(PRODUCT_HOME_PREMIUM, "Home Premium")
					PLNAME(PRODUCT_HOME_PREMIUM_E, "Not supported")
					PLNAME(PRODUCT_HOME_PREMIUM_N, "Home Premium N")
					PLNAME(PRODUCT_HOME_PREMIUM_SERVER, "Home Server 2011")
					PLNAME(PRODUCT_HOME_SERVER, "Storage Server 2008 R2 Essentials")
					PLNAME(PRODUCT_HYPERV, "Hyper - V Server")
					PLNAME(PRODUCT_IOTUAP, "IoT Core")
					PLNAME(PRODUCT_IOTUAPCOMMERCIAL, "IoT Core Commercial")
					PLNAME(PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT, "Essential Business Server Management Server")
					PLNAME(PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING, "Essential Business Server Messaging Server")
					PLNAME(PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY, "Essential Business Server Security Server")
					PLNAME(PRODUCT_MOBILE_CORE, "Mobile")
					PLNAME(PRODUCT_MOBILE_ENTERPRISE, "Mobile Enterprise")
					PLNAME(PRODUCT_MULTIPOINT_PREMIUM_SERVER, "MultiPoint Server Premium(full installation)")
					PLNAME(PRODUCT_MULTIPOINT_STANDARD_SERVER, "MultiPoint Server Standard(full installation)")
					PLNAME(PRODUCT_PRO_WORKSTATION, "Pro for Workstations")
					PLNAME(PRODUCT_PRO_WORKSTATION_N, "Pro for Workstations N")
					PLNAME(PRODUCT_PROFESSIONAL, "Professional")
					PLNAME(PRODUCT_PROFESSIONAL_E, "Not supported")
					PLNAME(PRODUCT_PROFESSIONAL_N, "Professional N")
					PLNAME(PRODUCT_PROFESSIONAL_WMC, "Professional with Media Center")
					PLNAME(PRODUCT_SB_SOLUTION_SERVER, "Small Business Server 2011 Essentials")
					PLNAME(PRODUCT_SB_SOLUTION_SERVER_EM, "Server For SB Solutions EM")
					PLNAME(PRODUCT_SERVER_FOR_SB_SOLUTIONS, "Server For SB Solutions")
					PLNAME(PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM, "Server For SB Solutions EM")
					PLNAME(PRODUCT_SERVER_FOR_SMALLBUSINESS, "Server 2008 for Windows Essential Server Solutions")
					PLNAME(PRODUCT_SERVER_FOR_SMALLBUSINESS_V, "Server 2008 without Hyper - V for Windows Essential Server Solutions")
					PLNAME(PRODUCT_SERVER_FOUNDATION, "Server Foundation")
					PLNAME(PRODUCT_SMALLBUSINESS_SERVER, "Small Business Server")
					PLNAME(PRODUCT_SMALLBUSINESS_SERVER_PREMIUM, "Small Business Server Premium")
					PLNAME(PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE, "Small Business Server Premium(core installation)")
					PLNAME(PRODUCT_SOLUTION_EMBEDDEDSERVER, "MultiPoint Server")
					PLNAME(PRODUCT_STANDARD_EVALUATION_SERVER, "Server Standard(evaluation installation)")
					PLNAME(PRODUCT_STANDARD_SERVER, "Server Standard")
					PLNAME(PRODUCT_STANDARD_SERVER_CORE, "Server Standard(core installation)")
					PLNAME(PRODUCT_STANDARD_SERVER_CORE_V, "Server Standard without Hyper - V(core installation)")
					PLNAME(PRODUCT_STANDARD_SERVER_V, "Server Standard without Hyper - V")
					PLNAME(PRODUCT_STANDARD_SERVER_SOLUTIONS, "Server Solutions Premium")
					PLNAME(PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE, "Server Solutions Premium(core installation)")
					PLNAME(PRODUCT_STARTER, "Starter")
					PLNAME(PRODUCT_STARTER_E, "Not supported")
					PLNAME(PRODUCT_STARTER_N, "Starter N")
					PLNAME(PRODUCT_STORAGE_ENTERPRISE_SERVER, "Storage Server Enterprise")
					PLNAME(PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE, "Storage Server Enterprise(core installation)")
					PLNAME(PRODUCT_STORAGE_EXPRESS_SERVER, "Storage Server Express")
					PLNAME(PRODUCT_STORAGE_EXPRESS_SERVER_CORE, "Storage Server Express(core installation)")
					PLNAME(PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER, "Storage Server Standard(evaluation installation)")
					PLNAME(PRODUCT_STORAGE_STANDARD_SERVER, "Storage Server Standard")
					PLNAME(PRODUCT_STORAGE_STANDARD_SERVER_CORE, "Storage Server Standard(core installation)")
					PLNAME(PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER, "Storage Server Workgroup(evaluation installation)")
					PLNAME(PRODUCT_STORAGE_WORKGROUP_SERVER, "Storage Server Workgroup")
					PLNAME(PRODUCT_STORAGE_WORKGROUP_SERVER_CORE, "Storage Server Workgroup(core installation)")
					PLNAME(PRODUCT_ULTIMATE, "Ultimate")
					PLNAME(PRODUCT_ULTIMATE_E, "Not supported")
					PLNAME(PRODUCT_ULTIMATE_N, "Ultimate N")
					PLNAME(PRODUCT_UNDEFINED, "An unknown product")
					PLNAME(PRODUCT_WEB_SERVER, "Web Server(full installation)")
					PLNAME(PRODUCT_WEB_SERVER_CORE, "Web Server(core installation)")
					PLNAME(PRODUCT_DATACENTER_WS_SERVER_CORE, "DATACENTER_WS_SERVER_CORE")
					PLNAME(PRODUCT_STANDARD_WS_SERVER_CORE, "STANDARD_WS_SERVER_CORE")
					PLNAME(PRODUCT_UTILITY_VM, "UTILITY_VM")
					PLNAME(PRODUCT_DATACENTER_EVALUATION_SERVER_CORE, "DATACENTER_EVALUATION_SERVER_CORE")
					PLNAME(PRODUCT_STANDARD_EVALUATION_SERVER_CORE, "STANDARD_EVALUATION_SERVER_CORE")
					PLNAME(PRODUCT_PRO_FOR_EDUCATION, "Pro for Education")
					PLNAME(PRODUCT_PRO_FOR_EDUCATION_N, "Pro for Education N")
					PLNAME(PRODUCT_AZURE_SERVER_CORE, "Azure Server Core")
					PLNAME(PRODUCT_AZURE_NANO_SERVER, "Azure Nano Server")
			   default: StringCchCat(pszOS, BUFSIZE, TEXT("An unknown product")); break;
			   }
#undef PLNAME

			   if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
				   StringCchCat(pszOS, BUFSIZE, TEXT( ", 64-bit" ));
			   else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL )
				   StringCchCat(pszOS, BUFSIZE, TEXT(", 32-bit"));
		   }

		   if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 ) {
			   if( GetSystemMetrics(89) )
				   StringCchCat(pszOS, BUFSIZE, TEXT( "Windows Server 2003 R2, "));
			   else if ( osvi.wSuiteMask==VER_SUITE_STORAGE_SERVER )
				   StringCchCat(pszOS, BUFSIZE, TEXT( "Windows Storage Server 2003"));
			   else if ( osvi.wSuiteMask==VER_SUITE_WH_SERVER )
				   StringCchCat(pszOS, BUFSIZE, TEXT( "Windows Home Server"));
			   else if( osvi.wProductType == VER_NT_WORKSTATION &&
				   si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
			   {
				   StringCchCat(pszOS, BUFSIZE, TEXT( "Windows XP Professional x64 Edition"));
			   }
			   else StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2003, "));

			   // Test for the server type.
			   if ( osvi.wProductType != VER_NT_WORKSTATION ) {

				   if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 ) {
					   if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
						   StringCchCat(pszOS, BUFSIZE, TEXT( "Datacenter Edition for Itanium-based Systems" ));
					   else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						   StringCchCat(pszOS, BUFSIZE, TEXT( "Enterprise Edition for Itanium-based Systems" ));
				   }
				   else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 ) {
					   if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
						   StringCchCat(pszOS, BUFSIZE, TEXT( "Datacenter x64 Edition" ));
					   else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						   StringCchCat(pszOS, BUFSIZE, TEXT( "Enterprise x64 Edition" ));
					   else StringCchCat(pszOS, BUFSIZE, TEXT( "Standard x64 Edition" ));

				   }
				   else {
					   if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
						   StringCchCat(pszOS, BUFSIZE, TEXT( "Compute Cluster Edition" ));
					   else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
						   StringCchCat(pszOS, BUFSIZE, TEXT( "Datacenter Edition" ));
					   else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						   StringCchCat(pszOS, BUFSIZE, TEXT( "Enterprise Edition" ));
					   else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
						   StringCchCat(pszOS, BUFSIZE, TEXT( "Web Edition" ));
					   else StringCchCat(pszOS, BUFSIZE, TEXT( "Standard Edition" ));
				   }
			   }
		   }

		   if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 ) {
			   StringCchCat(pszOS, BUFSIZE, TEXT("Windows XP "));
			   if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
				   StringCchCat(pszOS, BUFSIZE, TEXT( "Home Edition" ));
			   else StringCchCat(pszOS, BUFSIZE, TEXT( "Professional" ));
		   }

		   if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 ) {
			   StringCchCat(pszOS, BUFSIZE, TEXT("Windows 2000 "));

			   if ( osvi.wProductType == VER_NT_WORKSTATION ) {
				   StringCchCat(pszOS, BUFSIZE, TEXT( "Professional" ));
			   }
			   else {
				   if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
					   StringCchCat(pszOS, BUFSIZE, TEXT( "Datacenter Server" ));
				   else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
					   StringCchCat(pszOS, BUFSIZE, TEXT( "Advanced Server" ));
				   else StringCchCat(pszOS, BUFSIZE, TEXT( "Server" ));
			   }
		   }

		   // Include service pack (if any) and build number.
		   if (osvi.szCSDVersion[0]) {
			   StringCchCat(pszOS, BUFSIZE, TEXT(" ") );
			   char tbuff[128] = {0};
			   sprintf(tbuff, "%S", osvi.szCSDVersion);
			   StringCchCat(pszOS, BUFSIZE, tbuff);
		   }

		   TCHAR buf[80];
		   StringCchPrintf( buf, 80, TEXT(" (build %d)"), osvi.dwBuildNumber);
		   StringCchCat(pszOS, BUFSIZE, buf);

		   return TRUE;
	   }
	   else if (VER_PLATFORM_WIN32_WINDOWS==osvi.dwPlatformId ) {
		   if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
			   StringCchCat(pszOS, BUFSIZE, TEXT( "Microsoft Windows 98 "));
			   if ( osvi.szCSDVersion[1]=='A' || osvi.szCSDVersion[1]=='B')
				   StringCchCat(pszOS, BUFSIZE, TEXT( "SE " ));
		   }
		   if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90) {
			   StringCchCat(pszOS, BUFSIZE, TEXT( "Microsoft Windows Millennium Edition"));
		   }

		   return TRUE;
	   }
   }
   return FALSE;
}

void VSGetSystemInfo_Performance(VS_SimpleStr &prop, int benchMBps, int sndMBps, int rcvMBps, int rcvGroupMBps)
{
	char str[1024] = {0};

	tc_VideoLevelCaps lvlCaps;
	unsigned char lvl;
	tc_levelVideo_t descLvl;

	lvl = lvlCaps.MBps2Level(benchMBps);
	lvlCaps.GetLevelDesc(lvl, &descLvl);
	sprintf(str, "      Benchmark: %5s [MBframe = %6d, MBps = %9d] \n", descLvl.name, descLvl.maxFrameSizeMB, descLvl.maxMBps);
	prop += str;
	lvl = lvlCaps.MBps2Level(sndMBps);
	lvlCaps.GetLevelDesc(lvl, &descLvl);
	sprintf(str, "         Sender: %5s [MBframe = %6d, MBps = %9d] \n", descLvl.name, descLvl.maxFrameSizeMB, descLvl.maxMBps);
	prop += str;
	lvl = lvlCaps.MBps2Level(rcvMBps);
	lvlCaps.GetLevelDesc(lvl, &descLvl);
	sprintf(str, "       Receiver: %5s [MBframe = %6d, MBps = %9d] \n", descLvl.name, descLvl.maxFrameSizeMB, descLvl.maxMBps);
	prop += str;
	lvl = lvlCaps.MBps2Level(rcvGroupMBps);
	lvlCaps.GetLevelDesc(lvl, &descLvl);
	sprintf(str, " Receiver Group: %5s [MBframe = %6d, MBps = %9d]", descLvl.name, descLvl.maxFrameSizeMB, descLvl.maxMBps);
	prop += str;
}

#endif


std::vector<std::pair<string_view, string_view>> VS_ParseUrlParams(string_view sv)
{
	std::vector<std::pair<string_view, string_view>> kv;
	typedef boost::algorithm::find_iterator<string_view::const_iterator> find_iterator;
	for (find_iterator params_it(sv, boost::algorithm::token_finder(
		[](char x) { return x != '?' && x != '&' && x != ' '; }, boost::algorithm::token_compress_on));
		!params_it.eof(); ++params_it)
	{
		find_iterator pair_it(*params_it, boost::algorithm::token_finder(
			[](char x) { return x != '='; }, boost::algorithm::token_compress_on));
		{
			string_view key(pair_it->begin(), boost::distance(*pair_it));
			++pair_it;
			if (!pair_it.eof())
			{
				string_view value(pair_it->begin(), boost::distance(*pair_it));
				kv.emplace_back(key, value);
			}
		}
	}
	return kv;
}

void VSGetSystemInfo_OS(VS_SimpleStr &prop)
{
#ifdef _WIN32
	prop.Resize(1024);
	GetOSDisplayString(prop);
#else
	static const std::string magic = "PRETTY_NAME=";
	std::string str;
	std::ifstream f("/etc/os-release");
	for (std::string line; !f.eof(); std::getline(f, line)) {
		auto pos = line.find(magic);
		if (pos == 0 && line.length() > magic.length())
		{
			pos += magic.length();
			if (line[pos] == '"')
				++pos;
			auto end = line.length() - pos;
			if (line.back() == '"' && end > 0)
				--end;
			str = line.substr(pos, end);
			break;
		}
	}
	utsname info;
	if (uname(&info) == 0)
	{
		str += "; kernel r=";
		str += info.release;
		str += "; v=";
		str += info.version;
	}
	prop = str.substr(0, 128).c_str();		// todo(kt): remove cut 128, when RegDB can store more data
#endif
}

void VSGetSystemInfo_Processor(VS_SimpleStr &prop)
{
	char str[1024] = { 0 };

#ifdef _WIN32
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	sprintf(str, "Proc: Type %ld Level %d; ", SysInfo.dwProcessorType, SysInfo.wProcessorLevel);
	prop = str;
#endif
	if (VS_GetCPUInternalName(str, 512))
		prop += str;
	unsigned int phc = 0, lc = 0;
	VS_GetNumCPUCores(&phc, &lc);
	sprintf(str, "; %d (cores) / %d (threads)", phc, lc);
	prop += str;
}

std::string VS_ValidateHtml(const std::string &text)
{
	static const boost::regex _e_valid_tags(R"(<(/?([biua]|br|a\s+href="[^"]+"))\s*>)", boost::regex::optimize | boost::regex::icase);
	static const boost::regex _e_not_valid_tags("<[^>]*(>|$)", boost::regex::optimize);
	static const boost::regex _e_temporary_replacement(R"(&left-tmp;(/?([biua]|br|a\s+href="[^"]+"))&right-tmp;)", boost::regex::optimize | boost::regex::icase);

	std::string res = boost::regex_replace(text, _e_valid_tags, "&left-tmp;$1&right-tmp;"); // find valid tags and replace with temp values
	res = boost::regex_replace(res, _e_not_valid_tags, "");                                 // erase not valid tags
	res = boost::regex_replace(res, _e_temporary_replacement, "<$1>");                      // restore valid tags

	return res;
}

#if defined(_WIN32) // Not ported yet

unsigned __stdcall VS_MakeDump_Thread(void* arg);
static int numOfDumps = 0;

void VS_MakeDump(VS_DebugDump_Type type)
{
	if (numOfDumps >= 90)
		return;

	HANDLE hThread = 0;
	unsigned threadID = 0;

	hThread = (HANDLE)_beginthreadex(0, 0, &VS_MakeDump_Thread, (void*)type, 0, &threadID);
	WaitForSingleObject(hThread, 4000);
	CloseHandle(hThread);
}

typedef BOOLEAN(WINAPI * MINIDUMPWRITEDUMP)(HANDLE, DWORD, HANDLE, DWORD, DWORD, DWORD, DWORD);
unsigned __stdcall VS_MakeDump_Thread(void* arg)
{
	vs::SetThreadName("MakeDump");
	numOfDumps++;
	char path[1024];
	DWORD ret = GetModuleFileName(0, path, 1024);
	if (!ret)
		return 0;

	char names[4][MAX_PATH];
	_splitpath(path, names[0], names[1], names[2], names[3]);
	SYSTEMTIME st;
	GetSystemTime(&st);
	sprintf(path, "%s%s%s_d%ld_%03d_%4d-%02d-%02d_%02d-%02d-%02d-%04d.dmp",names[0], names[1], names[2], (DWORD)arg, numOfDumps,
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_NEW, 0, 0);
	if (hFile==INVALID_HANDLE_VALUE)
		return 0;

	static HINSTANCE h = LoadLibraryA("dbghelp.dll");
	if (h) {
		MINIDUMPWRITEDUMP f = (MINIDUMPWRITEDUMP) GetProcAddress(h, "MiniDumpWriteDump");
		if (f)
			f(GetCurrentProcess(), GetCurrentProcessId(), hFile, (DWORD)arg, 0, 0, 0);
	}
	CloseHandle(hFile);
	return 0;
}

#endif
