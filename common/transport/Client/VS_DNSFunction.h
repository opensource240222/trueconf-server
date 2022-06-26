/**************************************************************************
 * \file VS_DNSFunction.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief \b Project DNSFunction
 *
 * \author ktrushnikov
 * \date 03.11.05
 ****************************************************************************/
#pragma once
/****************************************************************************
 * Includes
 ****************************************************************************/

#include "VS_DNSServerParam.h"
#include "../../acs/connection/VS_ConnectionUDP.h"
#include "../../std/cpplib/VS_SimpleStr.h"
#include "net/DNSUtils/VS_DNSUtils.h"

#include <Windows.h>
#include <WinDNS.h>
#include <IPHlpApi.h>

#include <vector>

/****************************************************************************
 * Const
 ****************************************************************************/

typedef DNS_STATUS (__stdcall *TYPE_DnsQuery)(  PCSTR lpstrName,  WORD wType,
										 DWORD fOptions,  PIP4_ARRAY aipServers,
										 PDNS_RECORD* ppQueryResultsSet,
										 PVOID* pReserved);

typedef void (__stdcall *TYPE_DnsRecordListFree)(PDNS_RECORD pRecordList, DNS_FREE_TYPE FreeType);

typedef DNS_STATUS (__stdcall *TYPE_DnsModifyRecordsInSet)(	PDNS_RECORD pAddRecords, PDNS_RECORD pDeleteRecords,
															DWORD Options, HANDLE hContext, PVOID pExtraList,
															PVOID pReserved);

typedef DNS_STATUS (__stdcall *TYPE_DnsAcquireContextHandle)(DWORD CredentialFlags, PVOID Credentials, HANDLE* ContextHandle);

#define DNS_PRIORITY_ONLINE 0
#define DNS_PRIORITY_OFFLINE 0xFFFF
extern unsigned long DNS_SRV_TTL;
#define DNS_DEFAULT_WEIGHT 0
#define DNS_DEFAULT_SERVER_PORT 4307

struct VS_NaptrServices
{
	std::string service;
	std::string replacement;
};

/******************************************************************************
 * \brief Функции для определения библиотеки.
 *****************************************************************************/
bool VS_DNSLoadPtr();
bool VS_DNSClearPtr();



class VS_DDNS			// Dynamic DNS updates
{
public:
	VS_DDNS();
	virtual ~VS_DDNS();
	void SetSRV(const char* dns_domain, const char* dns_service);
	void SetParams(const char* dns_server, const char* dns_domain, const char* dns_service = net::dns::VCS2_DDNS_SRV);
	bool SetParamsFromReg(const unsigned int index = 0);
	bool GetRecords(PDNS_RECORD &records, WORD type = DNS_TYPE_SRV);
	bool GetRecords(PDNS_RECORD &records, const char *dns_request, WORD type = DNS_TYPE_SRV);
	bool UpdateRecords(PDNS_RECORD add_records, PDNS_RECORD erase_records, bool UseWindowsFunc = false);
	bool FreeRecords(PDNS_RECORD records);
	char *GetRequestName();

	VS_SimpleStr GetDNSDomain() const;
	void SetDNSDomain(const char* str);
	VS_SimpleStr GetDNSService() const;

	WORD GetPriority() const;
	WORD GetWeight() const;

	enum eUpdateSRVType
	{
		e_none = -1,
		e_ip,
		e_hostname
	};

	void SetUpdateSRVType(eUpdateSRVType type);
	eUpdateSRVType GetUpdateSRVType() const;

private:
	bool			m_inited;
	bool			m_enhanced;
	HANDLE			m_handle;
	VS_SimpleStr	m_dns_domain;
	VS_SimpleStr	m_dns_service;
	VS_SimpleStr	m_dns_request;
	unsigned long	m_dns_server_ip;
	WORD			m_priority;
	WORD			m_weight;
	eUpdateSRVType m_update_srv_type;


	// enhanced section for manual DNS update
	VS_ConnectionUDP me_dns_socket;
	char			 me_bind_host[256];
	unsigned short	 me_bind_port;
	unsigned long	eDnsModifyRecordsInSet(PDNS_RECORD add_records, PDNS_RECORD erase_records);

	bool			eEncodeString(const char* in, char* &out, const unsigned int out_sz);
	bool			eDecodeString(const char* in, VS_SimpleStr &out);

	unsigned long	eDecodeDnsSrvQueryResponse(const char* buff, const unsigned int sz, const unsigned int sent_question_sz, std::vector<VS_DNSServerParam*> &out);

public:
	bool			eUpdateDnsRecord(char* send, size_t send_sz);
	size_t			eMakeDnsSrvPacket(DNS_RECORDA &record, char *buf, size_t buf_size, unsigned int erase = false);

	bool			eSendDnsSrvQuery(char* send, size_t send_sz, std::vector<VS_DNSServerParam*> &out);
	size_t			eMakeDnsSrvQueryPacket(const char* query, char *buf, size_t buf_size, bool erase = false);

	bool			IsValid();
	bool			CleanUp(const char* srv = 0);

private:
	typedef DWORD (__stdcall *TYPE_GetNetworkParams)(PFIXED_INFO pFixedInfo, PULONG pOutBufLen);

	static TYPE_GetNetworkParams			sGetNetworkParams;
	static bool								sIPHlpLoaded;
	static HINSTANCE						sIPHlpLibHandle;

	static bool		LoadPtr_IPHlp();
	static bool		ClearPtr_IPHlpd();

	bool	LoadDNSServers();

	std::vector<unsigned long>		m_dns_servers;

public:
	bool	InitDNSServers();
};
