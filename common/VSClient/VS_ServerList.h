/**
 **************************************************************************
 * \file VS_ServerList.h
 * (c) 2002-2008 Visicron Inc.  http://www.visicron.net/
 * \brief Servers list
 *
 * \b Project Client
 * \author SmirnovK
 * \date 21.02.2008
 *
 * $Revision: 10 $
 *
 * $History: VS_ServerList.h $
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 18.10.11   Time: 20:24
 * Updated in $/VSNA/VSClient
 * -bugfix#8177
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 2.10.08    Time: 21:27
 * Updated in $/VSNA/VSClient
 * - geogrphic impruvements (enh #4699)
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 17.07.08   Time: 21:21
 * Updated in $/VSNA/VSClient
 * - geographic associating
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 14.07.08   Time: 19:20
 * Updated in $/VSNA/VSClient
 * SetEvent(srv_resp) added
 *
 * *****************  Version 6  *****************
 * User: Dront78      Date: 13.05.08   Time: 20:36
 * Updated in $/VSNA/VSClient
 * - current server selection algorithm changed
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 18.04.08   Time: 16:58
 * Updated in $/VSNA/VSClient
 * - bugfix with connect to server without BS
 * - improvement: if discovery found new server connect to it
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 10.04.08   Time: 19:48
 * Updated in $/VSNA/VSClient
 * - remove servers older than 7 days
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 9.04.08    Time: 19:33
 * Updated in $/VSNA/VSClient
 * - auto discovery every 600 sec
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 31.03.08   Time: 17:54
 * Updated in $/VSNA/VSClient
 * - super discovery
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 26.02.08   Time: 16:54
 * Created in $/VSNA/VSClient
 * - new servers coonect shceme
 *
 ****************************************************************************/

#ifndef VS_SERVER_LIST_H
#define VS_SERVER_LIST_H

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "..\std\cpplib\VS_SimpleStr.h"
#include "..\std\cpplib\VS_Map.h"
#include <vector>

class VS_RegistryKey;

/**
**************************************************************************
 * \brief Describe recievrs parameters need for connect
 ****************************************************************************/
class VS_ServerList
{
	VS_Map				m_map;
	VS_SimpleStr		m_service;
	unsigned long		m_LastDNSTime;
	std::vector<VS_SimpleStr> m_OrdServers;
	std::vector<VS_SimpleStr> m_CheckServers;
	std::vector<VS_SimpleStr>::iterator m_curr;
public:
	enum ServerEvents_Time
	{
		SRVT_DNSUPDATE,
		SRVT_SERVERUPDATE,
		SRVT_LASTCONNET,
		SRVT_LASTCHECK
	};
	VS_ServerList();
	~VS_ServerList();
	void Reload(const char* service);
	long Next(VS_SimpleStr &name);
	bool RenameServer(const char* olds, const char* news);
	void SetEvent(const char* server, ServerEvents_Time ev, unsigned long srv_resp = -1, const char* service = 0);
	bool IsInList(const char* server);
	long GetValidServers(VS_SimpleStr* &servers);
	long Discovery(VS_SimpleStr &server);
	void CheckServerDistance(const char* server, int timeout);
	void CheckServers(const char* CurrServer);
	bool GetTheBest(VS_SimpleStr &server);
private:
	void Reset();
	bool Current(VS_SimpleStr &name);
	bool ReadServer(const char* server, VS_RegistryKey * key);
	void RemoveOldServers();
};

#endif /*VS_SERVER_LIST_H*/
