#include "VS_MessageFloodBlock.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"
#include "std-generic/cpplib/scope_exit.h"
#include "../../std/debuglog/VS_Debug.h"

#include <algorithm>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

std::atomic<unsigned> VS_MessageFloodBlock::s_block_threshold(20); // msg/sec
std::atomic<unsigned> VS_MessageFloodBlock::s_unblock_threshold(5); // msg/sec
std::atomic<unsigned> VS_MessageFloodBlock::s_remove_timeout_s(60); // sec
std::atomic_flag VS_MessageFloodBlock::s_read_in_progress = ATOMIC_FLAG_INIT;
std::chrono::steady_clock::time_point VS_MessageFloodBlock::s_last_read_time = {};

VS_MessageFloodBlock::VS_MessageFloodBlock(string_view instance_name)
	: m_instance_name(instance_name)
	, m_last_update_time()
{
}

VS_MessageFloodBlock::~VS_MessageFloodBlock() = default;

bool VS_MessageFloodBlock::Add(string_view source, std::chrono::steady_clock::time_point time)
{
	auto& info = m_sources[std::string(source)];

	if (info.first_time == std::chrono::steady_clock::time_point())
		info.first_time = time;
	++info.total_msg;
	info.last_time = time;

	return info.blocked;
}

void VS_MessageFloodBlock::UpdateState(std::chrono::steady_clock::time_point time)
{
	if (time - m_last_update_time < std::chrono::seconds(1))
		return;
	m_last_update_time = time;

	ReadParameters();
	const auto block_threshold = s_block_threshold.load(std::memory_order_relaxed);
	const auto unblock_threshold = s_unblock_threshold.load(std::memory_order_relaxed);
	const auto remove_timeout = std::chrono::seconds(s_remove_timeout_s.load(std::memory_order_relaxed));

	for (auto it = m_sources.begin(); it != m_sources.end(); )
	{
		if (time - it->second.last_time > remove_timeout)
		{
			dstream4 << "BLOCK: " << m_instance_name << " | Remove " << it->first << " | Total is " << m_sources.size();
			it = m_sources.erase(it);
			continue;
		}

		const unsigned active_time_s = std::max<unsigned>(1, std::chrono::duration_cast<std::chrono::seconds>(time - it->second.first_time).count());
		const unsigned speed = it->second.total_msg / active_time_s;
		if (speed > block_threshold)
		{
			if (!it->second.blocked)
				dstream1 << "BLOCK: " << m_instance_name << " | Block " << it->first << ", time=" << active_time_s << ", N=" << it->second.total_msg;
			it->second.blocked = true;
		}
		else if (speed < unblock_threshold)
		{
			if (it->second.blocked)
				dstream1 << "BLOCK: " << m_instance_name << " | Unblock " << it->first << ", time=" << active_time_s << ", N=" << it->second.total_msg;
			it->second.blocked = false;
		}

		++it;
	}
}

void VS_MessageFloodBlock::ReadParameters()
{
	if (s_read_in_progress.test_and_set(std::memory_order_acq_rel))
		return; // Function is already running in some other thread
	VS_SCOPE_EXIT { s_read_in_progress.clear(std::memory_order_release); };

	const auto now = std::chrono::steady_clock::now();
	if (now - s_last_read_time < std::chrono::seconds(2))
		return;
	s_last_read_time = now;

	int32_t value;
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	if (key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "CidBlockTh") > 0)
		s_block_threshold.store(value, std::memory_order_relaxed);
	if (key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "CidUnBlockTh") > 0)
		s_unblock_threshold.store(value, std::memory_order_relaxed);
	if (key.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "CidTimeRemove") > 0)
		s_remove_timeout_s.store(value, std::memory_order_relaxed);
}
