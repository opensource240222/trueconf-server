#pragma once

#ifdef SERVER

#include "std/debuglog/VS_Debug.h"
namespace chat {
	class LogStream : public VS_LogEntry {
	public:
		LogStream() : VS_LogEntry(3, VS_DM_CHATS) {
		}
	};
}
#define CHAT_TRACE(...) (__VA_ARGS__)

#else

#include "Foundation/Library/VSLog.h"
#include <sstream>
namespace chat {
	class LogStream : public std::stringstream {
	public:
		LogStream() {
			imbue(std::locale::classic());
		}
		std::pair<bool, std::string> outcome() {
			return { true, str() };
		}
	};
}

#define CHAT_TRACE(...) do { \
	auto outcome = (__VA_ARGS__).outcome(); \
	if (outcome.first) \
		DTRACE(VSTM_CHAT, "| CHAT | %.*s", (int)outcome.second.length(), outcome.second.data()); \
} while (false)

#endif

#define log_stream LogStream{}

namespace chat {

#ifdef CHAT_LOG_VERBOSE
	using VerboseLogStream = LogStream;
#else
	struct VerboseLogStream {
		template<typename T>
		VerboseLogStream& operator<<(const T&) { return *this; }
		std::pair<bool, std::string> outcome() {
			return { false, {} };
		}
	};
#endif

}

#define verbose_log_stream VerboseLogStream{}