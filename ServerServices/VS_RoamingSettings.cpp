#include "VS_RoamingSettings.h"
#include "../common/std/cpplib/VS_Replace.h"
#include "../common/std/debuglog/VS_Debug.h"
#include "net/DNSUtils/VS_DNSUtils.h"
#include "std-generic/clib/strcasecmp.h"

#include <sstream>
#include <set>
#include "net/DNSUtils/VS_DNS.h"

#define DEBUG_CURRENT_MODULE VS_DM_RESOLVE

static const auto c_resolve_interval = std::chrono::minutes(15);
static const auto c_min_resolve_interval = std::chrono::minutes(1);

VS_RoamingSettings::VS_RoamingSettings(boost::asio::io_service& ios)
	: m_ios(ios)
	, m_timer(m_ios)
	, m_should_run(false)
	, m_roaming_mode(RM_INVALID)
{
}

void VS_RoamingSettings::Start()
{
	if (m_should_run.exchange(true, std::memory_order_acq_rel) == true)
		return; // Already started
	ScheduleTimer(c_min_resolve_interval);
}

void VS_RoamingSettings::Stop()
{
	m_should_run.store(false, std::memory_order_release);
	m_timer.cancel();
}

void VS_RoamingSettings::SetRoamingSettings(const eRoamingMode_t mode, const std::string& params)
{
	std::lock_guard<std::mutex> lock(m_lock);
	m_roaming_mode = mode;
	dprint3("RoamingSettings: mode=%d params='%s'\n", mode, params.c_str());
	if (mode == RM_DISABLED) {

	}
	else if ((mode == RM_WHITELIST) ||
			 (mode == RM_BLACKLIST)) {
		std::string p = params;
		VS_ReplaceAll(p, ",", " ");
		std::stringstream iss;
		iss << p;
		std::vector<std::string> tokens{ std::istream_iterator < std::string > {iss}, std::istream_iterator < std::string > {} };

		for (auto& i : tokens)
		{
			std::size_t pos = i.find_first_of("#");
			if (pos != std::string::npos)
				i.erase(pos);

			if (i.find_first_of("*") != std::string::npos)
			{
				VS_ReplaceAll(i, ".", "\\.");
				VS_ReplaceAll(i, "-", "\\-");
				VS_ReplaceAll(i, "*", ".*?");
				m_e.emplace_back(i, boost::regex::icase);
			}
			else{
				m_sid_with_domain[i] = i;
			}
		}
	}
}

eRoamingMode_t VS_RoamingSettings::RoamingMode()
{
	std::lock_guard<std::mutex> lock(m_lock);
	return m_roaming_mode;
}

bool VS_RoamingSettings::IsRoamingAllowed(const char *for_server_name)
{
	eRoamingMode_t mode;
	{
		std::lock_guard<std::mutex> lock(m_lock);
		mode = m_roaming_mode;
	}
	bool res(false);
	if (mode == RM_DISABLED) {
		res = false;
	}
	else if (mode == RM_WHITELIST){
		res = (IsFound(for_server_name)) ? true : false;
	}
	else if (mode == RM_BLACKLIST){
		res = (IsFound(for_server_name)) ? false : true;
	}
	else {
		res = true;		// no roaming restriction
	}
	dprint4("IsRoamingAllowed(%s)=%d\n", for_server_name, res);
	return res;
}

bool VS_RoamingSettings::IsFound(const char *for_server_name)
{
	if (!for_server_name || !*for_server_name)
		return false;

	std::string server_name = for_server_name;
	std::size_t pos = server_name.find_first_of("#");
	if (pos != std::string::npos)
		server_name.erase(pos);
	if (server_name.empty())
		return false;

	std::map<std::string, std::string> sid;
	std::vector<boost::regex> e;
	{
		std::lock_guard<std::mutex> lock(m_lock);
		e = m_e;
		sid = m_sid_with_domain;
	}

	for (const auto& i : e)
	{
		if (boost::regex_search(server_name, i))
			return true;
	}

	for (const auto&i : sid)
	{
		if (strcasecmp(i.first.c_str(),server_name.c_str())==0)
			return true;
	}

	return false;
}

void VS_RoamingSettings::ScheduleTimer(std::chrono::steady_clock::duration delay)
{
	m_timer.expires_from_now(delay);
	m_timer.async_wait(
		[this, self = shared_from_this()](const boost::system::error_code& ec)
		{
			if (ec == boost::asio::error::operation_aborted)
				return;

			// Make a copy of domains to resolve because:
			//    1. We can't release the lock while iterating over m_sid_with_domain.
			//    2. We shouldn't hold the lock while long synchronous calls (VS_DNSGet*ForDomain) are running.
			std::set<std::string> to_resolve;
			{
				std::lock_guard<std::mutex> lock(m_lock);
				for (const auto& kv : m_sid_with_domain)
					to_resolve.insert(kv.second);
			}
			dstream4 << "RoamingSettings: " << to_resolve.size() << " domains to resolve";

			const auto start_time = std::chrono::steady_clock::now();

			for (const auto& domain : to_resolve)
			{
				if (!m_should_run.load(std::memory_order_acquire))
					return;

				std::set<std::string> results;
				auto do_resolve = [&](bool(*filter)(const net::dns::srv_reply &reply)) {

					auto res = net::dns::make_srv_lookup(net::dns::get_srv_vcs2_query_by_domain(domain)).get();
					if (!res.second)
					{
						for (auto &item : res.first)
						{
							if ((*filter)(item))
								results.insert(std::move(item.host));
						}
					}
				};

				do_resolve(net::dns::is_as_srv);
				do_resolve(net::dns::is_rs_srv);

				dstream4 << "RoamingSettings: Found " << results.size() << " entries for " << domain;
				for (const auto& x : results)
					dstream4 << "RoamingSettings: Resolved " << domain << " to " << x;

				std::lock_guard<std::mutex> lock(m_lock);
				if (!results.empty())
				{
					for (const auto& x : results)
						m_sid_with_domain[x] = domain;
				}
				else
					m_sid_with_domain[domain] = domain;
			}


			for (const auto& domain : to_resolve)
			{
				if (!m_should_run.load(std::memory_order_acquire))
					return;

				std::set<std::string> results;
				auto do_resolve = [&](bool (*filter)(const net::dns::srv_reply &srv)) {

					auto res = net::dns::make_srv_lookup(net::dns::get_srv_vcs2_query_by_domain(domain)).get();
					if(!res.second) //no err
					{
						for(auto &item : res.first)
						{
							if(filter(item))
								results.insert(std::move(item.host));
						}
					}
				};

				do_resolve(net::dns::is_as_srv);
				do_resolve(net::dns::is_rs_srv);

				dstream4 << "RoamingSettings: Found " << results.size() << " entries for " << domain;
				for (const auto& x : results)
					dstream4 << "RoamingSettings: Resolved " << domain << " to " << x;

				std::lock_guard<std::mutex> lock(m_lock);
				if (!results.empty())
				{
					for (const auto& x : results)
						m_sid_with_domain[x] = domain;
				}
				else
					m_sid_with_domain[domain] = domain;
			}
			const auto end_time = std::chrono::steady_clock::now();

			ScheduleTimer(std::max<std::chrono::steady_clock::duration>(c_min_resolve_interval, c_resolve_interval - (end_time - start_time)));
		}
	);
}
