#include "FrameFilterLib/Base/TraceLog.h"
#include "FrameFilterLib/Base/Node.h"
#include "FrameFilterLib/Base/FrameMetadata.h"
#include "FrameFilterLib/Base/FilterFormat.h"
#include "FrameFilterLib/Base/FilterCommand.h"
#include "streams/Command.h"
#include "std/cpplib/ThreadUtils.h"
#include "std-generic/cpplib/SharedBuffer.h"
#include "std/cpplib/VS_MediaFormat_io.h"

#include <iomanip>
#include <thread>

namespace ffl {

TraceLog::TraceLog(std::streambuf* outbuf, unsigned flags)
	: m_flags(flags)
	, m_outbuf(outbuf)
	, m_log_start_time(std::chrono::steady_clock::now())
{
}

TraceLog::~TraceLog()
{
}

void TraceLog::PrintHeader(const Node* node, std::ostream& stream)
{
	auto flags = GetFlags();

	if (flags & PRINT_REL_TIME)
	{
		auto elapsed = std::chrono::steady_clock::now() - m_log_start_time;
		stream << std::right << std::setfill('0')
			<< std::setw(3) << std::chrono::duration_cast<std::chrono::hours>(elapsed).count() << ':'
			<< std::setw(2) << std::chrono::duration_cast<std::chrono::minutes>(elapsed).count() % 60 << ':'
			<< std::setw(2) << std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() % 60 << '.'
			<< std::setw(6) << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() % 1000000
		;
	}
	else
		stream << std::setfill(' ') << std::setw(15) << ""; // generate padding
	stream << '|';

	if (flags & PRINT_THREAD_ID)
		stream << std::right << std::setfill(' ') << std::setw(5) << vs::GetThreadID();
	else
		stream << std::setfill(' ') << std::setw(5) << ""; // generate padding
	stream << '|';

	stream << std::left << std::setfill(' ') << std::setw(Node::max_name_length) << node->Name() << '|';
	stream << std::right << std::setfill(' ') << std::setw(4) << node->ChainID() << '|';
}

void TraceLog::PrintCommand(const FilterCommand& cmd, std::ostream& stream)
{
	switch (cmd.type)
	{
	case FilterCommand::e_keyFrameRequest:
		stream << "key frame request";
		break;
	case FilterCommand::e_changeFormatRequest:
		stream << "change format request: mf: " << cmd.mf;
		break;
	case FilterCommand::e_setBitrateRequest:
		stream << "set bitrate request: bitrate: " << cmd.bitrate;
		break;
	default:
		stream << "invalid";
	}
}

void TraceLog::TraceBuffer(const Node* node, const vs::SharedBuffer& buffer, const FrameMetadata& md)
{
	auto flags = GetFlags();
	if (!(flags & TRACE_BUFFERS))
		return;

	std::lock_guard<std::mutex> lock(m_mutex);
	std::ostream stream(m_outbuf);
	PrintHeader(node, stream);
	stream << "buf| " << buffer.size() << "@0x" << buffer.data();
	switch (md.track)
	{
	case stream::Track::audio:
		stream << ", audio: interval=" << md.interval;
		break;
	case stream::Track::video:
		stream << ", video: interval=" << md.interval;
		if (md.keyframe)
			stream << ", key";
		break;
	case stream::Track::command:
	{
		stream << ", command: ";
		const auto& cmd = *buffer.data<const stream::Command>();
		if (buffer.size() >= stream::Command::header_size && buffer.size() == cmd.Size())
		{
			stream << "stream::Command: type=" << (int)cmd.type << ", sub_type=" << (int)cmd.sub_type << ", result=" << (int)cmd.result;
			if (cmd.type == stream::Command::Type::ChangeSndMFormat || cmd.type == stream::Command::Type::ChangeRcvMFormat)
				stream << ", mf=\"" << *reinterpret_cast<const VS_MediaFormat*>(cmd.data) << "\"";
		}
		else
			stream << "unknown";
		break;
	}
	default:
		stream << ", unknown type: interval=" << md.interval;
		if (md.keyframe)
			stream << ", key";
	}
	stream << '\n';
}

void TraceLog::TraceFormat(const Node* node, const FilterFormat& format)
{
	auto flags = GetFlags();
	if (!(flags & TRACE_FORMATS))
		return;

	std::lock_guard<std::mutex> lock(m_mutex);
	std::ostream stream(m_outbuf);
	PrintHeader(node, stream);
	stream << "fmt| ";
	switch (format.type)
	{
	case FilterFormat::e_invalid:
		stream << "invalid";
		break;
	case FilterFormat::e_mf:
		stream << "mf: " << format.mf;
		break;
	case FilterFormat::e_rtp:
		stream << "rtp";
		break;
	default:
		stream << "unknown";
	}
	stream << '\n';
}

void TraceLog::TraceCommand(const Node* node, const FilterCommand& cmd)
{
	auto flags = GetFlags();
	if (!(flags & TRACE_COMMANDS))
		return;

	std::lock_guard<std::mutex> lock(m_mutex);
	std::ostream stream(m_outbuf);
	PrintHeader(node, stream);
	stream << "cmd| ";
	PrintCommand(cmd, stream);
	stream << '\n';
}

void TraceLog::TraceDeletion(const Node* node)
{
	auto flags = GetFlags();
	if (!(flags & TRACE_DELETION))
		return;

	std::lock_guard<std::mutex> lock(m_mutex);
	std::ostream stream(m_outbuf);
	PrintHeader(node, stream);
	stream << "del|";
	stream << '\n';
}

void TraceLog::TraceMessage(const Node* node, string_view msg)
{
	auto flags = GetFlags();
	if (!(flags & TRACE_MESSAGES))
		return;

	std::lock_guard<std::mutex> lock(m_mutex);
	std::ostream stream(m_outbuf);
	PrintHeader(node, stream);
	stream << "msg| ";
	stream << msg;
	stream << '\n';
}

void TraceLog::TraceMessage(const Node* node, string_view msg, const FilterCommand& cmd)
{
	auto flags = GetFlags();
	if (!(flags & TRACE_MESSAGES))
		return;

	std::lock_guard<std::mutex> lock(m_mutex);
	std::ostream stream(m_outbuf);
	PrintHeader(node, stream);
	stream << "msg| ";
	stream << msg;
	PrintCommand(cmd, stream);
	stream << '\n';

}

}
