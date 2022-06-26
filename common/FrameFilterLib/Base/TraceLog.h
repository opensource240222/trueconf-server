#pragma once

#include "std-generic/cpplib/string_view.h"

#include <atomic>
#include <chrono>
#include <mutex>
#include <ostream>

namespace vs
{
class SharedBuffer;
}

namespace ffl {

class Node;
struct FrameMetadata;
struct FilterFormat;
struct FilterCommand;

class TraceLog
{
public:
	enum Flags : unsigned
	{
		TRACE_BUFFERS   = 0x00000001,
		TRACE_FORMATS   = 0x00000002,
		TRACE_COMMANDS  = 0x00000004,
		TRACE_DELETION  = 0x00000008,
		PRINT_THREAD_ID = 0x00000010,
		PRINT_REL_TIME  = 0x00000020,
		TRACE_MESSAGES  = 0x00000100,
		TRACE_ALL       = TRACE_BUFFERS | TRACE_FORMATS | TRACE_COMMANDS | TRACE_DELETION | TRACE_MESSAGES,
		PRINT_ALL       = PRINT_THREAD_ID | PRINT_REL_TIME,
	};

	TraceLog(std::streambuf* outbuf, unsigned flags = TRACE_ALL | PRINT_ALL);
	~TraceLog();

	void SetFlags(unsigned flags) { m_flags.store(flags); }
	unsigned GetFlags() { return m_flags.load(); }

	void TraceBuffer(const Node* node, const vs::SharedBuffer& buffer, const FrameMetadata& md);
	void TraceFormat(const Node* node, const FilterFormat& format);
	void TraceCommand(const Node* node, const FilterCommand& cmd);
	void TraceDeletion(const Node* node);
	void TraceMessage(const Node* node, string_view msg);
	void TraceMessage(const Node* node, string_view msg, const FilterCommand& cmd);

private:
	void PrintHeader(const Node* node, std::ostream& stream);
	static void PrintCommand(const FilterCommand& cmd, std::ostream& stream);

	std::atomic<unsigned> m_flags;
	std::mutex m_mutex;
	std::streambuf* m_outbuf;
	std::chrono::steady_clock::time_point m_log_start_time;
};

}
