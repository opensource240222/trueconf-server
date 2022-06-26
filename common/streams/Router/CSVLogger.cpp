#include "CSVLogger.h"
#include "../../std/cpplib/VS_Utils.h"

#include <iterator>

namespace stream {

const char c_streams_logs_directory_name[] = "streams_logs";

CSVLog::CSVLog()
	: m_data_pos(-1)
{
}

CSVLog::~CSVLog()
{
	Close();
}

bool CSVLog::Open(string_view work_dir, string_view conf_name)
{
	if (m_file.is_open())
		return false;

	m_file_name.clear();
	m_file_name.reserve(work_dir.size() + 2/* "/C" */ + conf_name.size() + 4/* ".csv" */);
	m_file_name += work_dir;
	m_file_name += "/C";
	VS_FilterPath(conf_name.begin(), conf_name.end(), std::back_inserter(m_file_name));
	m_file_name += ".csv";

	m_file.open(m_file_name.c_str(), std::ios::out | std::ios::trunc);
	if (!m_file)
		return false;

	m_file << "Participant,Direction,Track,Size,Offset.ms,Jitter.ms\n";
	m_data_pos = m_file.tellp();

	return true;
}

bool CSVLog::Open(string_view work_dir, string_view conf_name, string_view part_name)
{
	if (m_file.is_open())
		return false;

	m_file_name.clear();
	m_file_name.reserve(work_dir.size() + 2/* "/C" */ + conf_name.size() + 1/* "P" */ + part_name.size() + 4/* ".csv" */);
	m_file_name += work_dir;
	m_file_name += "/C";
	VS_FilterPath(conf_name.begin(), conf_name.end(), std::back_inserter(m_file_name));
	m_file_name += "P";
	VS_FilterPath(part_name.begin(), part_name.end(), std::back_inserter(m_file_name));
	m_file_name += ".csv";

	m_file.open(m_file_name.c_str(), std::ios::out | std::ios::trunc);
	if (!m_file)
		return false;

	m_file << "Direction,Track,Size,Offset.ms,Jitter.ms\n";
	m_data_pos = m_file.tellp();

	return true;
}

void CSVLog::Close()
{
	if (!m_file.is_open())
		return;

	const auto pos = m_file.tellp();
	m_file.close();

	if (pos <= m_data_pos)
		::remove(m_file_name.c_str());
}

CSVLogger::CSVLogger()
	: m_start_time(std::chrono::steady_clock::now())
	, m_in_jitter_correction(0)
	, m_out_jitter_correction(0)
{
}

void CSVLogger::LogFrameRead(const stream::FrameHeader& header, const char* part_name, CSVLog* part_log, CSVLog* conf_log)
{
	if (part_log && !part_log->IsOpen())
		part_log = nullptr;
	if (conf_log && !conf_log->IsOpen())
		conf_log = nullptr;
	if (!part_log && !conf_log)
		return;

	const auto relative_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start_time).count();
	if (part_log)
		*part_log << "IN," << static_cast<unsigned>(header.track) << ',' << header.length << ',' << relative_time_ms;
	if (conf_log)
		*conf_log << part_name << ",IN," << static_cast<unsigned>(header.track) << ',' << header.length << ',' << relative_time_ms;

	int jitter = static_cast<unsigned>(relative_time_ms) - static_cast<unsigned>(header.tick_count);
	if (m_in_jitter_correction == 0)
	{
		m_in_jitter_correction = jitter;
		jitter = 0;
	}
	else
	{
		jitter -= m_in_jitter_correction;

		if (part_log)
			*part_log << ',' << jitter;
		if (conf_log)
			*conf_log << ',' << jitter;
	}

	if (part_log)
		*part_log << '\n';
	if (conf_log)
		*conf_log << '\n';
}

void CSVLogger::LogFrameWrite(const stream::FrameHeader& header, const char* part_name, CSVLog* part_log, CSVLog* conf_log)
{
	if (part_log && !part_log->IsOpen())
		part_log = nullptr;
	if (conf_log && !conf_log->IsOpen())
		conf_log = nullptr;
	if (!part_log && !conf_log)
		return;

	const auto relative_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start_time).count();
	if (part_log)
		*part_log << "OUT," << static_cast<unsigned>(header.track) << ',' << header.length << ',' << relative_time_ms;
	if (conf_log)
		*conf_log << part_name << ",OUT," << static_cast<unsigned>(header.track) << ',' << header.length << ',' << relative_time_ms;

	int jitter = static_cast<unsigned>(relative_time_ms) - static_cast<unsigned>(header.tick_count);
	if (m_out_jitter_correction == 0)
	{
		m_out_jitter_correction = jitter;
		jitter = 0;
	}
	else
	{
		jitter -= m_out_jitter_correction;

		if (part_log)
			*part_log << ',' << jitter;
		if (conf_log)
			*conf_log << ',' << jitter;
	}

	if (part_log)
		*part_log << '\n';
	if (conf_log)
		*conf_log << '\n';
}

}

