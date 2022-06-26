#pragma once

#include "../Protocol.h"
#include "std-generic/cpplib/string_view.h"

#include <chrono>
#include <fstream>
#include <string>

namespace stream {

extern const char c_streams_logs_directory_name[]; // "streams_logs";

class CSVLog
{
public:
	CSVLog();
	~CSVLog();

	// Open log file for conference, write header.
	bool Open(string_view work_dir, string_view conf_name);
	// Open log file for participant, write header.
	bool Open(string_view work_dir, string_view conf_name, string_view part_name);
	// Close log file, delete it if contains no data.
	void Close();

	bool IsOpen() const
	{
		return m_file.is_open();
	}

	template <class T>
	CSVLog& operator<<(const T& x)
	{
		m_file << x;
		return *this;
	}

private:
	std::ofstream m_file;
	std::string m_file_name;
	std::ofstream::pos_type m_data_pos;
};

class CSVLogger
{
public:
	CSVLogger();

	void LogFrameRead(const stream::FrameHeader& header, const char* part_name, CSVLog* part_log, CSVLog* conf_log);
	void LogFrameWrite(const stream::FrameHeader& header, const char* part_name, CSVLog* part_log, CSVLog* conf_log);

private:
	std::chrono::steady_clock::time_point m_start_time;
	int m_in_jitter_correction; // Used to normalize jitter values to start from 0.
	int m_out_jitter_correction; // Used to normalize jitter values to start from 0.
};


}
