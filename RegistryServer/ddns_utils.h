#pragma once

#include <cstdlib>
#include <cstring>
#include <cstdint>

#include <utility>
#include <vector>
#include <string>

#include "transport/Client/VS_DNSFunction.h"

namespace ddns_utils {
	typedef std::pair<std::string, std::string> AddrPair; // address and port
	typedef std::vector<AddrPair> AddrSet;

	class ConnectionStringParser {
	public:
		ConnectionStringParser(const std::string &conn_string);
		virtual ~ConnectionStringParser();

		bool Parse(void);
		size_t GetCount(void);
		const AddrPair &GetAddr(const size_t num);
	private:
		const std::string &m_conn_string;
		AddrSet m_addrs;
	};

	void DeleteDNSServerRecords(const std::string &dns_req_name, const WORD type, VS_DDNS *ddns);
	// Update DDNS Server Records
	void UpdateDNSServerRecords(const std::string &srv, VS_DDNS *ddns, const AddrSet &gaddrs, bool delete_old = true);
}
