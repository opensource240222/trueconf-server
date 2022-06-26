/**
 **************************************************************************
 * \file VS_ServerList.cpp
 * (c) 2002-2008 Visicron Inc.  http://www.visicron.net/
 * \brief Servers list methods
 *
 * \b Project Client
 * \author SmirnovK
 * \date 21.02.2008
 *
 * $Revision: 22 $
 *
 * $History: VS_ServerList.cpp $
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 18.10.11   Time: 20:24
 * Updated in $/VSNA/VSClient
 * -bugfix#8177
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 1.12.10    Time: 15:35
 * Updated in $/VSNA/VSClient
 * - diagnose the best server
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 25.11.10   Time: 20:16
 * Updated in $/VSNA/VSClient
 * - new servers minimal time algorithm
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 29.04.10   Time: 19:27
 * Updated in $/VSNA/VSClient
 * - bugfix#7275
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 9.04.10    Time: 17:44
 * Updated in $/VSNA/VSClient
 * - bugfix#7187
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 1.07.09    Time: 21:11
 * Updated in $/VSNA/VSClient
 * - discovery every day in idle mode
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 9.10.08    Time: 19:29
 * Updated in $/VSNA/VSClient
 * - some improvements
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 9.10.08    Time: 14:22
 * Updated in $/VSNA/VSClient
 * - bugfix with registry (old server deleting)
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 2.10.08    Time: 21:27
 * Updated in $/VSNA/VSClient
 * - geogrphic impruvements (enh #4699)
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 25.07.08   Time: 14:52
 * Updated in $/VSNA/VSClient
 * - logging app_ID added
 * - logging multi_conf
 * - bug 4602 fixed
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 17.07.08   Time: 21:21
 * Updated in $/VSNA/VSClient
 * - geographic associating
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 15.07.08   Time: 18:12
 * Updated in $/VSNA/VSClient
 * - LogConfStart LogConfEnd time corrected
 * - SrvRespond added to registry
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 14.07.08   Time: 19:20
 * Updated in $/VSNA/VSClient
 * SetEvent(srv_resp) added
 *
 * *****************  Version 9  *****************
 * User: Dront78      Date: 14.05.08   Time: 11:09
 * Updated in $/VSNA/VSClient
 * - random 50msec correction added for prevent alphanumeric sorting with
 * equal time values
 *
 * *****************  Version 8  *****************
 * User: Dront78      Date: 13.05.08   Time: 20:36
 * Updated in $/VSNA/VSClient
 * - current server selection algorithm changed
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 18.04.08   Time: 16:58
 * Updated in $/VSNA/VSClient
 * - bugfix with connect to server without BS
 * - improvement: if discovery found new server connect to it
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 10.04.08   Time: 19:48
 * Updated in $/VSNA/VSClient
 * - remove servers older than 7 days
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 9.04.08    Time: 19:33
 * Updated in $/VSNA/VSClient
 * - auto discovery every 600 sec
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 3.04.08    Time: 14:21
 * Updated in $/VSNA/VSClient
 * - right current server in registry
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 31.03.08   Time: 17:54
 * Updated in $/VSNA/VSClient
 * - super discovery
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 24.03.08   Time: 21:24
 * Updated in $/VSNA/VSClient
 * - online client onlly after check login status
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 26.02.08   Time: 16:54
 * Created in $/VSNA/VSClient
 * - new servers coonect shceme
 *
 ****************************************************************************/

#include "VS_ServerList.h"
#include "VS_ApplicationInfo.h"
#include "VS_Dmodule.h"
#include "VSProxySettings.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "../std/cpplib/VS_Utils.h"
#include "../net/EndpointRegistry.h"
#include "transport/Client/VS_AutoDiscovery.h"
#include "transport/Client/VS_TransportClient.h"
#include <time.h>
#include "VS_ConnectionsCheck.h"

#include <Windows.h>
#include <mmsystem.h>

#include <string>
#include <vector>

#define TIME_REFRESH_DNS	60*10				/// 10 min
#define TIME_DELETE_SERVER	60*60*24*7			/// 7 days
#define TIME_CHECK_SERVER	60*60*24			/// 24 hours
#define TIME_CHECK_CHECKED	60*15				/// 15 min

struct VS_ClientServerInfo
{
	VS_ClientServerInfo() {
		m_time[0] = m_time[1] = m_time[2] = m_time[3] = 0;
		m_resp = -1;
	}
	VS_SimpleStr	m_name;
	VS_SimpleStr	m_serviceName;
	DWORD			m_time[4];
	DWORD			m_resp;
	static void* Factory(const void* p) {
		return p ? new VS_ClientServerInfo(*(const VS_ClientServerInfo*)p) : NULL;
	}
	static void Destructor(void* p)	{if (p) delete (VS_ClientServerInfo*)p;}
};

const char *reg_events[4] = {"DnsUpdate", "ServerUpdate", "LastConnect", "LastCheck"};

VS_ServerList::VS_ServerList()
{
	m_LastDNSTime = 0;
	VS_RegistryKey key(true, REG_Servers);
	key.GetValue(&m_LastDNSTime, 4, VS_REG_INTEGER_VT, reg_events[0]);
	DWORD ct = (DWORD)time(0);
	m_LastDNSTime = std::min(ct, m_LastDNSTime);
	m_map.SetPredicate(VS_SimpleStr::Predicate);
	m_map.SetKeyFactory(VS_SimpleStr::Factory, VS_SimpleStr::Destructor);
	m_map.SetDataFactory(VS_ClientServerInfo::Factory, VS_ClientServerInfo::Destructor);
	Reset();
}

VS_ServerList::~VS_ServerList()
{
	if (!!m_service)
		RemoveOldServers();
	m_map.Clear();
}

void VS_ServerList::Reset()
{
	m_OrdServers.clear();
	std::vector<VS_ClientServerInfo *> ss;
	for (VS_Map::Iterator it = m_map.Begin(); it != m_map.End(); ++it) {
		VS_ClientServerInfo * info = (VS_ClientServerInfo *)(*it).data;
		ss.push_back(info);
	}
	int s = ss.size();
	for (int j = 0; j < s; j++) {
		DWORD t_resp = -1;
		int first = 0;
		for (UINT i = 0; i < ss.size(); i++) {
			// find min m_resp
			if (t_resp > ss[i]->m_resp) {
				t_resp = ss[i]->m_resp;
				first = i;
			}
		}
		if (!m_service || m_service==ss[first]->m_serviceName)
			m_OrdServers.push_back(ss[first]->m_name);
		ss.erase(ss.begin() + first);
	}
	m_curr = m_OrdServers.begin();
	for (unsigned i = 0; i < m_OrdServers.size(); i++) {
		DTRACE(VSTM_PRTCL, "server (%s) num %d", m_OrdServers[i].m_str, i);
	}
}

void VS_ServerList::Reload(const char* service)
{
	m_service = service;
	VS_RegistryKey key(true, REG_Servers, false);
	VS_RegistryKey server;

	std::vector<std::string> to_delete;
	key.ResetKey();
	m_map.Clear();
	while (key.NextKey(server)) {
		if (!ReadServer(server.GetName(), &server))
			to_delete.emplace_back(server.GetName());
	}
	for (const auto& x: to_delete)
		key.RemoveKey(x);

	VS_SimpleStr srv(256);
	if (!VS_ReadAS(srv) || !IsInList(srv)) {
		if (Discovery(srv) >=0)
			VS_WriteAS(srv);
	}

	Reset();
}

/*
	0 - no servers
	1  - next server
	-1  - end of list
*/
long VS_ServerList::Next(VS_SimpleStr &name)
{
	if (!m_service || m_map.Size()==0 || m_curr==m_OrdServers.end()) {
		if ((DWORD)time(0) - m_LastDNSTime > TIME_REFRESH_DNS) {
			if (Discovery(name)==1) {
				return 1;
			}
		}
		Reset();
		return -1;
	}
	VS_SimpleStr NextName;
	Current(NextName);
	++m_curr;
	if (m_OrdServers.size() > 1) {
		if (NextName==name) { // obtained the same server
			if (Current(NextName))
				++m_curr;
		}
	}
	name = NextName;
	return 1;
}


bool VS_ServerList::Current(VS_SimpleStr &name)
{
	if (m_OrdServers.size() && (m_curr != m_OrdServers.end())) {
		name = *m_curr;
		return true;
	}
	return false;
}


bool VS_ServerList::ReadServer(const char* server, VS_RegistryKey * key)
{
	unsigned n_ctcp = net::endpoint::GetCountConnectTCP(server);
	for (unsigned i = 1; i <= n_ctcp; )
	{
		auto tcp = net::endpoint::ReadConnectTCP(i, server);
		if (!tcp || tcp->host.empty() || tcp->port == 0 || tcp->protocol_name.empty())
		{
			net::endpoint::DeleteConnectTCP(i, server);
			--n_ctcp;
		}
		else
			i++;
	}
	if (n_ctcp == 0)
	{
		net::endpoint::Remove(server);
		return false;
	}

	VS_ClientServerInfo info;
	info.m_name = server;
	key->GetValue(&info.m_time[0], 4, VS_REG_INTEGER_VT, reg_events[0]);
	key->GetValue(&info.m_time[1], 4, VS_REG_INTEGER_VT, reg_events[1]);
	key->GetValue(&info.m_time[2], 4, VS_REG_INTEGER_VT, reg_events[2]);
	key->GetValue(&info.m_time[3], 4, VS_REG_INTEGER_VT, reg_events[3]);
	key->GetValue(&info.m_resp, 4, VS_REG_INTEGER_VT, "ResponseTime");
	info.m_serviceName.Resize(256);
	key->GetValue(info.m_serviceName, 255, VS_REG_STRING_VT, "Service");
	m_map.Assign(server, &info);
	return true;
}

void VS_ServerList::RemoveOldServers()
{
	DWORD ct = (DWORD)time(NULL);
	for (VS_Map::Iterator it = m_map.Begin(); it!=m_map.End();) {
		VS_ClientServerInfo* info = (VS_ClientServerInfo*)(*it).data;
		DWORD ht = info->m_time[0];
		if (ht < info->m_time[1])
			ht = info->m_time[1];
		if (ht < info->m_time[2])
			ht = info->m_time[2];
		if (ht < info->m_time[3])
			ht = info->m_time[3];
		if (ht+TIME_DELETE_SERVER < ct) {
			VS_RegistryKey key(true, REG_Servers, false);
			key.RemoveKey(info->m_name.m_str);
			net::endpoint::Remove(info->m_name.m_str);
			DTRACE(VSTM_PRTCL, "Remove old server %s", info->m_name.m_str);
			it = m_map.Erase(it);
		}
		else {
			++it;
		}
	}
}

bool VS_ServerList::RenameServer(const char* olds, const char* news)
{
	if (!olds || !*olds ||!news ||!*news)
		return false;
	VS_RegistryKey key(true, REG_Servers, false);
	if (key.RenameKey(olds, news)) {
		DTRACE(VSTM_PRTCL, "Server %s renamed to %s", olds, news);
		VS_Map::Iterator it = m_map.Find(olds);
		if (it!=m_map.End()) {
			VS_ClientServerInfo info = *(VS_ClientServerInfo*)(*it).data;
			info.m_name = news;
			m_map.Erase(it);
			m_map.Assign(news, &info);
		}
		Reset();
		return true;
	}
	return false;
}


void VS_ServerList::SetEvent(const char* server, ServerEvents_Time ev, unsigned long srv_resp, const char* service)
{
	if (!server || !*server)
		return;
	VS_SimpleStr name(REG_Servers);
	name+="\\";
	name+=server;
	VS_RegistryKey key(true, name, false, true);
	DWORD ev_time = (DWORD)time(NULL);
	key.SetValue(&ev_time, 4, VS_REG_INTEGER_VT, reg_events[ev]);
	if (service && *service)
		key.SetString(service, "Service");
	VS_Map::Iterator it = m_map.Find(server);
	if (it!=m_map.End()) { // found
		((VS_ClientServerInfo *)(*it).data)->m_time[ev] = ev_time;
		if (service && *service)
			((VS_ClientServerInfo *)(*it).data)->m_serviceName = service;
		if (ev == SRVT_LASTCHECK ) {
			DWORD &resp = ((VS_ClientServerInfo *)(*it).data)->m_resp;
			if (resp==-1)
				resp = srv_resp;
			else
				resp = srv_resp < resp ? (srv_resp*3 + resp)/4 : (srv_resp + 3*resp)/4;
			key.SetValue(&resp, 4, VS_REG_INTEGER_VT, "ResponseTime");
		}
	}
	else {
		if (ev == SRVT_LASTCHECK )
			key.SetValue(&srv_resp, 4, VS_REG_INTEGER_VT, "ResponseTime");
		ReadServer(server, &key);
		Reset();
	}
}

bool VS_ServerList::IsInList(const char* server)
{
	if (!server || !*server)
		return false;
	VS_Map::Iterator it = m_map.Find(server);
	return it!=m_map.End();
}

// it returns only valid servers
long VS_ServerList::GetValidServers(VS_SimpleStr* &servers)
{
	unsigned long size = m_OrdServers.size();
	if (size) {
		unsigned long i = 0;
		servers = new VS_SimpleStr[size];
		for (unsigned i = 0; i < size; i++)
			servers[i] = m_OrdServers[i];
	}
	return size;
}


/**
****************************************************************************
* Found broker info in specified DNS
*
* \param	install		- install found broker
* \date    28-03-2008
******************************************************************************/
long VS_ServerList::Discovery(VS_SimpleStr &server)
{
	DTRACE(VSTM_PRTCL, "Run Discovery");

	m_LastDNSTime = (DWORD)time(0);
	VS_RegistryKey key(true, REG_Servers, false, true);
	key.SetValue(&m_LastDNSTime, 4, VS_REG_INTEGER_VT, reg_events[0]);

	VS_SimpleStr *servers = 0;
	unsigned int num = 0;
	if (!VS_AutoDiscoveryServers(m_service, servers, num, true))
		return -1;
	// update time for servers
	std::vector<VS_SimpleStr *> freshservers;
	for (unsigned int i = 0; i<num; i++) {
		if (!IsInList(servers[i])) {
			DTRACE(VSTM_PRTCL, "Discovered FRESH server %s!", servers[i].m_str);
			freshservers.push_back(servers + i);
		}
		SetEvent(servers[i], SRVT_DNSUPDATE, -1, m_service);
		// set protocol for all found servers
		VSProxySettings::UpdateEnpointsProtocol(servers[i]);
	}
	// check only new servers, no more than 30 sec since discovery starting
	unsigned int fnum = freshservers.size();
	for (unsigned int i = 0; i<fnum; i++) {
		CheckServerDistance(*freshservers[i], 3000);
		DWORD time_elapsed = (DWORD)time(0) - m_LastDNSTime;
		if (time_elapsed > 30)
			break;
	}

	if (!!m_service) {//TCO mode or fixed domain mode
		Reset();
		Current(server);
	}
	else {
		if (num)
			server = servers[0];
	}

	if (num) {
		delete[] servers;
		return 1;
	}
	else
		return 0;
}

/**
****************************************************************************
* ping server, resort ConnectTCP for server and mesure time responce
* \date    16-07-2008
******************************************************************************/
void VS_ServerList::CheckServerDistance(const char* server, int timeout)
{
	if (!server)
		return;

	VS_Map::Iterator it = m_map.Find(server);
	if (it!=m_map.End()) {
		VS_ClientServerInfo * info = (VS_ClientServerInfo *)(*it).data;
		DWORD dt =(DWORD)time(0) - info->m_time[SRVT_LASTCHECK];
		if (info->m_resp!= -1 && dt < TIME_CHECK_CHECKED) {
			DTRACE(VSTM_PRTCL, "Skip checking of %s, checked %d sec ago", server, dt);
			return;
		}
	}

	DWORD t1 = timeGetTime();
	unsigned long srv_resp = -1;
	VS_ConnectionsCheckFast(server, timeout, &srv_resp);
	if (srv_resp!=-1)
		SetEvent(server, SRVT_LASTCHECK, srv_resp);
	DTRACE(VSTM_PRTCL, "Check connects of Server %s, t=%d, resp=%d", server, timeGetTime()-t1, srv_resp);
	Sleep(100);
}

/**
****************************************************************************
* decide if server need in connection check
* \date    01-10-2008
******************************************************************************/
void VS_ServerList::CheckServers(const char* CurrServer)
{
	DWORD ct = (DWORD)time(0);
	VS_SimpleStr server;
	if (ct > m_LastDNSTime + TIME_CHECK_SERVER)
		if (Discovery(server)==1)
			return;

	unsigned int size = m_map.Size();
	if (size==0)
		return;
	size = VS_GenKeyByMD5()%size;
	VS_Map::Iterator it = m_map.Begin();
	for (; it != m_map.End() && size; ++it, --size);

	VS_ClientServerInfo * info = (VS_ClientServerInfo *)(*it).data;;
	// not tested at all
	bool docheck = info->m_resp==-1;

	if (!docheck) {
		// test checked more than 1 day before
		docheck = ct > info->m_time[SRVT_LASTCHECK] + TIME_CHECK_SERVER;
	}
	if (!docheck) {
		if (info->m_name==CurrServer) {
			// test current server after connect (reconnect?)
			docheck = info->m_time[SRVT_LASTCHECK] < info->m_time[SRVT_LASTCONNET];
		}
		else {
			it = m_map.Find(CurrServer);
			if (it!=m_map.End()) {
				VS_ClientServerInfo * curr = (VS_ClientServerInfo *)(*it).data;
				if (info->m_resp < curr->m_resp) {
					// if found server closer than current one, check it after current server connect
					docheck = info->m_time[SRVT_LASTCHECK] < curr->m_time[SRVT_LASTCONNET];
				}
			}
		}
	}
	DTRACE(VSTM_PRTCL, "CheckServers: select %s, docheck=%d", info->m_name.m_str, docheck);
	if (docheck)
		CheckServerDistance(info->m_name, 5000);
}

/**
****************************************************************************
* return the best server (by ping time, if better than current)
* \date    17-07-2008
******************************************************************************/
bool VS_ServerList::GetTheBest(VS_SimpleStr &server)
{
	if (!m_service)
		return false;

	VS_ClientServerInfo * best = 0;

	if (!server || m_map.Find(server)==m_map.End()) {
		DWORD bestResp = -1;
		for (VS_Map::Iterator it = m_map.Begin(); it != m_map.End(); ++it) {
			VS_ClientServerInfo * info = (VS_ClientServerInfo *)(*it).data;
			if (info->m_resp < bestResp) {
				bestResp = info->m_resp;
				best = info;
			}
		}
	}
	else {
		VS_Map::Iterator it = m_map.Find(server);
		VS_ClientServerInfo * curr = (VS_ClientServerInfo *)(*it).data;
		DWORD bestResp = curr->m_resp;
		for (it = m_map.Begin(); it != m_map.End(); ++it) {
			VS_ClientServerInfo * info = (VS_ClientServerInfo *)(*it).data;
			// check only servers refreshed after 'server' connect
			if (info->m_time[SRVT_LASTCHECK] > curr->m_time[SRVT_LASTCONNET]) {
				if (info->m_resp < bestResp) {
					bestResp = info->m_resp;
					best = info;
				}
			}
		}
	}
	if (best) {
		server = best->m_name;
		return true;
	}
	else
		return false;
}
