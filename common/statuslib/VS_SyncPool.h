#pragma once
#include "../std/cpplib/VS_Endpoint.h"
#include "std-generic/cpplib/VS_Container.h"

#include <mutex>
#include <map>
#include <chrono>
#include <cstdint>
#include <boost/heap/binomial_heap.hpp>

/// syncronization pool class


class VS_SyncPool
{
	class VS_SyncStruct
	{
	public:
		VS_SyncStruct(const int32_t seq_id, const bool reset_flag) : seq_id_(seq_id), reset_flag_(reset_flag), last_update_(std::chrono::steady_clock::now())
		{}
		VS_SyncStruct() :seq_id_(-1), reset_flag_(false)
		{}
		int32_t seq_id() const
		{
			return seq_id_;
		}
		void seq_id(const int32_t seq_id)
		{
			seq_id_ = seq_id;
			update_time();
		}
		bool reset_flag()const
		{
			return reset_flag_;
		}
		void reset_flag(const bool reset_flag)
		{
			reset_flag_ = reset_flag;
			update_time();
		}
		VS_SyncStruct& operator++()
		{
			++seq_id_;
			update_time();
			return *this;
		}
		const std::chrono::steady_clock::duration silence_duration() const
		{
			return std::chrono::steady_clock::now() - last_update_;
		}
		std::chrono::steady_clock::time_point last_update() const
		{
			return last_update_;
		}
	private:
		void update_time()
		{
			last_update_ = std::chrono::steady_clock::now();
		}

		int32_t seq_id_;
		bool reset_flag_;
		std::chrono::steady_clock::time_point last_update_;
	};

	const VS_SyncStruct &Update(const VS_FullID &id, const VS_SyncStruct& sync);


	using HeapItemType = std::pair<std::reference_wrapper<const VS_FullID>, std::chrono::steady_clock::time_point>;
	class min_timestamp
	{
	public:
		bool operator()(const HeapItemType&l, const HeapItemType&r) const
		{
			return l.second > r.second;
		}
	};
	using HeapType = boost::heap::binomial_heap<HeapItemType, boost::heap::compare<min_timestamp>>;

	HeapType m_queue_by_oldest_time_point;
	std::map<VS_FullID, std::pair<VS_SyncStruct,HeapType::handle_type>> m_pool;

	mutable std::mutex m_mutex;
public:
	static bool CheckExistenceParamsForSync(const VS_Container&cnt);

	int32_t Reset(const VS_FullID& out_ep);
	void Init(const VS_FullID& in_ep, int32_t seed);
	int32_t Inc(const VS_FullID& ep);
	void SetSeqId(const VS_FullID& ep, int32_t seed);
	int32_t GetSeqId(const VS_FullID& ep) const;
	void MarkForReset(const VS_FullID &ep);
	bool IsWaitForReset(const VS_FullID &ep) const;
};

class VS_InputSync
{
	VS_SyncPool in_sync_;
	std::map<VS_FullID, std::chrono::steady_clock::time_point /**expired time*/> waiting_for_reset_;
	static std::chrono::steady_clock::duration reset_interval_;

public:
	bool ConsistentCheck(const VS_FullID& src_ep, const VS_Container &in_cnt);
	auto GetCurrentSeqId(const VS_FullID& src) -> decltype(VS_SyncPool().GetSeqId(src));

};

