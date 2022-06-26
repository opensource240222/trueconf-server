/**
 **************************************************************************
 * \file VS_Debug.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Debug variables in Visicron Server services
 *
 * \b Project Standart Libraries
 * \author StasS
 * \author SMirnovK
 * \date 22.01.04
 *
 * $Revision: 8 $
 *
 * $History: VS_Debug.cpp $
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 10.05.12   Time: 15:12
 * Updated in $/VSNA/std/debuglog
 * -ver up
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 4.05.12    Time: 20:33
 * Updated in $/VSNA/std/debuglog
 *  - onConnect (err) handled in transcoder
 *  - refactoring subscribtion (VS_ExternalPresenceInterface added)
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 3.05.12    Time: 22:00
 * Updated in $/VSNA/std/debuglog
 *  - logging gateway
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 9.02.09    Time: 13:46
 * Updated in $/VSNA/std/debuglog
 * \n - removed
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 30.01.09   Time: 16:57
 * Updated in $/VSNA/std/debuglog
 * - time added in dprint#
 * - reset flag added to sync
 * - wait reset timeout added to unsubscribe
 * - logging large msg (>25000 b)
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 18.06.08   Time: 17:17
 * Updated in $/VSNA/std/debuglog
 * - dump near exe
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 18.06.08   Time: 12:59
 * Updated in $/VSNA/std/debuglog
 * - dump added
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/debuglog
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/debuglog
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 30.12.04   Time: 14:12
 * Updated in $/VS/std/debuglog
 * files headers
 *
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VS_Debug.h"
#include "../cpplib/VS_RegistryKey.h"
#include "../cpplib/VS_RegistryConst.h"
#include "std-generic/cpplib/AtomicCache.h"
#include "std-generic/cpplib/string_view.h"
#include "std/cpplib/ThreadUtils.h"
#include "std-generic/clib/vs_time.h"
#include "std-generic/cpplib/scope_exit.h"

#include "std-generic/compat/iomanip.h"
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <mutex>

#if defined(_WIN32) && defined(_SVKS_M_BUILD_)
#	include <Windows.h>
#endif

std::atomic<unsigned> g_debug_level(0);
std::atomic<unsigned> g_debug_modules(0);
std::atomic<FILE*>	g_debug_fd(nullptr);

void VS_SetDebug(unsigned level, unsigned modules)
{
	g_debug_level.store(level, std::memory_order_relaxed);
	g_debug_modules.store(modules, std::memory_order_relaxed);
}
void VS_SetDebugFile(FILE * fd)
{
	g_debug_fd.store(fd, std::memory_order_relaxed);
}
#define DEBUG_CURRENT_MODULE VS_DM_NONE

void VS_ReadDebugKeys()
{
	using clock = std::chrono::steady_clock;

	const auto check_duration = std::chrono::seconds(2);
	static std::atomic<clock::rep> last_check_time_raw(
		(clock::now() - check_duration * 2).time_since_epoch().count()
	);

	const auto last_check_time = clock::time_point(clock::duration(last_check_time_raw.load(std::memory_order_relaxed)));
	const auto now = clock::now();
	if (now - last_check_time < check_duration)
		return;
	last_check_time_raw.store(now.time_since_epoch().count(), std::memory_order_relaxed);

	VS_RegistryKey cfg(false, CONFIGURATION_KEY);

	unsigned level = 1;
	cfg.GetValue(&level, sizeof(level), VS_REG_INTEGER_VT, "Debug Level");
	auto old_level = VS_GetDebugLevel();
	if (old_level != level)
		dstream0 << "Debug Level changed: old=" << old_level << ", new=" << level;
	g_debug_level.store(level, std::memory_order_relaxed);

	unsigned modules = 0;
	cfg.GetValue(&modules, sizeof(modules), VS_REG_INTEGER_VT, "Debug Modules");
	auto old_modules = g_debug_modules.load(std::memory_order_relaxed);
	if (old_modules != modules)
		dstream0 << "Debug Modules changed: old=" << std::hex << old_modules << ", new=" << modules;
	g_debug_modules.store(modules, std::memory_order_relaxed);
}

unsigned VS_GetDebugLevel()
{
	return g_debug_level.load(std::memory_order_relaxed);
}

std::mutex stdout_mtx;

static vs::AtomicCache<char*, std::default_delete<char[]>, 8> s_log_buffer_cache(8);

const char* VS_LogEntry::get_module_name(VS_DebugModule module)
{
	switch (module)
	{
	case VS_DM_NONE:                  return "NONE"; break;
	case VS_DM_READLIC:               return "READLIC"; break;
	case VS_DM_RSTOR:                 return "STORAGE"; break;
	case VS_DM_DBSTOR:                return "DBSTOR"; break;
	case VS_DM_USPRS:                 return "PRESENCE"; break;
	case VS_DM_LOGINS:                return "LOGINS"; break;
	case VS_DM_MCS:                   return "MCONF"; break;
	case VS_DM_LOGS:                  return "LOGS"; break;
	case VS_DM_STREAMS:               return "STREAMS"; break;
	case VS_DM_CONFIGS:               return "CONFIGS"; break;
	case VS_DM_CONFS:                 return "CONF"; break;
	case VS_DM_CLUSTS:                return "CLUSTER"; break;
	case VS_DM_CHATS:                 return "CHAT"; break;
	case VS_DM_REGDBST:               return "REGDBST"; break;
	case VS_DM_FILETRANSFER:          return "BTRNT"; break;
	case VS_DM_REGS:                  return "REGS"; break;
	case VS_DM_DIRS:                  return "DIRS"; break;
	case VS_DM_SMTP:                  return "SMTP"; break;
	case VS_DM_SERVCONFIG:            return "SRV_CFG"; break;
	case VS_DM_RESOLVE:               return "RESOLVE"; break;
	case VS_DM_LOGSERVICE:            return "LOG_SRV"; break;
	case VS_DM_DSCONTROL:             return "DSCNTRL"; break;
	case VS_DM_VERIFYSERVICE:         return "VERIFY"; break;
	case VS_DM_MULTIGATEWAY:          return "MULTIGW"; break;
	case VS_DM_SIPPARSER:             return "SIP"; break;
	case VS_DM_WEBRTC_PEERCONNECTION: return "WEBRTC"; break;
	case VS_DM_TRANSPORT:             return "TRANSPORT"; break;
	case VS_DM_H323PARSER:            return "H323"; break;
	case VS_DM_TRANSCEIVER:           return "TRANSCEIVER"; break;
	case VS_DM_OTHER:                 return "OTHER"; break;
	case VS_DM_NHP:                   return "NHP"; break;
	case VS_DM_FAKE_CLIENT:           return "FAKECLI"; break;
	case VS_DM_PHP:					  return "TC_PHP"; break;
	default:                          return "UNKNOWN"; break;
	}
}

VS_LogEntry::VS_LogEntry(unsigned level, VS_DebugModule module)
	: m_level(level)
	, m_module(module)
	, m_stream(&m_streambuf)
{
	const bool enabled = m_level == 0 || (m_level <= g_debug_level.load(std::memory_order_relaxed) && m_module & g_debug_modules.load(std::memory_order_relaxed));
	if (!enabled)
	{
		m_module = VS_DM_NONE;
		return;
	}

	m_buffer.reset(s_log_buffer_cache.Get());
	if (!m_buffer)
		m_buffer.reset(new char[c_buffer_size]);

	m_streambuf.pubsetbuf(m_buffer.get(), c_buffer_size - 1); // Reserve 1 char for terminating '\n'
	std::chrono::system_clock::time_point tp(std::chrono::system_clock::now());
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count() % 1000;
	auto now(std::chrono::system_clock::to_time_t(tp));
	tm now_tm;
	m_stream.imbue(std::locale::classic());
	auto flags(m_stream.flags());
	m_stream
		<< vs::put_time(localtime_r(&now, &now_tm), "%d/%m/%Y %H:%M:%S") << "." << std::setw(3) << std::setfill('0') << ms <<" |"
		<< m_level << "| "
		<< std::setw(4) << std::setfill('0') << std::hex << vs::GetThreadID() << std::setfill(' ') << "| "
		<< std::setw(7) << std::right << string_view(get_module_name(m_module)).substr(0, 7) << "| ";
	m_stream.flags(flags);
	m_header_sz = m_streambuf.pubseekoff(0,std::ios_base::cur);
}

VS_LogEntry::~VS_LogEntry()
{
	if (!enabled())
		return;

	size_t length = m_streambuf.pubseekoff(0, std::ios_base::cur);
	assert(length >= m_header_sz);
	assert(length <= c_buffer_size - 1);
	if (!(length >= m_header_sz && length <= c_buffer_size - 1))
		return; // This is a fail-safe for release build
	// Drop empty message
	if (length == m_header_sz)
		return;
	// Add terminating '\n' if needed
	if (m_buffer[length - 1] != '\n')
		m_buffer[length++] = '\n';

#if defined(_WIN32) && defined(_SVKS_M_BUILD_)
	HANDLE g_mut = CreateMutex(NULL, FALSE, "DebugLockForDLL");
	if (g_mut == NULL)
		return;
	DWORD result;
	result = WaitForSingleObject(g_mut, 1000);
#endif

	FILE *dbgOut = g_debug_fd.load(std::memory_order_relaxed);
	if (!dbgOut)
		dbgOut = stdout;

	{
		std::lock_guard<std::mutex> lock(stdout_mtx);
		fwrite(m_buffer.get(), length, 1, dbgOut);
	}

#if defined(_WIN32) && defined(_SVKS_M_BUILD_)
	if (result == WAIT_OBJECT_0)
		ReleaseMutex(g_mut);
	CloseHandle(g_mut);
#endif

	m_stream.rdbuf(nullptr);
	s_log_buffer_cache.Put(m_buffer.release());
}

void VS_LogEntry::printf(const char* format, ...)
{
	// vsnprintf sometimes returns error when %S is used in the format string.
	// This breaks workaround for non-standard compliant return code from
	// vsnprintf in MSVS 2013.
	// Since %S is non-standard and neither it nor the standard compliant
	// version (%ls) works, they are banned in dprint. Use dstream if you want
	// to print wide strings to log.
	assert(strstr(format, "%S") == nullptr);
	assert(strstr(format, "%ls") == nullptr);

	if (!enabled())
		return;

	size_t length = m_streambuf.pubseekoff(0, std::ios_base::cur);
	assert(!(length > c_buffer_size));
	if (length > c_buffer_size)
		return;

	if (length == c_buffer_size)
		return; // buffer is full, no reason to continue

	int written;
	va_list vlist;
	va_start(vlist, format);
	written = vsnprintf(m_buffer.get() + length, c_buffer_size - length, format, vlist);
#if defined(_MSC_VER) && _MSC_VER < 1900
	// MSVS 2013 has broken vsnprintf that doesn't write null-terminator and returns non-conformant value on overflow
	if (written == -1)
		written = c_buffer_size; // large enough value to indicate lack of space
#endif
	va_end(vlist);

	if (written < 0) // error occured, nothing can be added
		written = 0;
	else if (static_cast<size_t>(written) >= c_buffer_size - length) // not enough space in m_buffer, less characters were actually written
		written = c_buffer_size - length - 1;
	m_streambuf.pubseekoff(written, std::ios_base::cur);
}
