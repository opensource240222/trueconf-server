#include "VS_AutoDiscovery.h"
#include "VS_DNSServerParam.h"
#include "net/DNSUtils/VS_DNSUtils.h"
#include "net/DNSUtils/VS_DNS.h"
#include "../../net/EndpointRegistry.h"

#include "std-generic/compat/map.h"

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <unistd.h>
#include <netdb.h>
#endif

inline static void VS_DNSGetLocalDomain(VS_SimpleStr& out)
{
	char name[512] = { 0 };
	hostent	*host(0);

	if ((0 != gethostname(name, 512)) || (!(host = gethostbyname(name))))
		return;
	if (!host->h_name)
		return;

	out = host->h_name;
}

inline static bool FindSRVEnpointRecursively(const string_view service, const string_view domain, std::vector<std::pair<std::string, net::port>> &found_endpoints, std::string &found_domain, const unsigned short priority = 10)
{
	if (domain.find('.') == string_view::npos) return false;	// don't search in 1-level domain (example: ru, org, com, net)

	string_view _domain = domain;
	size_t pos(0);
	do {

		std::string service_domain(service); service_domain += _domain;
		auto res = net::dns::make_srv_lookup(std::move(service_domain)).get();
		if (!res.second)
		{
			for(auto &record : res.first)
			{
				if(!(record.priority != priority)) // old impl: remove if ep.priority != priority
					found_endpoints.emplace_back(std::move(record.host), record.port);
			}

			if (!found_endpoints.empty())
			{
				found_domain = static_cast<std::string>(_domain);
				return true;
			}
		}

		pos = _domain.find('.', pos);
		if (pos == string_view::npos) break;
		_domain = _domain.substr(pos + 1);
	} while (true);

	return false;
}

bool VS_AutoDiscoveryServers(char* _domain, VS_SimpleStr* &servers, unsigned int &num, bool /*get_info*/)
{
	bool IsEmptyDomain = !_domain || !*_domain;
	bool IsTrueConfOnline = !IsEmptyDomain;
	VS_SimpleStr domain;
	if (IsEmptyDomain)
		VS_DNSGetLocalDomain(domain);
	else
		domain = _domain;

	if (!domain)
		return false;

	std::vector<std::pair<std::string, net::port>> found_enpoints;
	num = 0;
	const char* def_srv = VS_DNSGetDefaultService(_domain);
	if (!def_srv) return false;

	std::string found_domain;
	if (def_srv && domain) {
		FindSRVEnpointRecursively(def_srv, domain.m_str, found_enpoints, found_domain);
		num = found_enpoints.size();
	}

	if (IsTrueConfOnline) {
		if (!num) {		// no SRV, try A-type resolve: vcs2.tcp.domain
			std::string A = def_srv; A += _domain;
			std::vector<std::string> ips;

			auto dns_ipv6_res = net::dns::make_aaaa_lookup(A);
			auto dns_ipv4_res = net::dns::make_a_lookup(A);

			const auto resolve = [](decltype(dns_ipv4_res.get()) &&dns, decltype(ips) &ips) noexcept
			{
				if (!dns.second)
				{
					for(auto &addr : dns.first.addrs)
						ips.push_back(addr.to_string());
				}
			};

			resolve(dns_ipv6_res.get(), ips);
			resolve(dns_ipv4_res.get(), ips);

			if (ips.empty()) { // no SRV, no A - use static IPs
				if (domain == "trueconf.com") {
					ips.emplace_back("70.38.54.221");
					ips.emplace_back("88.99.166.96");
					ips.emplace_back("92.53.73.212");
					ips.emplace_back("95.213.162.203");
					ips.emplace_back("178.20.154.93");
					ips.emplace_back("208.76.250.218");
				}
				else if (domain == "conferen.do") {
					ips.emplace_back("92.53.73.211");
					ips.emplace_back("95.169.187.104");
					ips.emplace_back("174.142.26.28");
				}
				// removed unused domains
				//else if (domain == "ccs.vn") {
				//	strcpy(ips[ips_count++], "113.164.1.61");
				//	strcpy(ips[ips_count++], "113.164.1.43");
				//}
				//else if (domain == "videoconf.org.ua") {
				//	strcpy(ips[ips_count++], "212.90.168.7");
				//	strcpy(ips[ips_count++], "212.90.168.8");
				//}
				else {
					std::string main("main."); main+=domain.m_str;
					ips.emplace_back(main);
				}
			}

			found_enpoints.reserve(ips.size() * 3);	// three ports per server: 4307, 80, 443
			for (auto& ip : ips)
			{
				found_enpoints.emplace_back(ip, 4307);
				found_enpoints.emplace_back(ip, 80);
				found_enpoints.emplace_back(ip, 443);
				num += 3;
			}
		}

		struct s_port { std::string s; bool p443;  bool p80; s_port(const char* server) : s(server), p443(false), p80(false) {} };

		vs::map<std::string, s_port> m;
		char ServerID[256] = {0};
		unsigned int idx(0);
		servers = new VS_SimpleStr[num];
		for (const auto& ep : found_enpoints){
			auto it = m.find(ep.first);
			if (it == m.end())
			{
				sprintf(ServerID, "%.127s#as", ep.first.c_str());
				it = m.emplace(ep.first, s_port(ServerID)).first;
				net::endpoint::ClearAllConnectTCP(ServerID, true);
				servers[idx] = ServerID;
				idx++;
			}
			net::endpoint::AddConnectTCP({ ep.first, ep.second, net::endpoint::protocol_tcp }, it->second.s, true);
			it->second.p443 = ep.second == 443;
			it->second.p80 = ep.second == 80;
		}
		for (const auto& serv : m) {
			if (!serv.second.p443)
				net::endpoint::AddConnectTCP({ serv.first, net::port(443), net::endpoint::protocol_tcp }, serv.second.s, true);
			if (!serv.second.p80)
				net::endpoint::AddConnectTCP({ serv.first, net::port(80), net::endpoint::protocol_tcp }, serv.second.s, true);
		}
		num = m.size();		// unique servers (many ports)
	}
	else {
		if (!num)	return false;

		servers = new VS_SimpleStr[num];
		VS_SimpleStr server = (!found_domain.empty())? found_domain.c_str(): domain.m_str;
		server += "#vcs";
		servers[0] = server;

		net::endpoint::ClearAllConnectTCP(server.m_str, true);
		for (auto& ep : found_enpoints){
			net::endpoint::AddConnectTCP({ ep.first, ep.second, net::endpoint::protocol_tcp }, server.m_str, true);
		}
		// do not try service AS ports at TCC mode
		//net::endpoint::AddConnectTCP({ d, 4307, net::endpoint::protocol_tcp }, server.m_str, true);
		//net::endpoint::AddConnectTCP({ d, 443,  net::endpoint::protocol_tcp }, server.m_str, true);
		//net::endpoint::AddConnectTCP({ d, 80,   net::endpoint::protocol_tcp }, server.m_str, true);
		num = 1;			// number of servers found (not number of server IP/Host)
	}

	return true;
}

const char* VS_DNSGetDefaultService(const char* domain, bool IsOldSRV)
{
	const char* srv = 0;
	if (domain && *domain && strcasecmp(domain, "vzochat.com")==0)
		srv = net::dns::VZO_DDNS_SRV;
	else
		srv = (IsOldSRV) ? net::dns::VCS_DDNS_SRV : net::dns::VCS2_DDNS_SRV;
	return srv;
}