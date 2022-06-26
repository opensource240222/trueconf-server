#pragma once

#include "std-generic/cpplib/string_view.h"

#include <atomic>
#include <chrono>
#include <map>

class VS_MessageFloodBlock
{
public:
	explicit VS_MessageFloodBlock(string_view instance_name);
	~VS_MessageFloodBlock();

	bool Add(string_view source, std::chrono::steady_clock::time_point time);
	void UpdateState(std::chrono::steady_clock::time_point time);

private:
	static void ReadParameters();

	std::string m_instance_name;
	struct source_info
	{
		std::chrono::steady_clock::time_point first_time = {};
		std::chrono::steady_clock::time_point last_time = {};
		unsigned total_msg = 0;
		bool blocked = false;
	};
	std::map<std::string, source_info> m_sources;;
	std::chrono::steady_clock::time_point m_last_update_time;

	static std::atomic<unsigned> s_block_threshold; // msg/sec
	static std::atomic<unsigned> s_unblock_threshold; // msg/sec
	static std::atomic<unsigned> s_remove_timeout_s; // sec

	static std::atomic_flag s_read_in_progress;
	static std::chrono::steady_clock::time_point s_last_read_time;
};