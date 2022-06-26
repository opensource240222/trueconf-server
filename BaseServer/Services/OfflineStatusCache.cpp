#include "OfflineStatusCache.h"
#include "storage/VS_LocatorStorage.h"
#include "storage/VS_DBStorageInterface.h"
#include "std/cpplib/ThreadUtils.h"
#include "../../common/std/debuglog/VS_Debug.h"
#include <chrono>

#define DEBUG_CURRENT_MODULE VS_DM_USPRS

#if defined(_MSC_VER)
#pragma warning(disable: 4503)
#endif

static OfflineStatusCache::WorkMode global_work_mode = OfflineStatusCache::WorkMode::async;
namespace
{
static const auto s_timeout = std::chrono::minutes(1);
const static std::chrono::steady_clock::duration defaut_expiration_period = std::chrono::hours(24);
}

void OfflineStatusCache::process_request_visitor::operator()(const LocateRequestT &req) const
{
	req_forward_->ProcessRequest(req);
}
void OfflineStatusCache::process_request_visitor::operator()(const ReadToCacheRequestT &req) const
{
	req_forward_->ProcessRequest(req);
}
void OfflineStatusCache::process_request_visitor::operator()(const UpdateStatusRequestT &req) const
{
	req_forward_->ProcessRequest(req);
}
OfflineStatusCache::OfflineStatusCache(const OnExtStatusUpdated& cb)
	: m_condition_changed(false)
	, m_stop(false)
	, m_mode(global_work_mode)
	, m_on_ext_status_updated(cb)
	, m_request_id(0)
{
	if (m_mode == WorkMode::async)
		m_process_thread = std::thread([this] () {
			vs::SetThreadName("OfflineStatus");
			ProcessingThread();
		});
}
OfflineStatusCache::~OfflineStatusCache()
{
	StopThread();
}

void OfflineStatusCache::SetWorkMode(const WorkMode &mode)
{
	global_work_mode = mode;
}

void OfflineStatusCache::StopThread()
{
	if (WorkMode::sync == m_mode)
		return;
	try
	{
		m_stop = true;
		Notify();
		m_process_thread.join();
	}
	catch (const std::system_error &)
	{}
	catch (...)
	{
		throw;
	}
}
void OfflineStatusCache::ProcessingThread()
{
	assert(m_mode == WorkMode::async);
	std::remove_reference<decltype(*m_req_queue.lock())>::type temp_req_queue;
	bool timeout_rise(false);
	std::chrono::steady_clock::time_point last_timeout = m_clock.now();;
	while (!m_stop || !temp_req_queue.empty() || !m_req_queue->empty())
	{
		if (!temp_req_queue.empty())
		{
			auto req = std::move(temp_req_queue.front());
			temp_req_queue.pop();
			dstream4 << "OfflineStatusCache: processing request id = " << std::hex << std::get<0>(req);
			auto processing_delay = std::chrono::steady_clock::now() - std::get<1>(req);
			if (std::chrono::seconds(1) < processing_delay)
			{
				dstream4 << "OfflineStatusCache: Processing request was delayed for " << std::chrono::duration_cast<std::chrono::seconds>(processing_delay).count() << " sec.\n";
			}
			ProcessRequest(std::get<2>(req));
			continue;
		}
		if (timeout_rise || m_clock.now() - last_timeout >= s_timeout)
		{
			UpdateExtStatusesFromDB();
			last_timeout = m_clock.now();
		}
		{
			std::unique_lock<std::mutex> l(m_lock);
			timeout_rise = !m_cond_var.wait_for(l, s_timeout, [this]() {return m_condition_changed == true; });
			m_condition_changed = false;
		}
		temp_req_queue = std::move(*m_req_queue.lock());
	}
}
void OfflineStatusCache::ProcessRequest(const RequestType & req)
{
	boost::apply_visitor(process_request_visitor(this), req);
}
void OfflineStatusCache::ProcessRequest(const LocateRequestT &req)
{
	auto processing_start = std::chrono::steady_clock::now();
	std::map<std::string, std::set<std::string>> location;
	{
		dstream4 << "OfflineStatusCache: processing locate request. Number of requests = " << req.first.size();
		auto ds = dstream4;
		ds << "OfflineStatusCache: locate result:";
		auto &storage = VS_LocatorStorage::Instance();
		for (auto &i : req.first)
		{
			if (m_stop)
			{
				dstream4 << "OfflineStatusCache: It is stopped. Skip remained locate requests\n";
				break;
			}
			std::string locatorBS;
			//Check cache
			auto info = GetOfflineStatus(i,true);
			if (info.second && !info.first.locatorBS_.empty())
				locatorBS = info.first.locatorBS_;
			else
			{
				storage.GetServerByCallID(i, locatorBS);
				if (locatorBS.empty())
					EraseStatus(i);
				else
				{
					info.first.locatorBS_ = locatorBS;
					UpdateStatus(i, std::move(info.first));
				}
			}
			ds << " call_id = " << i << ", "<< "locatorBS is " << (locatorBS.empty() ? "empty" : locatorBS) << ";";
			location[locatorBS].insert(i);
		}
		ds << '\n';
	}
	auto processing_duration = std::chrono::steady_clock::now() - processing_start;
	if (processing_duration > std::chrono::seconds(1))
		dstream4 << "OfflineStatusCache: processing request duration = " << std::chrono::duration_cast<std::chrono::seconds>(processing_duration).count() << "sec.\n";
	req.second(std::move(location)); // emit callback
}
void OfflineStatusCache::ProcessRequest(const ReadToCacheRequestT & req)
{
	auto processing_start = std::chrono::steady_clock::now();
	{
		dstream4 << "OfflineStatusCache: processing ReadToCacheRequest. Number of requests = " << std::get<0>(req).size();
		auto dbStorage = g_dbStorage;
		for (auto &i : std::get<0>(req))
		{
			if (m_stop)
			{
				dstream4 << "OfflineStatusCache: It is stopped. Skip remained ReadToCache requests\n";
				break;
			}
			dstream4 << "call_id = " << i;
			auto info = GetOfflineStatus(i);
			if (info.second && !!info.first.ext_status_)
				continue;
			if (dbStorage)
			{
				dbStorage->GetExtendedStatus(i.c_str(), info.first.ext_status_);
				UpdateStatus(i, std::move(info.first));
			}
		}
	}
	auto processing_duration = std::chrono::steady_clock::now() - processing_start;
	if (processing_duration > std::chrono::seconds(1))
		dstream4 << "OfflineStatusCache: processing request duration = " << std::chrono::duration_cast<std::chrono::seconds>(processing_duration).count() << "sec.\n";
	std::get<1>(req)();
}
void OfflineStatusCache::ProcessRequest(const UpdateStatusRequestT& req)
{
	auto processing_start = std::chrono::steady_clock::now();
	if (m_stop)
		dstream4 << "OfflineStatusCache: It is stopped. Skip remained UpdateStatus requests\n";
	else
	{
		dstream4 << "OfflineStatusCache: processing UpdateStatus. "<< "call_id = " << req.call_id << ";\n";
		auto dbStorage = g_dbStorage;
		if (dbStorage)
			dbStorage->SetUserStatus(req.call_id.c_str(), req.presence_staus, req.ext_status, req.set_server, req.source_server.c_str());
		auto info = GetOfflineStatus(req.call_id);
		info.first.ext_status_ += req.ext_status;
		UpdateStatus(req.call_id, std::move(info.first));
	}
	auto processing_duration = std::chrono::steady_clock::now() - processing_start;
	if (processing_duration > std::chrono::seconds(1))
		dstream4 << "OfflineStatusCache: processing request duration = " << std::chrono::duration_cast<std::chrono::seconds>(processing_duration).count() << "sec.\n";
	req.cb();
}
void OfflineStatusCache::Locate(std::list<std::string> && call_id_for_locate, CallBackFunc &&call_back)
{
	if(m_stop)
	{
		call_back(std::map<std::string,std::set<std::string>>());
		return;
	}
	auto req = std::make_tuple(++m_request_id,std::chrono::steady_clock::now(),std::make_pair(std::move(call_id_for_locate), std::move(call_back)));
	dstream4 << "OfflineStatusCache: Locate req; request id = " << std::hex << std::get<0>(req);
	if (m_mode == WorkMode::async)
	{
		m_req_queue->push(std::move(req));
		Notify();
	}
	else
		ProcessRequest(std::get<2>(req));
}
void OfflineStatusCache::ReadStickyToCache(std::list<std::string> &&ids, std::function<void(void)> &&cb)
{
	if (ids.empty())
	{
		cb();
		return;
	}
	auto req = std::make_tuple(++m_request_id, std::chrono::steady_clock::now(), std::make_tuple(std::move(ids), std::move(cb)));
	dstream4 << "OfflineStatusCache: ReadStickyToCache req; request id = " << std::hex << std::get<0>(req);
	if (m_mode == WorkMode::async)
	{
		m_req_queue->push(std::move(req));
		Notify();
	}
	else
		ProcessRequest(std::get<2>(req));
}
void OfflineStatusCache::UpdateUserStatus(std::string &&call_id, VS_UserPresence_Status presence_status, VS_ExtendedStatusStorage &&ext_status, bool set_server, std::string &&source_server, std::function<void(void) > cb)
{
	if(m_stop)
	{
		cb();
		return;
	}
	auto req = std::make_tuple(
		++m_request_id,
		std::chrono::steady_clock::now(),
		UpdateStatusRequestT
		{
			std::move(call_id),
			presence_status,
			std::move(ext_status),
			set_server,
			std::move(source_server),
			std::move(cb)
		});
	dstream4 << "OfflineStatusCache: UpdateUserStatus req; request id = " << std::hex << std::get<0>(req);
	if (m_mode == WorkMode::async)
	{
		m_req_queue->push(std::move(req));
		Notify();
	}
	else
		ProcessRequest(std::get<2>(req));
}
std::pair<OfflineStatusCache::CacheItemT,bool> OfflineStatusCache::GetOfflineStatus(const std::string& call_id, const bool skip_expired)
{
	dstream4 << "OfflineStatusCache: GetOfflineStatus for callid = " << call_id << "; skip_expired = " << skip_expired;
	std::lock_guard<std::mutex> l(m_lock);
	auto it = m_offline_status_cache.find(call_id);
	if (bool not_found = it == m_offline_status_cache.end() || (skip_expired && it->second.second < m_clock.now()))
	{
		dstream4 << (not_found ? "status is absent in cache" : "status is expired");
		return std::make_pair(CacheItemT(), false);
	}
	dstream4 << "status found in cache\n";
	return std::make_pair(it->second.first,true);
}
void OfflineStatusCache::UpdateStatus(const std::string& call_id, CacheItemT&& new_status)
{
	dstream4 << "OfflineStatusCache: update status in cache for call_id = " << call_id;
	std::lock_guard<std::mutex> l(m_lock);
	m_offline_status_cache[call_id] = std::make_pair(std::move(new_status), m_clock.now() + defaut_expiration_period);
}
void OfflineStatusCache::EraseStatus(const std::string& call_id)
{
	dstream4 << "OfflineStatusCache: erase status form cache for call_id = " << call_id;
	std::lock_guard<std::mutex> l(m_lock);
	m_offline_status_cache.erase(call_id);
}
void OfflineStatusCache::Notify() const
{
	{
		std::lock_guard<std::mutex> l(m_lock);
		m_condition_changed = true;
	}
	m_cond_var.notify_one();
}
void OfflineStatusCache::UpdateExtStatusesFromDB()
{
	dprint4("OfflineStatusCache: UpdateExtStatusesFromDB by timeout\n");
	auto dbStorage = g_dbStorage;
	if (dbStorage)
	{
		std::map<std::string, VS_ExtendedStatusStorage> res;
		dbStorage->GetUpdatedExtStatuses(res);
		if (res.empty())
			return;
		dstream4 << "count of updated stauses = " << res.size() << "; updateing and notify\n";
		ExtStatusUpdateInfo updated_status;
		updated_status.reserve(res.size());
		for(auto& i:res)
		{
			auto info = GetOfflineStatus(i.first);
			info.first.ext_status_ += i.second;
			updated_status.emplace_back(i.first, info.first.ext_status_);
			UpdateStatus(i.first, std::move(info.first));
		}
		m_on_ext_status_updated(std::move(updated_status));
	}
}