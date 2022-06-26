#include "VS_DNSUtils.h"

#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/VS_Protocol.h"
#include "net/EndpointRegistry.h"

namespace net {
	namespace dns
	{
		std::string get_srv_query_by_server_id(string_view serverId)
		{
			auto type_pos = serverId.find('#');
			const auto server_name = serverId.substr(0, type_pos);
			const auto server_type = (type_pos == string_view::npos) ? "" : serverId.substr(type_pos + 1);

			std::string srv_record;
			if (server_type.empty()) {	// by default it is #as
				srv_record = "as.tcp."; srv_record += server_name;
			}
			else if (VS_GetServerType(serverId) == ST_VCS) {
				srv_record = net::dns::VCS2_DDNS_SRV; srv_record += server_name;
			}
			else if (!server_type.empty()) {
				srv_record = static_cast<std::string>(server_type); srv_record += ".tcp."; srv_record += server_name;
			}

			return srv_record;
		}

		bool resolved_srv_to_registry(string_view serverId, const net::dns::srv_reply_list &srvs, bool user)
		{
			if (!srvs.empty())
				return false;

			std::string server_name(serverId);
			if (server_name.find('#') == std::string::npos)
				server_name += "#as";	// use #as by default

			for (auto &item : srvs)
				net::endpoint::AddConnectTCP({ item.host, item.port, net::endpoint::protocol_tcp }, server_name, user);

			return true;
		}
	}
}
