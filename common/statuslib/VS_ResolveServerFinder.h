#pragma once
#include "VS_ExternalPresenceInterface.h"
#include "std-generic/cpplib/macro_utils.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/asio_fwd.h"

#include <set>
#include <chrono>
#include <map>
#include <memory>
#include <vector>
#include <mutex>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/tag.hpp>
#include "net/Port.h"

class VS_TransportRouterServiceBase;
namespace
{
	namespace mi = boost::multi_index;
}
class VS_ResolveServerFinder
{
	static std::mutex m_instance_lock;
	static std::map<std::string, std::unique_ptr<VS_ResolveServerFinder>> m_instance_by_epname;
	static const std::chrono::steady_clock::duration domain_cache_lifetime;
	using ServerNameByDomainMapT = std::map<std::string/** domain */, std::pair<std::string /** server name */, std::chrono::steady_clock::time_point /** expiration time*/>>; // key -> domain;
	struct UplinkForCallId
	{
		VS_FORWARDING_CTOR4(UplinkForCallId, call_id_, uplink_, by_push_, expire_) {}
		std::string call_id_;
		std::string uplink_;
		bool by_push_;
		std::chrono::steady_clock::time_point expire_;
	};
	using UplinkCallIdSet = boost::multi_index_container <
		UplinkForCallId,
		mi::indexed_by <
		mi::ordered_unique<mi::tag < struct by_user > , mi::member<UplinkForCallId, std::string, &UplinkForCallId::call_id_ >> ,
		mi::ordered_non_unique<mi::tag < struct by_uplink > , mi::member<UplinkForCallId, std::string, &UplinkForCallId::uplink_ >>
		>
	>;
	UplinkCallIdSet uplink_callid_st_;
	ServerNameByDomainMapT	m_serverNameByDomain; //cache for domain->server;
	mutable	std::mutex				m_lock;
	std::string				m_defaultResolveServer;
	std::chrono::steady_clock::time_point	m_last_check_domain_cache = std::chrono::steady_clock::now();
	VS_ResolveServerFinder(){}
	std::set<std::shared_ptr<VS_ExternalPresenceInterface>> m_external_presences;
	static std::function<bool(const std::string&, std::string&)>	m_getResolverByDomainFunc;	//1. [in] domain, 2. [out] serverName,
public:
	static VS_ResolveServerFinder * Instance();
	static void Release();
	static void SetResolverByDomainFunc(const std::function<bool(const std::string&, std::string&)>&);
	static const std::function<bool(const std::string&, std::string&)> &GetResolverByDomainFunc();
	static bool GetServerNameByHostPort(const std::string &host, net::port port, std::string& serverName);
	static bool IsServerOnline(string_view endpoint_name);
	bool GetServerForResolve(const char *id, std::string &server, bool use_cache, const bool byDomain = false); // if byDomain == true => id == 'user@domain' else just 'domain'
	bool GetServerByUser(const std::string &id, std::string &server) const; // allways from cache
	bool GetASServerForResolve(const char *call_id, std::string &server, bool use_cache);
	void CheckDomainsCacheForLifetime();
	bool IsCacheActual();

	void SetDefaultResolveServer(const char *server_name);
	void GetDefaultResolveServer(std::string &server_name);

	void RegisterExternalPresence(std::shared_ptr<VS_ExternalPresenceInterface> presence);
	void UnRegisterExternalPresence(std::shared_ptr<VS_ExternalPresenceInterface> presence);

	std::shared_ptr<VS_ExternalPresenceInterface> GetExternalPresence(const char *call_id);
	void SetServerForUser(const std::string&user_id, const std::string &server_name, bool by_push = false);
	std::vector<std::string> GetUsersForUplink(const std::string &uplink) const;

	static void SetIOService(boost::asio::io_service& ios);
	static boost::asio::io_service* pios;
};