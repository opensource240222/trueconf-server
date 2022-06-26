/**
 **************************************************************************
 * \file VS_Util.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief ancillary functions
 *
 * \b Project Standart Libraries
 * \author SMirnovK
 * \date 15.08.03
 *
 * $Revision: 15 $
 *
 * $History: VS_Utils.h $
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 5.07.12    Time: 13:53
 * Updated in $/VSNA/std/cpplib
 * - projects depens repaired
 *
 * *****************  Version 14  *****************
 * User: Mushakov     Date: 13.07.11   Time: 17:35
 * Updated in $/VSNA/std/cpplib
 * - Roaming
 *
 * *****************  Version 13  *****************
 * User: Ktrushnikov  Date: 28.01.11   Time: 22:25
 * Updated in $/VSNA/std/cpplib
 * VCS3.2
 * - VS_PresenceService::NetworkResolve() support for VCS (using
 * confRestrick with AS)
 * - common func VS_GetServerFromCallID() to VS_Utils
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 12.08.09   Time: 16:54
 * Updated in $/VSNA/std/cpplib
 * - old hardware key for client
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 21.04.08   Time: 20:34
 * Updated in $/VSNA/std/cpplib
 * - new Application Id algorithm
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 26.02.08   Time: 16:54
 * Updated in $/VSNA/std/cpplib
 * - new servers coonect shceme
 *
 * *****************  Version 9  *****************
 * User: Ktrushnikov  Date: 22.02.08   Time: 12:02
 * Updated in $/VSNA/std/cpplib
 * - GetUserNameA() added to support Win98
 * - func renamed
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 20.02.08   Time: 19:34
 * Updated in $/VSNA/std/cpplib
 * - hardware key
 *
 * *****************  Version 7  *****************
 * User: Ktrushnikov  Date: 20.02.08   Time: 16:34
 * Updated in $/VSNA/std/cpplib
 * - new func added: VS_Utils::VS_GetServerType()
 * - SM:ManagerService:DDNS_UPDATE add only AS servers
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 31.01.08   Time: 22:08
 * Updated in $/VSNA/std/cpplib
 * - bugs
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 31.01.08   Time: 19:04
 * Updated in $/VSNA/std/cpplib
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 30.01.08   Time: 19:26
 * Updated in $/VSNA/std/cpplib
 * - VS_Utils: VS_GetUniqueID_LoginComputer() func added
 * - test for above func added at _TestStandardLibraries
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 16.01.08   Time: 15:15
 * Updated in $/VSNA/std/cpplib
 * - network settings repaired
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
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 28.11.06   Time: 16:44
 * Updated in $/VS/std/cpplib
 * - more complex random generation functions
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 1.08.05    Time: 19:23
 * Updated in $/VS/std/cpplib
 * - moved GenRAndom()
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
 ****************************************************************************/
#ifndef VS_STD_UTILS_H
#define VS_STD_UTILS_H

#include "std-generic/attributes.h"
#include "VS_SimpleStr.h"

#include <array>
#include <cstdint>
#include <chrono>
#include "std-generic/cpplib/string_view.h"
#include <vector>

enum VS_ServerTypes : int;

/// read unique keys based on disk and OS
bool VS_ReadDiskandOsKeys(VS_SimpleStr &key);

///Generate unique key based on hardware configuration
bool VS_ReadClientHardwareKey(VS_SimpleStr &key);

// Returns key unique to the installation (based on hardware configuration).
// Can be used as AES-256 key to encrypt/decrypt data that shouldn't be readable by everyone before storing it in the registry.
std::array<unsigned char, 32> VS_GetPersistentKey();

///Generate unique MD5 16 byte hash
void VS_GenKeyByMD5(unsigned char key[16]);
///Generate unique MD5 string (32 symbols+\0)
void VS_GenKeyByMD5(char *key);
///Generate unique MD5 4-byte hash
inline uint32_t VS_GenKeyByMD5()
{
	unsigned char key[16];
	VS_GenKeyByMD5(key);
	const uint32_t* const p = reinterpret_cast<const uint32_t*>(key);
	return p[0] ^ p[1] ^ p[2] ^ p[3];
}

/// Get POSIX time in milliseconds
unsigned long long VS_GetTimeOfDayMs();
///Check format
bool VS_IsBrokerFormat(string_view ep);
// Get Server Type in NewArch
VS_ServerTypes VS_GetServerType(string_view uid);
string_view VS_RemoveServerType(string_view server_name);

// Returns connection string usable by cppdb to the main storage DB.
std::string VS_GetDBConnectionString();

// Redirects stdout and stderr to specified file.
// Pass nullptr to revert to original file descriptors.
bool VS_RedirectOutput(const char* file);

// Returns absolute path to the running executable.
// In case of error returns empty string.
std::string VS_GetExecutablePath();

// Returns absolute path to the directory containing the running executable (directory part of the GetExecutablePath() result).
// Returned path is guaranteed to have native path separator at the end.
// In case of error returns empty string.
std::string VS_GetExecutableDirectory();

// Get System Information
void VSGetSystemInfo_OS(VS_SimpleStr &prop);
// Get Processor description
void VSGetSystemInfo_Processor(VS_SimpleStr &prop);
// Get Performance Level
void VSGetSystemInfo_Performance(VS_SimpleStr &prop, int benchMBps, int sndMBps, int rcvMBps, int rcvGroupMBps);

std::vector<std::pair<string_view, string_view>> VS_ParseUrlParams(string_view sv);

template <class InputIterator, class OutputIterator>
OutputIterator VS_FilterPath(InputIterator first, InputIterator last, OutputIterator result)
{
	for (; first != last; (void)++first, ++result)
	{
		const char c = *first;
		if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
			*result = c;
		else
			*result = '_';
	}
	return result;
}
inline char* VS_FilterPath(char* path)
{
	VS_FilterPath(path, path + std::strlen(path), path);
	return path;
}

std::string VS_ValidateHtml(const std::string &text);

// Return Sum of Elements (Check Sum) counted by simple algorithm
inline uint32_t VS_SimpleChkSum(const void* buffer, size_t size, const uint32_t start_sequence = 0x3D141592)
{
	uint32_t chkSum = start_sequence;
	size_t i = 0;
	for (; i < size / sizeof(uint32_t); ++i)
		chkSum += static_cast<const uint32_t*>(buffer)[i] + 1;
	switch (size % sizeof(uint32_t))
	{
	case 3:
		chkSum += static_cast<const uint8_t*>(buffer)[i*sizeof(uint32_t) + 2] + 1;
		VS_FALLTHROUGH;
	case 2:
		chkSum += static_cast<const uint8_t*>(buffer)[i*sizeof(uint32_t) + 1] + 1;
		VS_FALLTHROUGH;
	case 1:
		chkSum += static_cast<const uint8_t*>(buffer)[i*sizeof(uint32_t) + 0] + 1;
	}
	return chkSum;
}

// Make process dump
enum VS_DebugDump_Type
{
	DDT_MINI = 0,
	DDT_FULL = 2,
};
void VS_MakeDump(VS_DebugDump_Type type = DDT_MINI);

#endif
