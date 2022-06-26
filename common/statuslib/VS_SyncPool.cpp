#include "VS_SyncPool.h"
#include "../std/cpplib/VS_Protocol.h"
#include "../std/debuglog/VS_Debug.h"


#define DEBUG_CURRENT_MODULE VS_DM_USPRS
std::chrono::steady_clock::duration VS_InputSync::reset_interval_ = std::chrono::minutes(2);

int32_t VS_SyncPool::Reset(const VS_FullID& out_ep)
{
	return Update(out_ep,VS_SyncStruct(1, false)).seq_id();
}

void VS_SyncPool::Init(const VS_FullID& in_ep, int32_t seed)
{
	Update(in_ep,VS_SyncStruct(seed, false));
}
int32_t VS_SyncPool::Inc(const VS_FullID& ep)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_pool.find(ep);
	if (it == m_pool.end())
		return -1;
	auto res = (++it->second.first).seq_id();
	m_queue_by_oldest_time_point.update(it->second.second, std::make_pair(std::cref(it->first), it->second.first.last_update()));
	return res;
}
void VS_SyncPool::SetSeqId(const VS_FullID& ep, int32_t seed)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_pool.find(ep);
	if (it != m_pool.end())
	{
		it->second.first.seq_id(seed);
		m_queue_by_oldest_time_point.update(it->second.second, std::make_pair(std::cref(it->first), it->second.first.last_update()));
	}
}
void VS_SyncPool::MarkForReset(const VS_FullID &ep)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_pool.find(ep);
	if (it != m_pool.end())
	{
		it->second.first.reset_flag(true);
		m_queue_by_oldest_time_point.update(it->second.second, std::make_pair(std::cref(it->first), it->second.first.last_update()));
	}
}
bool VS_SyncPool::IsWaitForReset(const VS_FullID &ep) const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it= m_pool.find(ep);
	if (it!=m_pool.end())
		return it->second.first.reset_flag();
	else
		return false;
}
int32_t VS_SyncPool::GetSeqId(const VS_FullID& ep) const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_pool.find(ep);
	return it == m_pool.end() ? -1 : it->second.first.seq_id();
}

bool VS_SyncPool::CheckExistenceParamsForSync(const VS_Container &cnt)
{
	int32_t seq_id(-1);
	if (!cnt.GetValue(SEQUENCE_ID_PARAM, seq_id))
		return false;
	return true;
}
const VS_SyncPool::VS_SyncStruct& VS_SyncPool::Update(const VS_FullID &id, const VS_SyncStruct &sync)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto iter = m_pool.find(id);
	if (m_pool.end() == iter)
	{
		iter = m_pool.emplace(id, std::make_pair(sync, HeapType::handle_type())).first;
		iter->second.second = m_queue_by_oldest_time_point.push(std::make_pair(std::cref(iter->first), iter->second.first.last_update()));
	}
	else
	{
		iter->second.first = sync;
		m_queue_by_oldest_time_point.update(iter->second.second, std::make_pair(std::cref(iter->first), iter->second.first.last_update()));
	}
	return iter->second.first;
}

bool VS_InputSync::ConsistentCheck(const VS_FullID& src_ep, const VS_Container &in_cnt)
{
	/**
	1. if reset thet reinit in_sync;
	*/
	int32_t seq_id = -1;
	if (!in_cnt.GetValue(SEQUENCE_ID_PARAM, seq_id))
		return false;
	int32_t reset = 0;
	in_cnt.GetValue(CAUSE_PARAM, reset);
	if (1 == reset)
	{
		dprint3("sync reset for FullID(server=%s,user=%s) init seq_id=%d\n",src_ep.m_serverID.IsEmpty()?"<nul>": src_ep.m_serverID.m_str,src_ep.m_userID.IsEmpty()?"<nul>": src_ep.m_userID.m_str,seq_id);
		in_sync_.Init(src_ep, seq_id);
		waiting_for_reset_.erase(src_ep);
		return true;
	}
	/**
	if we are waiting for reset then return true;
	*/
	auto it = waiting_for_reset_.find(src_ep);
	if (it != waiting_for_reset_.end())
	{
		auto now = std::chrono::steady_clock::now();
		if (it->second > now)
			return true;
		it->second = now + reset_interval_;
		dprint2("Waiting fo reset timeout FullID(server=%s,user=%s)\n", src_ep.m_serverID.m_str, src_ep.m_userID.m_str);
		return false;
	}
	auto test_seq = in_sync_.Inc(src_ep);
	if (test_seq != seq_id)
	{
		waiting_for_reset_.emplace(src_ep, std::chrono::steady_clock::now() + reset_interval_);
		dprint2("Sync lost: FullID(server=%s,user=%s)\n", src_ep.m_serverID.IsEmpty() ? "<null>":src_ep.m_serverID.m_str, src_ep.m_userID.IsEmpty() ? "<null>" : src_ep.m_userID.m_str);
		return false;
	}
	dprint4("InputSync for VS_FullID(server=%s,user=%s) is synchronized. Remote seq = %08x, local seq=%08x\n", src_ep.m_serverID.IsEmpty() ? "<null>" : src_ep.m_serverID.m_str, src_ep.m_userID.IsEmpty() ? "<null>" : src_ep.m_userID.m_str, seq_id, test_seq);
	return true;
}

auto VS_InputSync::GetCurrentSeqId(const VS_FullID &src) -> decltype(VS_SyncPool().GetSeqId(src))
{
	return in_sync_.GetSeqId(src);
}