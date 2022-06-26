#if defined(_WIN32) // Not ported yet

/**
 **************************************************************************
 * \file VS_DNSFunction.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief DNS functions
 *
 * \author ktrushnikov
 * \date 03.11.2005
 ****************************************************************************/
#include "VS_DNSFunction.h"
#include "VS_AutoDiscovery.h"
#include "../../acs/Lib/VS_AcsLib.h"
#include "../../net/EndpointRegistry.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"
#include "../../std/cpplib/VS_Utils.h"
#include "../../std/cpplib/VS_Protocol.h"
#include "std-generic/cpplib/hton.h"

#include <algorithm>
TYPE_DnsQuery				 gDnsQuery = 0;
TYPE_DnsRecordListFree		 gDnsRecordListFree = 0;
TYPE_DnsModifyRecordsInSet	 gDnsModifyRecordsInSet = 0;
TYPE_DnsAcquireContextHandle gDnsAcquireContextHandle = 0;
bool						 gDnsLoaded = false;
HINSTANCE					 gDnsLibHandle = 0;

VS_DDNS::TYPE_GetNetworkParams VS_DDNS::sGetNetworkParams = 0;
bool VS_DDNS::sIPHlpLoaded = false;
HINSTANCE VS_DDNS::sIPHlpLibHandle = 0;

unsigned long DNS_SRV_TTL = 480;

bool VS_DNSGetServerParam(const char* aService, VS_DNSServerParam *&aOutput, unsigned int &length, const unsigned short priority = 10)
{
	if (!gDnsLoaded && !VS_DNSLoadPtr())
		return false;

	if ( !aService )
		return false;

    PDNS_RECORD theDNS;
	PDNS_RECORD ptrDNS;

	if (gDnsQuery(aService, DNS_TYPE_SRV, DNS_QUERY_STANDARD, NULL, &theDNS, NULL) )
		return false;

	ptrDNS = theDNS;
	while(ptrDNS)
	{
		if ((ptrDNS->wType == DNS_TYPE_SRV) &&(ptrDNS->Data.Srv.wPriority == priority))
			++length;

		ptrDNS = ptrDNS->pNext;
	}
	if (length==0)
		return false;
	aOutput = new VS_DNSServerParam[length];

	ptrDNS = theDNS;
	length = 0;
	for(;ptrDNS!=0 ;ptrDNS=ptrDNS->pNext)
	{
		if (ptrDNS->wType != DNS_TYPE_SRV)
			continue;

		if(!ptrDNS->Data.SRV.pNameTarget || !strlen(ptrDNS->Data.SRV.pNameTarget) || !ptrDNS->Data.SRV.wPort || ptrDNS->Data.SRV.wPriority != priority)
			continue;
		aOutput[length].iHost = ptrDNS->Data.SRV.pNameTarget;
		aOutput[length].iPort = ptrDNS->Data.SRV.wPort;
		length++;
	}
	gDnsRecordListFree(theDNS, DnsFreeRecordList);
	return true;
}

bool VS_DNSLoadPtr()
{
	if (gDnsLoaded) { return true; }
	gDnsQuery = 0;	gDnsRecordListFree = 0;	gDnsLoaded = 0;	gDnsModifyRecordsInSet = 0;
	gDnsLibHandle = LoadLibrary("DNSAPI");
	if (gDnsLibHandle != INVALID_HANDLE_VALUE)
	{
		gDnsQuery =  (TYPE_DnsQuery)GetProcAddress( gDnsLibHandle, "DnsQuery_A");
		gDnsRecordListFree =  (TYPE_DnsRecordListFree)GetProcAddress( gDnsLibHandle, "DnsRecordListFree");
		gDnsModifyRecordsInSet = (TYPE_DnsModifyRecordsInSet)GetProcAddress( gDnsLibHandle, "DnsModifyRecordsInSet_A");
		gDnsAcquireContextHandle = (TYPE_DnsAcquireContextHandle)GetProcAddress( gDnsLibHandle, "DnsAcquireContextHandle_A");
		gDnsLoaded = gDnsQuery && gDnsRecordListFree && gDnsModifyRecordsInSet && gDnsAcquireContextHandle;
		if (gDnsLoaded) { return true; }
	}
	if (gDnsLibHandle) { FreeLibrary( gDnsLibHandle ); }
	gDnsLibHandle = 0;	gDnsQuery = 0;	gDnsRecordListFree = 0;	gDnsModifyRecordsInSet = 0; gDnsAcquireContextHandle = 0; gDnsLoaded = false;
	return false;
}

bool VS_DNSClearPtr()
{
	if (gDnsLibHandle) { FreeLibrary( gDnsLibHandle ); }
	gDnsLibHandle = 0;	gDnsQuery = 0;	gDnsRecordListFree = 0;	gDnsModifyRecordsInSet = 0; gDnsAcquireContextHandle = 0; gDnsLoaded = false;
	return true;
}

VS_DDNS::VS_DDNS()
:m_dns_server_ip(0),m_priority(0),m_weight(0),m_update_srv_type(e_none)
{

	unsigned long ddns_ip(0);
	const unsigned short dns_port(53);


	VS_RegistryKey key(false, CONFIGURATION_KEY);

	me_bind_host[0] = 0;
	key.GetValue(me_bind_host, sizeof(me_bind_host) - 1, VS_REG_STRING_VT, "DDNS MyHost");
	if (!*me_bind_host)
		VS_GetDefaultHostName(me_bind_host, 256);

	unsigned long ttl(0);
	key.GetValue(&ttl, sizeof(ttl), VS_REG_INTEGER_VT, "DDNS Ttl");
	if (ttl) { DNS_SRV_TTL = ttl; }

	m_inited = VS_DNSLoadPtr();
	gDnsAcquireContextHandle(false, 0, &m_handle);

	m_enhanced = m_inited /*& (*ddns_addr != 0)*/;

	me_bind_port = (rand() % 3000) + 1000;
	if (m_enhanced && *me_bind_host) {
		m_enhanced = false;
		for (int i = 0; i < 10; ++i) {
			m_enhanced = me_dns_socket.Bind(me_bind_host, me_bind_port + (i * 7),false);
			if (m_enhanced) { break; } else { me_dns_socket.Close(); }
		}
	}
}

VS_DDNS::~VS_DDNS()
{
}

bool VS_DDNS::GetRecords(PDNS_RECORD &records, WORD type)
{
	if (!m_inited) { return false; }
	IP4_ARRAY dns_server_ip = {1, vs_htonl(m_dns_server_ip)};
	return (gDnsQuery(m_dns_request.m_str, type, DNS_QUERY_BYPASS_CACHE | DNS_QUERY_TREAT_AS_FQDN | DNS_QUERY_DONT_RESET_TTL_VALUES, (m_dns_server_ip)? &dns_server_ip: 0, &records, 0) == 0);
}

bool VS_DDNS::GetRecords(PDNS_RECORD &records, const char *dns_request, WORD type)
{
	if (!m_inited)
	{
		return false;
	}

	char *req = m_dns_request.m_str;

	if (dns_request != NULL)
		req = (char *)dns_request;

	IP4_ARRAY dns_server_ip = { 1, vs_htonl(m_dns_server_ip) };
	return (gDnsQuery(req, type, DNS_QUERY_BYPASS_CACHE | DNS_QUERY_TREAT_AS_FQDN | DNS_QUERY_DONT_RESET_TTL_VALUES, (m_dns_server_ip) ? &dns_server_ip : 0, &records, 0) == 0);
}

bool VS_DDNS::UpdateRecords(PDNS_RECORD add_records, PDNS_RECORD erase_records, bool UseWindowsFunc)
{
	if (!m_inited) { return false; } int result(0);

	if (UseWindowsFunc || !m_enhanced)
	{
		// fill DNS server address
		IP4_ARRAY ip_arr;

		ip_arr.AddrCount = 1;
		ip_arr.AddrArray[0] = vs_htonl(m_dns_server_ip);

		result = gDnsModifyRecordsInSet(add_records, erase_records, DNS_UPDATE_SECURITY_USE_DEFAULT, NULL, &ip_arr, NULL);
	}
	else if (m_enhanced)
	{
		result = eDnsModifyRecordsInSet(add_records, erase_records);
	}

	return (result == 0);
}

bool VS_DDNS::FreeRecords(PDNS_RECORD records)
{
	if (!m_inited) { return false; }
	if (records) { gDnsRecordListFree(records, DnsFreeRecordList); }
	return true;
}

char *VS_DDNS::GetRequestName()
{
	if (!m_inited) { return 0; }
	return m_dns_request.m_str;
}

bool VS_DDNS::eUpdateDnsRecord(char* send, size_t send_sz)
{
	if (!m_enhanced) { return false; }

	const unsigned short m_dns_server_port(53);

	unsigned long ip(0); unsigned short port(0);
	bool result(false);
	const unsigned short packet_id = *((const unsigned short*) send);
	unsigned int max_dns = (m_dns_servers.size() <= 5)? m_dns_servers.size(): 5;

	if (!m_dns_server_ip && !max_dns)		// no any DNS server to update
		return false;

	char recv_buff[2048] = {};

	for (int i = 0; i < 3; ++i) { // three retries to prevent possible packets lost
		if (m_dns_server_ip) {
			int out_bytes = me_dns_socket.SendTo(send, (unsigned long)send_sz, m_dns_server_ip, m_dns_server_port);
			if (out_bytes < 1) { continue; }
		} else {			// try to send to several DNS servers
			for(unsigned int j=0; j < max_dns; j++)
				me_dns_socket.SendTo(send, (unsigned long)send_sz, m_dns_servers[j], m_dns_server_port);
		}

		unsigned int n_dns = 0;
		unsigned long mills = 0;
		do
		{
			n_dns++;
			mills = 5000;
			int n_recv = me_dns_socket.ReceiveFrom(recv_buff, 2048, &ip, &port, mills);
			if (n_recv<=0)
				break;
			if (n_recv<4) // error or small header
				continue;
			unsigned short rpacket_id = *((const unsigned short*) &recv_buff);
			int RCODE = (recv_buff[3] && 0x0f);

			// check ip, that answered
			bool is_server_ok = false;
			if (m_dns_server_ip)
				is_server_ok = (ip == m_dns_server_ip);
			else if (max_dns)
				for(unsigned int j=0; j < n_dns; j++)
					if (ip == m_dns_servers[j])
						is_server_ok = true;

			result |= (is_server_ok && (port == m_dns_server_port) && (!RCODE) && (rpacket_id == packet_id));
		} while(mills && (n_dns < max_dns));
		if (result) { break; }
	}
	return result;
}

bool VS_DDNS::eSendDnsSrvQuery(char* send, size_t send_sz, std::vector<VS_DNSServerParam*> &out)
{
	if (!m_enhanced) { return false; }

	const unsigned short m_dns_server_port(53);

	unsigned long mills(5000);
	unsigned long ip(0); unsigned short port(0);
	bool result(false);
	const unsigned short packet_id = *((const unsigned short*) send);

	for (int i = 0; i < 3; ++i) { // three retries to prevent possible packets lost
		int out_bytes = me_dns_socket.SendTo(send, (unsigned long)send_sz, m_dns_server_ip, m_dns_server_port);
		if (out_bytes < 1) { continue; }
		char recv_buff[2048] = {};
		int n_recv = me_dns_socket.ReceiveFrom(recv_buff, 2048, &ip, &port, mills);
		if (n_recv<4) // error or small header
			continue;
		unsigned short rpacket_id = *((const unsigned short*) &recv_buff);
		int RCODE = (recv_buff[3] && 0x0f);
		result = (ip == m_dns_server_ip) && (port == m_dns_server_port) && (!RCODE) && (rpacket_id == packet_id);
		if (result)
		{
			result = eDecodeDnsSrvQueryResponse(recv_buff, n_recv, (const unsigned int)send_sz, out) > 0;
			break;
		}
	}
	return result;
}

size_t VS_DDNS::eMakeDnsSrvPacket(DNS_RECORDA &record, char *buf, size_t buf_size, unsigned int erase)
{
	if ((!m_enhanced) || (buf_size < 2048) || !IsValid()) { return 0; }

	memset(buf, 0, buf_size);
	size_t index = 0;

	// header
	buf[0] = (rand() % 26) + 65; buf[1] = (rand() % 26) + 65;	index += 2;		// ID
	memcpy(buf + index, "\x28\x00", 2);							index += 2;		// QR = 0; Opcode = 5 (UPDATE); Zero; RCODE = 0;
	memcpy(buf + index, "\x00\x01\x00\x00\x00\x01\x00\x00", 8);	index += 8;		// ZCount(1), PRCount(0), UPCount(1), ADCount(0)
	// domain address
	{
		const unsigned int service_sz = 512;
		char service[service_sz];		memset(service, 0, service_sz);		char* pservice = (char*) service;

		VS_DDNS::eEncodeString(m_dns_domain.m_str, pservice, service_sz);
		unsigned int service_len = (unsigned int) strlen(service);		service_len++;		// add '\0'

		memcpy(buf + index, (void*)&service, service_len);		index += service_len;
	}
	memcpy(buf + index, "\x00\x06\x00\x01", 4);					index += 4;		// ZTYPE: SOA; ZCLASS: IN
	// srv address
	{
		const unsigned int service_sz = 512;
		char service[service_sz];		memset(service, 0, service_sz);		char* pservice = (char*) service;

		VS_SimpleStr str = m_dns_service;
		str += m_dns_domain.m_str;
		VS_DDNS::eEncodeString(str.m_str, pservice, service_sz);
		unsigned int service_len = (unsigned int) strlen(service);		service_len++;		// add '\0'

		memcpy(buf + index, (void*)&service, service_len);		index += service_len;
	}

	if (!erase)	{ // create srv record
		memcpy(buf + index, "\x00\x21\x00\x01", 4);	index += 4;		// TYPE: SRV; CLASS: IN

		union __lxchg {
			unsigned long __long;
			unsigned char __char[4];
		};

		__lxchg ttl = { record.dwTtl }; // ttl
		buf[index++] = ttl.__char[3];
		buf[index++] = ttl.__char[2];
		buf[index++] = ttl.__char[1];
		buf[index++] = ttl.__char[0];

	} else if (erase == 1) {	// erase one srv record
			memcpy(buf + index, "\x00\x21\x00\xFE", 4);	index += 4;		// TYPE: SRV; CLASS: NONE (0xFE=254)
			memcpy(buf + index, "\x00\x00\x00\x00", 4);	index += 4;		// TTL: 0

	} else if (erase == 0xffffffff) {	// erase all srv records
			memcpy(buf + index, "\x00\x21\x00\xFF", 4);	index += 4;		// TYPE: SRV; CLASS: NONE (0xFE=254)
			memcpy(buf + index, "\x00\x00\x00\x00", 4);	index += 4;		// TTL: 0

			memcpy(buf + index, "\x00\x00", 2); index += 2;		// Data Length
			return index;
	}

	char srv_str[1024] = {};
	size_t srv_str_sz = 0;

	union __xchg {
		unsigned short __word;
		unsigned char  __char[2];
	};

	__xchg priority	= { record.Data.SRV.wPriority };
	__xchg weight	= { record.Data.SRV.wWeight };
	__xchg port		= { record.Data.SRV.wPort };

	srv_str[srv_str_sz++] = priority.__char[1];	// priority
	srv_str[srv_str_sz++] = priority.__char[0];
	srv_str[srv_str_sz++] = weight.__char[1];	// weight
	srv_str[srv_str_sz++] = weight.__char[0];
	srv_str[srv_str_sz++] = port.__char[1];		// port
	srv_str[srv_str_sz++] = port.__char[0];

	// Host
	{
		const unsigned int h_sz = 512;
		char h[h_sz];		memset(h, 0, h_sz);		char* ph = (char*) h;
		VS_SimpleStr host = record.Data.SRV.pNameTarget;
		if (host.m_str[host.Length()- 1] != '.') { host += "."; }
		VS_DDNS::eEncodeString(host.m_str, ph, h_sz);
		unsigned int h_len = (unsigned int) strlen(h);		h_len++;		// add '\0'

		memcpy(srv_str+srv_str_sz, (void*)&h, h_len);		srv_str_sz += h_len;
	}

	__xchg srv = { (unsigned short) srv_str_sz };
	buf[index++] = srv.__char[1];
	buf[index++] = srv.__char[0];

	memcpy(buf + index, srv_str, srv_str_sz); index += srv_str_sz;
	return index;
}

size_t VS_DDNS::eMakeDnsSrvQueryPacket(const char* query, char *buf, size_t buf_size, bool erase)
{
	if ((!m_enhanced) || (buf_size < 2048)) { return 0; }

	memset(buf, 0, buf_size);
	size_t index = 0;

	// header
	//buf[0] = (rand() % 26) + 65; buf[1] = (rand() % 26) + 65;	index += 2;		// ID
	memcpy(buf + index, "\x00\x00", 2);							index += 2;		// ID

	memcpy(buf + index, "\x01\x00", 2);							index += 2;		// QR = 0; Opcode = 0 (QUERY); Zero; RCODE = 0;
	memcpy(buf + index, "\x00\x01\x00\x00\x00\x00\x00\x00", 8);	index += 8;		// ZCount(1), PRCount(0), UPCount(0), ADCount(0)

	// srv address
	{
		const unsigned int service_sz = 512;
		char service[service_sz];		memset(service, 0, service_sz);		char* pservice = (char*) service;

		VS_DDNS::eEncodeString(query, pservice, service_sz);
		unsigned int service_len = (unsigned int) strlen(service);		service_len++;		// add '\0'

		memcpy(buf + index, (void*)&service, service_len);		index += service_len;
	}

	memcpy(buf + index, "\x00\x21\x00\x01", 4);	index += 4;		// TYPE: SRV; CLASS: IN

	return index;
}

unsigned long VS_DDNS::eDecodeDnsSrvQueryResponse(const char* buff, const unsigned int sz, const unsigned int sent_question_sz, std::vector<VS_DNSServerParam*> &out)
{
	if (!buff || !sz || !sent_question_sz)
		return 0;

	// 7 and 8 bytes - num of Answers RR
	int n = (int) buff[7];
	if (!n)
		return 0;

	unsigned int index = sent_question_sz;

	for(int i=0; i < n; i++)
	{
		char* p = (char*) buff+index;
		if (p[0]!=(char)0xc0 || p[1]!=(char)0x0c)	// start c0 0c
			break;
		index += 2;

		index += 14;	// 14 bytes: type, class, ttl,

		VS_DNSServerParam* server = new VS_DNSServerParam;

		// swap two bytes with port
		server->iPort = *(buff+index) << 8;
		server->iPort |= (unsigned char) *(buff+index+1);

		index += 2;

		// target: string
		if ( !eDecodeString(buff+index, server->iHost) || !server->iHost.Length() )
		{
			delete server;
			return 0;
		}
		index += (server->iHost.Length() + 1 + 1);	// length + last point + null

		out.push_back(server);
	}

	return (unsigned long) out.size();
}

unsigned long VS_DDNS::eDnsModifyRecordsInSet(PDNS_RECORD add_records, PDNS_RECORD erase_records)
{
	PDNS_RECORD _rec = erase_records;
	const size_t psize = 2048;
	char *packet = new char[psize];
	if (!packet) { return -1; }

	while (_rec) {
		size_t ssize = eMakeDnsSrvPacket(*_rec, packet, psize, true);
		eUpdateDnsRecord(packet, ssize);
		_rec = _rec->pNext;
	}

	_rec = add_records;
	while (_rec) {
		size_t ssize = eMakeDnsSrvPacket(*_rec, packet, psize, false);
		eUpdateDnsRecord(packet, ssize);
		_rec = _rec->pNext;
	}

	delete[] packet;
	return 0;
}

bool VS_DDNS::eEncodeString(const char* in, char* &out, const unsigned int out_sz)
{
	if (!m_inited) return false;
	unsigned int out_index = 0;

	unsigned int len = 0;		// length
	char* s = (char*) in;			// start
	char* e = 0;			// end

	unsigned int max_hops = 1000;
	while(true)
	{
		if ( !max_hops )
			break;
		max_hops--;

		char* e = (char*) strchr(s, '.');
		if ( !e )
		{
			out[out_index] = 0;					out_index += 1;
			return true;
		}

		len = (unsigned int) (e - s);
		if (!len)
			break;

		if ((len + out_index) > out_sz)
			break;

		out[out_index] = len;					out_index += 1;
		memcpy(out+out_index, s, len);			out_index += len;

		s += len;
		s++;	// skip '.'
	}
	return false;
}

bool VS_DDNS::eDecodeString(const char* in, VS_SimpleStr &out)
{
	if (!in)
		return false;

	char* p = (char*) in;

	int len = in[0];	p++;

	while(len)
	{
		out.Append(p, len);	p += len;
		len = *p;	p++;
		if (len)
			out.Append(".");
	}

	return out.Length() != 0;
}

bool VS_DDNS::IsValid()
{
	return (!m_inited || !m_enhanced || !m_dns_domain || !m_dns_service || !m_dns_request || (!m_dns_server_ip && m_dns_servers.size()<=0))? false: true;
}

bool VS_DDNS::CleanUp(const char* srv)
{
	char buff[2048] = {0};

	// start dns update
	DNS_RECORDA record =
	{
		0, this->GetRequestName(), DNS_TYPE_SRV, sizeof(DNS_RECORDA), {}, DNS_SRV_TTL, 0, {}
	};

	VS_SimpleStr NameTarget;
	int erase = 0;
	if (srv && *srv) {		// erase one record
		NameTarget = srv;
		if (!!NameTarget && NameTarget.m_str[NameTarget.Length()] != '.')
			NameTarget += ".";
		record.Data.SRV.pNameTarget = NameTarget.m_str;
		record.Data.SRV.wPort = 4307;
		erase = 1;
	} else {				// erase all records
		record.Data.SRV.pNameTarget = 0;
		record.Data.SRV.wPort = 0;
		erase = 0xffffffff;
	}

	size_t pcksize = this->eMakeDnsSrvPacket(record, buff, 2048, erase);

	return (pcksize && this->eUpdateDnsRecord(buff, pcksize))? true: false;
}

void VS_DDNS::SetSRV(const char* dns_domain, const char* dns_service)
{
	if (!dns_service || !*dns_service)
		m_dns_service = VS_DNSGetDefaultService(dns_domain);
	else
		m_dns_service = dns_service;

	m_dns_domain = dns_domain;
	m_dns_request = m_dns_service;
	m_dns_request += dns_domain;
}

void VS_DDNS::SetParams(const char* dns_server, const char* dns_domain, const char* dns_service)
{
	if (((!dns_server || !*dns_server) && m_dns_servers.size()<=0) ||
		!dns_domain || !*dns_domain)
		return ;

	unsigned long ip(0);
	if (VS_GetIpByHostName(dns_server, &ip))
		m_dns_server_ip = ip;

	SetSRV(dns_domain, dns_service);
}

bool VS_DDNS::SetParamsFromReg(const unsigned int index)
{
	char domain[256] = {0};
	char ddns_addr[256] = {0};
	char ddns_service[256] = {0};

	VS_SimpleStr key_name = CONFIGURATION_KEY;
	if  (index>0)
	{
		key_name += "\\DDNS";
		char tmp[128] = {0};
		key_name += std::to_string(index).c_str();
	}
	VS_RegistryKey key(false, key_name);
	if (!key.IsValid())
		return false;
	if (key.GetValue(domain, sizeof(domain) - 1, VS_REG_STRING_VT, "DDNS Disabled"))
		return true;
	memset(domain, 0, sizeof(domain));
	key.GetValue(domain, sizeof(domain) - 1, VS_REG_STRING_VT, "DDNS Domain");
	key.GetValue(ddns_addr, sizeof(ddns_addr) - 1, VS_REG_STRING_VT, "DDNS Host");
	key.GetValue(ddns_service, sizeof(ddns_service) - 1, VS_REG_STRING_VT, "DDNS Service");

	unsigned long priority(0);
	if (key.GetValue(&priority, sizeof(priority), VS_REG_INTEGER_VT, "DDNS Priority"))
		m_priority = (WORD)priority;

	unsigned long weight(0);
	if (key.GetValue(&weight, sizeof(weight), VS_REG_INTEGER_VT, "DDNS Weight"))
		m_weight = (WORD)weight;

	unsigned long update_srv_type(0);
	if (key.GetValue(&update_srv_type, sizeof(update_srv_type), VS_REG_INTEGER_VT, "DDNS UpdateSRVType"))
		m_update_srv_type = (eUpdateSRVType) update_srv_type;

	SetParams(ddns_addr, domain, ddns_service);

	return IsValid();
}

bool VS_DDNS::LoadPtr_IPHlp()
{
	if (sIPHlpLoaded) { return true; }
	sGetNetworkParams = 0;	sIPHlpLoaded = 0;
	sIPHlpLibHandle = LoadLibrary("iphlpapi.dll");
	if (sIPHlpLibHandle != INVALID_HANDLE_VALUE)
	{
		sGetNetworkParams =  (TYPE_GetNetworkParams)GetProcAddress( sIPHlpLibHandle, "GetNetworkParams");
		sIPHlpLoaded = sGetNetworkParams != 0;
		if (sIPHlpLoaded) { return true; }
	}
	if (sIPHlpLibHandle) { FreeLibrary( sIPHlpLibHandle ); }
	sIPHlpLibHandle = 0;	sGetNetworkParams = 0;	sIPHlpLoaded = false;
	return false;
}

bool VS_DDNS::ClearPtr_IPHlpd()
{
	if (sIPHlpLibHandle) { FreeLibrary( sIPHlpLibHandle ); }
	sIPHlpLibHandle = 0;	sGetNetworkParams = 0;	sIPHlpLoaded = false;
	return true;
}

bool VS_DDNS::LoadDNSServers()
{
	if (!sGetNetworkParams)
		return false;

	FIXED_INFO* info = 0;
	ULONG buff_sz = 0;
	if (sGetNetworkParams(info, &buff_sz) == ERROR_BUFFER_OVERFLOW) {
		info = (FIXED_INFO*) new char[buff_sz];
	}

	if (!info || !buff_sz || sGetNetworkParams(info, &buff_sz) != ERROR_SUCCESS)
		return false;

	m_dns_servers.clear();

	_IP_ADDR_STRING* ptr = &(info->DnsServerList);
	while(ptr)
	{
		unsigned long ip = 0;
		if (VS_GetIpByHostName(ptr->IpAddress.String, &ip) && ip)
			m_dns_servers.push_back(ip);

		ptr = ptr->Next;
	}
	delete[] (char*)info;

	return m_dns_servers.size() > 0;
}

bool VS_DDNS::InitDNSServers()
{
	LoadPtr_IPHlp();
	bool ret = LoadDNSServers();
	ClearPtr_IPHlpd();
	return ret;
}

VS_SimpleStr VS_DDNS::GetDNSDomain() const
{
	return m_dns_domain;
}

void VS_DDNS::SetDNSDomain(const char* str)
{
	m_dns_domain = str;

	m_dns_request = m_dns_service;
	m_dns_request += str;
}

WORD VS_DDNS::GetPriority() const
{
	return m_priority;
}

WORD VS_DDNS::GetWeight() const
{
	return m_weight;
}

void VS_DDNS::SetUpdateSRVType(VS_DDNS::eUpdateSRVType type)
{
	m_update_srv_type = type;
}

VS_DDNS::eUpdateSRVType VS_DDNS::GetUpdateSRVType() const
{
	return m_update_srv_type;
}

VS_SimpleStr VS_DDNS::GetDNSService() const
{
	return m_dns_service;
}

#endif
