#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include "ddns_utils.h"
#include "../common/commonlibs/std-generic/cpplib/hton.h"

#include "../common/std/cpplib/netutils.h"

using namespace netutils;

// Connection String Parser

#define DELIMITERS ","

ddns_utils::ConnectionStringParser::ConnectionStringParser(const std::string &conn_string)
	: m_conn_string(conn_string)
{}

ddns_utils::ConnectionStringParser::~ConnectionStringParser(void)
{}

bool ddns_utils::ConnectionStringParser::Parse(void)
{
	char *ctx;
	char *tok;
	int pnum;
	std::vector<char> conn_string(m_conn_string.size() + 1);

	// split ip:port token into ip and port strings
	auto parse_addr = [this](char *tok) -> bool {
		AddrPair addr;
		size_t addr_len;
		char *port = strrchr(tok, ':');

		if (port == NULL)
		{
			m_addrs.clear();
			return false;
		}

		addr_len = port - tok;
		port++; // skip ':'

		// save ip and port
		addr.first.assign(tok, addr_len);
		addr.second.assign(port);

		m_addrs.push_back(addr);
		return true;
	};

	// copy string to temporary storage
	strcpy(&conn_string[0], m_conn_string.c_str());

	// parse ip:port tokens
	for (ctx = tok = NULL, pnum = 0; (tok = strtok_s((pnum == 0 ? &conn_string[0] : NULL), DELIMITERS, &ctx)) != NULL ; pnum++)
	{
		if (!parse_addr(tok))
			return false;
	}

	return (pnum > 0 ? true : false);
}

size_t ddns_utils::ConnectionStringParser::GetCount(void)
{
	return m_addrs.size();
}

const ddns_utils::AddrPair &ddns_utils::ConnectionStringParser::GetAddr(const size_t num)
{
	return m_addrs[num];
}

void ddns_utils::DeleteDNSServerRecords(const std::string &dns_req_name, const WORD type, VS_DDNS *ddns)
{
	PDNS_RECORDA recs = NULL;

	if (ddns->GetRecords(recs, dns_req_name.c_str(), type) && recs != NULL)
	{
		PDNS_RECORDA it;
		std::vector<DNS_RECORDA> del_records;

		// find all A records to delete
		for (it = recs; it != NULL; it = it->pNext)
		{
			if (_stricmp(it->pName, dns_req_name.c_str()) == 0)
			{
				del_records.push_back(*it);
			}
		}

		// fix pNext pointers
		for (size_t i = 0; i < del_records.size(); i++)
		{
			if (i < (del_records.size() - 1))
			{
				del_records[i].pNext = &del_records[i + 1];
			}
			else
			{
				del_records[i].pNext = NULL;
			}
		}

		// delete old records
		if (del_records.size() > 0)
			ddns->UpdateRecords(NULL, &del_records[0], true);

		ddns->FreeRecords(recs);
		recs = NULL;
	}
}

void ddns_utils::UpdateDNSServerRecords(const std::string &srv, VS_DDNS *ddns, const AddrSet &gaddrs, const bool delete_old)
{
	std::string service = ddns->GetDNSService().m_str;
	std::string srv_name  = ((service.back()) == '.' ? service : service + ".") + srv;

	if (delete_old)
	{
		// deleta old A records
		DeleteDNSServerRecords(srv, DNS_TYPE_A, ddns);

		// delete old SRV records
		DeleteDNSServerRecords(srv_name, DNS_TYPE_SRV, ddns);
	}

	// create records for all global addresses
	for (auto &a : gaddrs)
	{
		IPAddress ip_res;
		uint32_t ip;

		DNS_RECORDA rec_A, rec_SRV;

		if (!StringToIPAddress(a.first.c_str(), ip_res) || ip_res.type != IP_ADDR_V4)
			continue;

		ip = ip_res.addr.ipv4;

		// fill records with zeroes
		memset((void *)&rec_A, 0, sizeof(rec_A));
		memset((void *)&rec_SRV, 0, sizeof(rec_SRV));

		// fill A record
		rec_A.pNext = NULL;
		rec_A.pName = (char *)srv.c_str();
		rec_A.wType = DNS_TYPE_A;
		rec_A.wDataLength = sizeof(DNS_RECORDA);
		rec_A.dwTtl = DNS_SRV_TTL;
		rec_A.Data.A.IpAddress = vs_htonl(ip);

		// fill SRV record
		rec_SRV.pNext = NULL;
		rec_SRV.pName = (char *)srv_name.c_str();
		rec_SRV.wType = DNS_TYPE_SRV;
		rec_SRV.wDataLength = sizeof(DNS_RECORDA);
		rec_SRV.dwTtl = DNS_SRV_TTL;
		rec_SRV.Data.SRV.pNameTarget = (PSTR)srv.c_str();
		rec_SRV.Data.SRV.wPort = atoi(a.second.c_str());
		rec_SRV.Data.SRV.wPriority = ddns->GetPriority();
		rec_SRV.Data.SRV.wWeight = ddns->GetWeight();

		// add new A record
		ddns->UpdateRecords(&rec_A, NULL, true);

		// add new SRV record
		ddns->UpdateRecords(&rec_SRV, NULL, true);
	}
}
