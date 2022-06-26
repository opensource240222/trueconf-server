#pragma once

#include <memory>
#include <ostream>
#include <type_traits>

enum VS_DebugModule : unsigned
{
	VS_DM_LOGS			= 0x00000040, // log utils
	VS_DM_CHATS			= 0x00000800,
	VS_DM_OTHER			= 0x10000000,
};

/****************************************************************************
 * Defines
 ****************************************************************************/
#if defined(DEBUG_CURRENT_MODULE)
#pragma message(__FILE__ ": DEBUG_CURRENT_MODULE is defined! Use this define only in *.cpp files!")
#endif

class VS_LogEntryStub
{
public:
	VS_LogEntryStub(unsigned /*level*/, VS_DebugModule /*module*/) {}
	// Move ctor is declared to enable this usage pattern:
	//     auto ds = VS_LogEntry(...)
	// It's left undefined because we don't actually want to move
	// (and also because move ctor is impossible to implement (std::ostream has protected move ctor)).
	VS_LogEntryStub(VS_LogEntryStub&&);

#if defined(__GNUC__)
	__attribute__ ((format (printf, 2, 3)))
#endif
	void printf(const char* /*format*/, ...) {}
	template <class T>
	VS_LogEntryStub& operator<<(const T& /*x*/)
	{
		return *this;
	}
};

#ifndef dstream0
#define dstream0 VS_LogEntryStub(0, DEBUG_CURRENT_MODULE)
#define dstream1 VS_LogEntryStub(1, DEBUG_CURRENT_MODULE)
#define dstream2 VS_LogEntryStub(2, DEBUG_CURRENT_MODULE)
#define dstream3 VS_LogEntryStub(3, DEBUG_CURRENT_MODULE)
#define dstream4 VS_LogEntryStub(4, DEBUG_CURRENT_MODULE)
#endif

#ifndef dprint0
#define dprint0(...) dstream0.printf(__VA_ARGS__)
#define dprint1(...) dstream1.printf(__VA_ARGS__)
#define dprint2(...) dstream2.printf(__VA_ARGS__)
#define dprint3(...) dstream3.printf(__VA_ARGS__)
#define dprint4(...) dstream4.printf(__VA_ARGS__)
#endif
