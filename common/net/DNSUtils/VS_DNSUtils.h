#pragma once

#include "VS_DNS.h"

namespace net {

	namespace dns
	{
		static const char VZO_DDNS_SRV[] = "login.tcp.";
		static const char VCS_DDNS_SRV[] = "vcs.tcp.";			// SRV vcs.tcp.pca.ru stores SIDs (ru5.v-port.net#as)
		static const char VCS2_DDNS_SRV[] = "vcs2.tcp.";			// SRV vcs2.tcp.pca.ru stores IP/Host:Port

		bool resolved_srv_to_registry(string_view serverId, const net::dns::srv_reply_list &srvs, bool user = false);

		std::string get_srv_query_by_server_id(string_view serverId);

		inline std::string get_srv_vcs2_query_by_domain(string_view domain)
		{
			std::string tc_srv = VCS2_DDNS_SRV;
			tc_srv.append(domain.data(), domain.length());
			return tc_srv;
		}

		inline bool is_rs_srv(const net::dns::srv_reply &reply)
		{
			return reply.priority == 100;
		}

		inline bool is_as_srv(const net::dns::srv_reply &reply)
		{
			return reply.priority == 10;
		}

	}
}
