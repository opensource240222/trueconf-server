/**
 **************************************************************************
 * \file VS_Debug.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Debug print macros used in Visicron Server services
 *
 * \b Project Standart Libraries
 * \author StasS
 * \author SMirnovK
 * \date 22.01.04
 *
 * $Revision: 9 $
 *
 * $History: VS_Debug.h $
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 3.05.12    Time: 22:00
 * Updated in $/VSNA/std/debuglog
 *  - logging gateway
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 10.09.10   Time: 20:24
 * Updated in $/VSNA/std/debuglog
 * - Registration on SM added
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 30.01.09   Time: 16:57
 * Updated in $/VSNA/std/debuglog
 * - time added in dprint#
 * - reset flag added to sync
 * - wait reset timeout added to unsubscribe
 * - logging large msg (>25000 b)
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 18.06.08   Time: 17:17
 * Updated in $/VSNA/std/debuglog
 * - dump near exe
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 18.06.08   Time: 17:04
 * Updated in $/VSNA/std/debuglog
 * - VS_MemoryLeak included
 * - Logging to smtp service added
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 18.06.08   Time: 12:59
 * Updated in $/VSNA/std/debuglog
 * - dump added
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 25.04.08   Time: 14:17
 * Updated in $/VSNA/std/debuglog
 * dprint3 to LogService added
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 5.12.07    Time: 11:09
 * Updated in $/VSNA/std/debuglog
 * BS - new iteration
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/debuglog
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/debuglog
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 30.12.04   Time: 14:12
 * Updated in $/VS/std/debuglog
 * files headers
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 29.09.04   Time: 14:24
 * Updated in $/VS/std/debuglog
 * pragma_once removed
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 26.08.04   Time: 14:44
 * Updated in $/VS/std/debuglog
 * removed h323 service
 *
 * *****************  Version 2  *****************
 * User: Avlaskin     Date: 16.03.04   Time: 20:02
 * Updated in $/VS/std/debuglog
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 22.01.04   Time: 15:42
 * Created in $/VS/std/debuglog
 * moved debug print funcs
 *
 * *****************  Version 6  *****************
 * User: Avlaskin     Date: 12.01.04   Time: 19:54
 * Updated in $/VS/Servers/Server
 ****************************************************************************/

// This check is designed to prevent definitions of DEBUG_CURRENT_MODULE leaking from one file to the other.
// Such leakage is bad because the second file may then log its messages with an unrelated module code (if it doesn't override DEBUG_CURRENT_MODULE with a correct value).
// This check shouldn't have false positives as long as DEBUG_CURRENT_MODULE is undefined at the end of headers and source files compiled as unity.
#if defined(DEBUG_CURRENT_MODULE)
#error DEBUG_CURRENT_MODULE is already defined! Someone forgot to undefine it in some file included from this one. Rerun compilation with /showIncludes (MSVC) or -H (GCC/Clang) to get more info.
#endif

// Don't replace this this include guard with "pragma once", it will break the check above.
#ifndef VS_DEBUG_H
#define VS_DEBUG_H

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "../cpplib/membuf.h"
#include "std-generic/cpplib/utf8.h"

#include <memory>
#include <ostream>
#include <type_traits>

void VS_SetDebug(unsigned level, unsigned modules);
void VS_SetDebugFile(FILE* fd);
void VS_ReadDebugKeys();
unsigned VS_GetDebugLevel();

/**
 **************************************************************************
 * \brief enum of biary bit flags, every flag for one service
 ****************************************************************************/
enum VS_DebugModule : unsigned
{
	VS_DM_NONE			= 0x00000000,
	VS_DM_READLIC		= 0x00000001, // ServerServices\VS_ReadLicense.cpp
	VS_DM_RSTOR			= 0x00000002, // diff storages
	VS_DM_DBSTOR		= 0x00000004, // BS stoarge
	VS_DM_USPRS			= 0x00000008, // staus, precence, adress books
	VS_DM_LOGINS		= 0x00000010, // authorization
	VS_DM_MCS			= 0x00000020, // multi conference
	VS_DM_LOGS			= 0x00000040, // log utils
	VS_DM_STREAMS		= 0x00000080, // streams
	VS_DM_CONFIGS		= 0x00000100, // app config
	VS_DM_CONFS			= 0x00000200, // conference, use VS_DM_MCS?
	VS_DM_CLUSTS		= 0x00000400, // servers interaction
	VS_DM_CHATS			= 0x00000800, // chat
	VS_DM_REGDBST		= 0x00001000, // reg DB
	VS_DM_FILETRANSFER	= 0x00002000, // file transfer related
	VS_DM_REGS			= 0x00004000, // registration server
	VS_DM_DIRS			= 0x00008000, // routing server, outdated
	VS_DM_SMTP			= 0x00010000, // SMTP component
	VS_DM_SERVCONFIG	= 0x00020000,
	VS_DM_RESOLVE  		= 0x00040000, // resolving related
	VS_DM_LOGSERVICE	= 0x00080000, // log services
	VS_DM_DSCONTROL		= 0x00100000, // desktop control
	VS_DM_VERIFYSERVICE = 0x00200000, // ServerServices\VS_VerificationService.cpp
	VS_DM_MULTIGATEWAY	= 0x00400000,
	VS_DM_SIPPARSER		= 0x00800000,
	VS_DM_WEBRTC_PEERCONNECTION	   =  0x01000000,
	VS_DM_TRANSPORT		= 0x02000000,
	VS_DM_H323PARSER	= 0x04000000,
	VS_DM_TRANSCEIVER	= 0x08000000,
	VS_DM_OTHER			= 0x10000000,
	VS_DM_NHP			= 0x20000000,
	VS_DM_FAKE_CLIENT	= 0x40000000,
	VS_DM_PHP			= 0x80000000,
	VS_DM_ALL_			= 0xffffffff
	// Remember to update module names in VS_LogEntry::get_module_name after modifying this enum
};

class VS_LogEntry
{
public:
	VS_LogEntry(unsigned level, VS_DebugModule module);
	~VS_LogEntry();
	// Move ctor is declared to enable this usage pattern:
	//     auto ds = VS_LogEntry(...)
	// It's left undefined because we don't actually want to move
	// (and also because move ctor is impossible to implement (std::ostream has protected move ctor)).
	VS_LogEntry(VS_LogEntry&&);

#if defined(__GNUC__)
	__attribute__ ((format (printf, 2, 3)))
#endif
	void printf(const char* format, ...);
	template <class T>
	VS_LogEntry& operator<<(const T& x)
	{
		static_assert(!std::is_base_of<class VS_SimpleStr, T>::value, "Don't pass VS_SimpleStr here directly, pass m_str member instead.");
		if (!enabled())
			return *this;

		m_stream << x;
		return *this;
	}
	template <class T>
	VS_LogEntry& operator<<(T& x)
	{
		static_assert(!std::is_base_of<class VS_SimpleStr, T>::value, "Don't pass VS_SimpleStr here directly, pass m_str member instead.");
		if (!enabled())
			return *this;

		m_stream << x;
		return *this;
	}
	VS_LogEntry& operator<<(const char* x)
	{
		if (!enabled())
			return *this;

		m_stream << (x ? x : "(null)");
		return *this;
	}
	VS_LogEntry& operator<<(char* x)
	{
		return operator<<(static_cast<const char*>(x));
	}
#if defined(_WIN32) // Not ported yet
	VS_LogEntry& operator<<(const wchar_t* x)
	{
		if (!enabled())
			return *this;

		if (x)
			m_stream << vs::UTF16toUTF8Convert(x);
		else
			m_stream << "(null)";
		return *this;
	}
	VS_LogEntry& operator<<(const std::wstring& x)
	{
		if (!enabled())
			return *this;

		m_stream << vs::UTF16toUTF8Convert(x);
		return *this;
	}
	VS_LogEntry& operator<<(wchar_t* x)
	{
		return operator<<(static_cast<const wchar_t*>(x));
	}
	VS_LogEntry& operator<<(std::wstring& x)
	{
		return operator<<(static_cast<const std::wstring&>(x));
	}
#endif

	static const char* get_module_name(VS_DebugModule module);
	bool enabled() const
	{
		return m_module != VS_DM_NONE || m_level == 0;
	}

private:
	static const size_t c_buffer_size = 0xffff;

	unsigned m_level;
	VS_DebugModule m_module;
	size_t m_header_sz = 0;
	std::unique_ptr<char[]> m_buffer;
	omembuf m_streambuf;
	std::ostream m_stream;
};

#define dstream0 VS_LogEntry(0, DEBUG_CURRENT_MODULE)
#define dstream1 VS_LogEntry(1, DEBUG_CURRENT_MODULE)
#define dstream2 VS_LogEntry(2, DEBUG_CURRENT_MODULE)
#define dstream3 VS_LogEntry(3, DEBUG_CURRENT_MODULE)
#define dstream4 VS_LogEntry(4, DEBUG_CURRENT_MODULE)

#define dprint0(...) dstream0.printf(__VA_ARGS__)
#define dprint1(...) dstream1.printf(__VA_ARGS__)
#define dprint2(...) dstream2.printf(__VA_ARGS__)
#define dprint3(...) dstream3.printf(__VA_ARGS__)
#define dprint4(...) dstream4.printf(__VA_ARGS__)

#endif
