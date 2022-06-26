#pragma once

#include "../../common/std/cpplib/VS_Protocol.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../../common/std/cpplib/VS_Map.h"
#include "../../ServerServices/Types/VS_BrokerInfo.h"
//
#include <mutex>
#include <list>
#include <map>




class VS_AppServerData
{
	///VS_SimpleStr	m_RS;
	VS_Map			m_servers;
	VS_Container	m_netinfo;

	VS_Map			m_cids;
	bool GetInstance(const char * id, VS_ServerInfo *&info, bool set = false);
protected:
	std::map<VS_SimpleStr,std::list<std::string>>	m_bs_by_dns;
	//std::map<VS_SimpleStr,std::list<std::string>>	m_domains_for_rs;

	std::recursive_mutex m_lock;
public:
	VS_AppServerData();
	bool Set(const char * id, bool state = false, VS_ServerTypes type = ST_UNKNOWN);
	bool GetState(const char * id);
	unsigned long GetDistance(const char * id);
	bool IsPingDistance(const char * id, unsigned long currtime);
	bool SetDistance(const char * id, unsigned long distance, unsigned long currtime);
	bool SetNetConfig(const char * id, const void* buff, unsigned long size);
	bool GetNetConfig(const char * id, VS_BinBuff &cfg);
	bool SetNetInfo(const char * id, const char * service);
	bool GetNetInfo(VS_Container &cfg);

	bool AddCid(const char * id);
	bool DelCid(const char * id);
	long GetCids(VS_SimpleStr* &cids);

	//void SetRS(const char * id);
	//bool GetRS(VS_SimpleStr	&server);
	bool IsBSList(const char* id);
	//void SetBS(const char * id);
	bool GetFirstBS(VS_SimpleStr &server);
	bool GetBSByDomain(const VS_SimpleStr &domain, VS_SimpleStr& bs);
	void SetBSList(VS_Container&cnt);
	static VS_ServerTypes DetermineType(const char * id);
};

extern VS_AppServerData*	g_appServer;
