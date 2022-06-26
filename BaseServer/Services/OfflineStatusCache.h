#pragma once
#include "std-generic/cpplib/string_view.h"
#include "../../common/statuslib/status_types.h"
#include "std-generic/cpplib/synchronized.h"
#include "std-generic/cpplib/VS_ClockWrapper.h"
#include "std-generic/compat/condition_variable.h"

#include <boost/variant.hpp>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include <queue>
#include <list>
#include <string>
#include <set>


class VS_ExtendedStatusStorage;
class OfflineStatusCache
{
	/**
		locatorBS;
		ExtStatusStorage
	*/
public:
	using ExtStatusUpdateInfo = std::vector<std::pair<std::string, VS_ExtendedStatusStorage>>;
private:
	using CacheItemT = struct { std::string locatorBS_; VS_ExtendedStatusStorage ext_status_; };
	using CallBackFunc = std::function<void(std::map<std::string, std::set<std::string>> &&)>;
	using LocateRequestT = std::pair<std::list<std::string>, CallBackFunc>;
	using ReadToCacheRequestT = std::tuple<std::list<std::string>, std::function<void(void)>>;
	using UpdateStatusRequestT = struct { std::string call_id; VS_UserPresence_Status presence_staus; VS_ExtendedStatusStorage ext_status; bool set_server; std::string source_server; std::function<void(void)>cb; };
	using RequestType = boost::variant<LocateRequestT, ReadToCacheRequestT, UpdateStatusRequestT>;
	using OnExtStatusUpdated = std::function<void(ExtStatusUpdateInfo&&)>;
	class process_request_visitor :public boost::static_visitor<>
	{
		OfflineStatusCache *req_forward_;
	public:
		process_request_visitor(OfflineStatusCache *forward) :req_forward_(forward) {}
		void operator()(const LocateRequestT &) const;
		void operator()(const ReadToCacheRequestT &) const;
		void operator()(const UpdateStatusRequestT&) const;
	};
public:
	enum class WorkMode
	{
		sync,
		async
	};
	static void SetWorkMode(const WorkMode& mode);
	OfflineStatusCache(const OnExtStatusUpdated&);
	~OfflineStatusCache();
	void Locate(std::list<std::string> && call_id_for_locate, CallBackFunc &&call_back);
	void ReadStickyToCache(std::list<std::string> &&call_ids, std::function<void()> &&call_back);
	void StopThread();
	std::pair<CacheItemT,bool> GetOfflineStatus(const std::string& call_id,const bool skip_expired = false);
	void UpdateUserStatus(std::string &&call_id, VS_UserPresence_Status presence_status, VS_ExtendedStatusStorage &&ext_status, bool set_server, std::string &&source_server, std::function<void (void) > cb);
	void UpdateExtStatusesFromDB();
	void Notify() const;
	steady_clock_wrapper& clock() { return m_clock; }
private:
	void ProcessingThread();
	void UpdateStatus(const std::string& call_id, CacheItemT&& new_status);
	void EraseStatus(const std::string& call_id);
	void ProcessRequest(const LocateRequestT & req);
	void ProcessRequest(const ReadToCacheRequestT &req);
	void ProcessRequest(const UpdateStatusRequestT& req);
	void ProcessRequest(const RequestType &req);

	mutable std::mutex	m_lock;
	mutable vs::condition_variable m_cond_var;
	mutable std::atomic<bool> m_condition_changed;
	std::atomic<bool> m_stop;
	const WorkMode m_mode;
	std::thread	m_process_thread;
	vs::Synchronized<std::queue<std::tuple<uint64_t,std::chrono::steady_clock::time_point,RequestType>>> m_req_queue;

	std::map < std::string, std::pair < CacheItemT, std::chrono::steady_clock::time_point> > m_offline_status_cache; // offline CacheItem -> expiration time
	steady_clock_wrapper	m_clock;
	OnExtStatusUpdated		m_on_ext_status_updated;
	std::atomic<uint64_t>	m_request_id;
};