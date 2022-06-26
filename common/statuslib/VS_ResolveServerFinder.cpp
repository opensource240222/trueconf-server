#include "VS_ResolveServerFinder.h"
#include "net/DNSUtils/VS_DNS.h"
#include "std/cpplib/VS_UserData.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/debuglog/VS_Debug.h"
#include "std/cpplib/event.h"
#include "std/cpplib/VS_CallIDUtils.h"
#ifdef _WIN32
#include "../transport/Lib/VS_TransportUtils.h"
#endif
#include "newtransport/Lib/TransportUtils.h"
#include "net/DNSUtils/VS_DNSUtils.h"
#include <list>
#include "net/Lib.h"

#include <cctype>
#include <algorithm>

#define DEBUG_CURRENT_MODULE VS_DM_USPRS
std::mutex	VS_ResolveServerFinder::m_instance_lock;
std::map<std::string, std::unique_ptr<VS_ResolveServerFinder>> VS_ResolveServerFinder::m_instance_by_epname = decltype(VS_ResolveServerFinder::m_instance_by_epname)();
extern std::string g_tr_endpoint_name;
boost::asio::io_service* VS_ResolveServerFinder::pios = nullptr;

bool VS_ResolveServerFinder::GetServerNameByHostPort(const std::string &host, net::port port, std::string& serverName) {
	if (!VS_ResolveServerFinder::pios) return false;

	boost::system::error_code ec;
	net::address addr = net::address::from_string(host, ec);

	if (addr.is_unspecified())
	{
		auto reply_a_aaaa = net::dns::make_a_aaaa_lookup(host).get();

		bool is_resolved = false;

		for (const net::dns::a_hostent_reply *item : { &reply_a_aaaa.first, &reply_a_aaaa.second })
		{
			if (!item->ec) // no err
			{
				for (auto &item_addr : item->host.addrs)
				{
					addr = item_addr;
					if (!addr.is_unspecified())
					{
						is_resolved = true;
						break;
					}
				}
			}

			if(is_resolved)
				break;
		}

		if (!is_resolved) //err
		{
			dstream4 << "Error\tCan't find address for '" << host;
			return false;
		}
	}

	if (false);
#ifndef DENY_NEW_TRANSPORT
	else if (VS_ResolveServerFinder::pios) {
		boost::asio::ip::tcp::endpoint tcp_ep(addr, port);

		auto done = std::make_shared<vs::event>(false);
		auto pSrvName = std::make_shared<std::string>();
		transport::GetServerNameByAddress(*VS_ResolveServerFinder::pios, tcp_ep, [pSrvName, host, port, done](const boost::system::error_code& ec, const char* name) {
			VS_SCOPE_EXIT{ done->set(); };
			if (name)
				*pSrvName = name;
			else
				dstream4 << "Error\tGet server name for '" << host << "' port = '" << port << "' failed. " << ec.message();
		});
		if (done->wait_for(std::chrono::seconds(5))) {
			serverName = *pSrvName;
			return !serverName.empty();
		}
	}
#endif
#ifdef _WIN32
	else {
		ServerNameByAddressResult res = ServerNameByAddressResult::err_unknow;
		if (VS_GetServerNameByAddress(addr.to_string(), port, [&serverName, &res](const std::string &name, const ServerNameByAddressResult &r) {serverName = name;	res = r; }))
			if (ServerNameByAddressResult::res_ok == res)
				return true;
			else {
				dstream4 << "VS_GetServerNameByAddress return error code = " << static_cast<uint32_t>(res) << " for host = " << host << " port = " << port;
			}
	}
#endif
	return false;
}

bool VS_ResolveServerFinder::IsServerOnline(string_view endpoint_name)
{
	if (endpoint_name.empty())
		return false;
	const auto n_tcp_endpoints = net::endpoint::GetCountConnectTCP(endpoint_name, false);
	for (unsigned i = 1; i <= n_tcp_endpoints; ++i)
	{
		auto conn = net::endpoint::ReadConnectTCP(i, endpoint_name);
		if (!conn)
			continue;
		std::string got_endpoint_name;
		if (GetServerNameByHostPort(conn->host, conn->port, got_endpoint_name))
			return strcasecmp(((std::string)endpoint_name).c_str(), got_endpoint_name.c_str()) == 0;
	}
	return false;
}

const std::chrono::steady_clock::duration VS_ResolveServerFinder::domain_cache_lifetime = std::chrono::minutes(15); //15 mins
static const std::function<bool(const std::string&, std::string&)> default_get_resolver_by_domain_func = [](const std::string&domain, std::string&serverName)
{
	auto reply = net::dns::make_srv_lookup(net::dns::get_srv_vcs2_query_by_domain(domain)).get();
	if(!reply.second) //no err
	{
		for(auto &item : reply.first)
		{
			if (net::dns::is_rs_srv(item) && VS_ResolveServerFinder::GetServerNameByHostPort(item.host, item.port, serverName))
				return true;
		}
	}
	return false;
};

std::function<bool(const std::string&, std::string&)> VS_ResolveServerFinder::m_getResolverByDomainFunc = default_get_resolver_by_domain_func;

void VS_ResolveServerFinder::SetResolverByDomainFunc(const std::function<bool(const std::string&, std::string&)>& func)
{
	if (!func)
		m_getResolverByDomainFunc = default_get_resolver_by_domain_func;
	else
		m_getResolverByDomainFunc = func;
}

const std::function<bool(const std::string&, std::string&)> & VS_ResolveServerFinder::GetResolverByDomainFunc()
{
	if (!!m_getResolverByDomainFunc)
		return m_getResolverByDomainFunc;
	else
		return default_get_resolver_by_domain_func;
}


bool VS_ResolveServerFinder::GetServerForResolve(const char *id, std::string &server, bool use_cache, const bool byDomain)
{
	if(!id || !*id )
		return false;

	if (VS_IsRTPCallID(id)) {
		dstream4 << "VS_ResolveServerFinder::GetServerForResolve suppressed searching for '" << id << "'. This function is used only for VS protocol.\n";
		return false;
	}

	dprint4("GetServerForResolve for call_id = %s, use_cache = %d, byDomain = %d\n", id, use_cache, byDomain);
	bool res(false);
	VS_SCOPE_EXIT{
		if (res)
		{
			dprint4("ResolveServer = %s\n", server.c_str());
		}
		else
		{
			dprint4("ResolveServer not found\n");
		}
	};

	std::string defaultServer;
	GetDefaultResolveServer(defaultServer);
	const char *p(0);
	const char* adomain = byDomain? id : !(p = strchr(id,'@'))?0:p + 1;
	if(!adomain || !(*adomain))
	{
		dprint4("domain is empty\n");
		if(defaultServer.empty())
			return res = false;
		server = defaultServer;
		return res= true;
	}

	std::string sdomain{ VS_RemoveTranscoderID_sv(adomain) };
	std::transform(sdomain.begin(), sdomain.end(), sdomain.begin(), ::tolower);

	const char *domain = sdomain.c_str();
	unsigned long len = !(p = strchr(g_tr_endpoint_name.data(), '#')) ? strlen(g_tr_endpoint_name.data()) : g_tr_endpoint_name.length() - strlen(p);
	if (!g_tr_endpoint_name.compare(0, len, domain))
	{
		server = g_tr_endpoint_name;
		return res = true;
	}
	{
		std::lock_guard<std::mutex> lock(m_lock);
		if (!byDomain)
		{
			auto by_user_it = uplink_callid_st_.get<by_user>().find(id); // never expired until users migrate
			if (by_user_it != uplink_callid_st_.get<by_user>().end()&&by_user_it->by_push_)
			{
				server = by_user_it->uplink_;
				return res = true;
			}
		}
		auto iter = m_serverNameByDomain.find(domain);
		if(use_cache || iter!=m_serverNameByDomain.end() )
		{
			if (iter != m_serverNameByDomain.end())
			{
				auto now_time = std::chrono::steady_clock::now();
				if(use_cache || now_time < iter->second.second)
				{
					server = iter->second.first;
					dprint4("@ResolveServer in cache; call_id = %s ==> serv = %s\n",id,server.c_str());
					return res = true;
				}
			}
			else
				return res = false;
		}
	}
	std::string serverName;
	if (m_getResolverByDomainFunc(domain, serverName))
	{
		server = serverName;
		std::lock_guard<std::mutex> lock(m_lock);
		dprint4("@ResolveServer by address; call_id = %s ==> serv = %s\n", id, server.c_str());
		auto & item = m_serverNameByDomain[domain];
		item.first = std::move(serverName);
		item.second = std::chrono::steady_clock::now() + domain_cache_lifetime;
		return res = true;
	}
	else
	{
		server = domain;
		server+= "#vcs";
		{
			std::lock_guard<std::mutex> lock(m_lock);
			auto& item = m_serverNameByDomain[domain];
			if (item.first.size() <= 0)
				item.first = server;
			item.second = std::chrono::steady_clock::now() + domain_cache_lifetime;
			dprint4("@ResolveServer is vcs; call_id = %s ==> serv = %s (cache:%s)\n", id, server.c_str(), item.first.c_str());
		}
		return res=true;
	}
}
bool VS_ResolveServerFinder::GetServerByUser(const std::string &id, std::string &server) const
{
	std::lock_guard<std::mutex> l(m_lock);
	auto it = uplink_callid_st_.get<by_user>().find(id);
	if (it == uplink_callid_st_.get<by_user>().end())
		return false;
	else
		server = it->uplink_;
	return true;
}

bool VS_ResolveServerFinder::GetASServerForResolve(const char *call_id, std::string &server, bool use_cache)
{
	if(!call_id || !*call_id )
		return false;
	/**
		TODO: if call_id is alias then the code will not work in AS
	*/
	dprint4("GetASServerForResolve(...) for call_id = %s, use_cache = %d\n", call_id, use_cache);

	const char *p(0);
	const char* domain = !(p = strchr(call_id,'@'))?call_id:p + 1;
	bool res(false);
	VS_SCOPE_EXIT{
		if (res)
		{
			dprint4("ResolveServer = %s\n", server.c_str());
		}
		else
		{
			dprint4("ResolveServer not found \n");
		}
	};
	if(!domain || !(*domain))
		return res;

	unsigned long len = !(p = strchr(g_tr_endpoint_name.data(), '#')) ? g_tr_endpoint_name.length() : g_tr_endpoint_name.length() - strlen(p);
	if (!g_tr_endpoint_name.compare(0, len, domain))
	{
		server = g_tr_endpoint_name;
		return res = true;
	}

	auto reply = net::dns::make_srv_lookup(net::dns::get_srv_vcs2_query_by_domain(domain)).get();

	if (!reply.second) // no err
	{
		for(auto &item : reply.first)
		{
			if (net::dns::is_as_srv(item) && GetServerNameByHostPort(item.host, item.port, server))
			{
				res = true;
				break;
			}
		}
		return res;
	}

	server = domain;
	server += "#vcs";
	return res = true;
}

void VS_ResolveServerFinder::CheckDomainsCacheForLifetime()
{
	auto now = std::chrono::steady_clock::now();
	decltype(m_serverNameByDomain) tempSrvMap;
	for (auto &iter : tempSrvMap)
	{
		if (now <= iter.second.second)
		{
			std::string server;
			GetServerForResolve(iter.first.c_str(), server, false, true);
		}
	}
	m_last_check_domain_cache = now;
}

VS_ResolveServerFinder * VS_ResolveServerFinder::Instance()
{
	std::lock_guard<std::mutex> lock(m_instance_lock);

	auto it = m_instance_by_epname.find(g_tr_endpoint_name);
	if (it == m_instance_by_epname.end())
		it = m_instance_by_epname.emplace(g_tr_endpoint_name, std::unique_ptr<VS_ResolveServerFinder>(new VS_ResolveServerFinder())).first;
	return it->second.get();
}
void VS_ResolveServerFinder::Release()
{
	std::lock_guard<std::mutex> lock(m_instance_lock);
	m_instance_by_epname.clear();
}
bool VS_ResolveServerFinder::IsCacheActual()
{
	std::lock_guard<std::mutex>	lock(m_lock);
	return std::chrono::steady_clock::now() < domain_cache_lifetime + m_last_check_domain_cache;
}

void VS_ResolveServerFinder::RegisterExternalPresence(std::shared_ptr<VS_ExternalPresenceInterface> presence)
{
	std::lock_guard<std::mutex> lock(m_lock);
	m_external_presences.emplace(std::move(presence));
}

void VS_ResolveServerFinder::UnRegisterExternalPresence(std::shared_ptr<VS_ExternalPresenceInterface> presence)
{
	std::lock_guard<std::mutex> lock(m_lock);
	m_external_presences.erase(presence);
}

std::shared_ptr<VS_ExternalPresenceInterface> VS_ResolveServerFinder::GetExternalPresence(const char *call_id)
{
	std::shared_ptr<VS_ExternalPresenceInterface> empty_ptr;
	std::lock_guard<std::mutex> lock(m_lock);
	for (auto iter = m_external_presences.begin(); iter != m_external_presences.end(); iter++)
	{
		if ((*iter)->IsMyCallId(call_id))
			return *iter;
	}
	return empty_ptr;
}
void VS_ResolveServerFinder::SetDefaultResolveServer(const char *server_name)
{
	std::lock_guard<std::mutex> lock(m_lock);
	m_defaultResolveServer = server_name;
}

void VS_ResolveServerFinder::GetDefaultResolveServer(std::string &server_name)
{
	std::lock_guard<std::mutex> lock(m_lock);
	server_name = m_defaultResolveServer;
}
void VS_ResolveServerFinder::SetServerForUser(const std::string&user_id, const std::string &server_name, bool by_push)
{
	std::lock_guard<std::mutex> l(m_lock);
	dstream4 << "VS_ResolveServerFinder::SetServerForUser: user_id = " << user_id << "; server_name = " << server_name << "; force = " << by_push << ";\n";
	auto it = uplink_callid_st_.get<by_user>().find(user_id);
	if (it == uplink_callid_st_.get<by_user>().end())
		uplink_callid_st_.emplace(user_id, server_name,by_push, std::chrono::steady_clock::now());
	else
	{
		uplink_callid_st_.modify(it, [&](UplinkForCallId&item)
		{
			if (item.uplink_ != server_name)
				item.uplink_ = server_name;
			item.by_push_ = by_push;
			item.expire_ = std::chrono::steady_clock::now();
		});
	}
}

std::vector<std::string> VS_ResolveServerFinder::GetUsersForUplink(const std::string &uplink) const
{
	std::lock_guard<std::mutex> l(m_lock);
	auto range = uplink_callid_st_.get<by_uplink>().equal_range(uplink);
	std::vector<std::string> res;
	std::transform(range.first, range.second, std::back_inserter(res), [](const UplinkForCallId&item) -> std::string {return item.call_id_;});
	return res;
}

void VS_ResolveServerFinder::SetIOService(boost::asio::io_service & ios)
{
	VS_ResolveServerFinder::pios = &ios;
}

#undef DEBUG_CURRENT_MODULE