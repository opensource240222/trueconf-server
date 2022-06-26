#include "VS_AppServerData.h"
#include "std/debuglog/VS_Debug.h"
#include "net/EndpointRegistry.h"

#define DEBUG_CURRENT_MODULE VS_DM_CLUSTS

VS_AppServerData*	g_appServer=0;

//////// VS_AppServerData ////////////
VS_AppServerData::VS_AppServerData()
{
	m_servers.SetPredicate(VS_SimpleStr::Predicate);
	m_servers.SetKeyFactory(VS_SimpleStr::Factory, VS_SimpleStr::Destructor);
	m_servers.SetDataFactory(VS_ServerInfo::Factory, VS_ServerInfo::Destructor);
	m_cids.SetPredicate(VS_SimpleStr::Predicate);
	m_cids.SetKeyFactory(VS_SimpleStr::Factory, VS_SimpleStr::Destructor);
}

bool VS_AppServerData::GetInstance(const char * id, VS_ServerInfo *&info, bool set)
{
	if (!id || !*id)
		return false;
	VS_Map::Iterator it = m_servers.Find(id);
	if (it!=m_servers.End()) {
		info = (VS_ServerInfo *)(*it).data;
		return true;
	}
	else {
		if (set) {
			VS_ServerInfo si;
			if (m_servers.Insert((char*)id, &si)) {
				return GetInstance(id, info);
			}
			else
				return false;
		}
		else
			return false;
	}
}

bool VS_AppServerData::Set(const char * id, bool state, VS_ServerTypes type)
{
	std::lock_guard<std::recursive_mutex> lock(m_lock);
	VS_ServerInfo *info = 0;
	if (GetInstance(id, info, true)) {
		info->m_id = id;
		info->m_type = type==ST_UNKNOWN ? DetermineType(id) : type;
		if (state)
			info->m_lastConnected = std::chrono::system_clock::now();
		info->m_status = state;
		return true;
	}
	return false;
}

bool VS_AppServerData::GetState(const char * id)
{
	std::lock_guard<std::recursive_mutex> lock(m_lock);
	VS_ServerInfo *info = 0;
	return GetInstance(id, info) && info->m_status;
}


unsigned long VS_AppServerData::GetDistance(const char * id)
{
	std::lock_guard<std::recursive_mutex> lock(m_lock);
	VS_ServerInfo *info = 0;
	unsigned long res = -1;
	if (GetInstance(id, info)) {
		res = info->m_Distance;
		if (res != -1)
			res+= rand()&7; // add random number < 8 mc to allow switching between BS with the same distance
	}
	return res;
}


bool VS_AppServerData::IsPingDistance(const char * id, unsigned long currtime)
{
	std::lock_guard<std::recursive_mutex> lock(m_lock);
	VS_ServerInfo *info = 0;
	if (GetInstance(id, info) && info->m_status) { // connected
		if (info->m_pingTime == 0 || currtime-info->m_pingTime > 3*60*1000) {
			return true;
		}
	}
	return false;
}


bool VS_AppServerData::SetDistance(const char * id, unsigned long distance, unsigned long currtime)
{
	std::lock_guard<std::recursive_mutex> lock(m_lock);
	VS_ServerInfo *info = 0;
	if (GetInstance(id, info)) {
		if (info->m_Distance == -1) {
			info->m_Distance = distance;
		}
		else if (distance != -1) {
			if (info->m_Distance > distance)
				info->m_Distance = (info->m_Distance + 3*distance + 2) >> 2;
			else
				info->m_Distance = (3*info->m_Distance + distance + 2) >> 2;
		}
		info->m_pingTime = currtime;
		return true;
	}
	return false;
}


bool VS_AppServerData::SetNetConfig(const char * id, const void* buff, unsigned long size)
{
	std::lock_guard<std::recursive_mutex> lock(m_lock);
	VS_ServerInfo *info = 0;
	if (GetInstance(id, info, true)) {
		info->m_id = id;
		info->m_netConfig.Set(buff, size);
		return true;
	}
	return false;
}


bool VS_AppServerData::GetNetConfig(const char * id, VS_BinBuff &cfg)
{
	std::lock_guard<std::recursive_mutex> lock(m_lock);
	VS_ServerInfo *info = 0;
	if (GetInstance(id, info)) {
		cfg = info->m_netConfig;
		return true;
	}
	return false;
}

bool VS_AppServerData::SetNetInfo(const char * id, const char * service)
{
	std::vector<uint8_t> data = net::endpoint::Serialize(true, id, false);
	if (data.empty())
		data = net::endpoint::Serialize(false, id, false);
	if (data.empty())
		return false;

	std::lock_guard<std::recursive_mutex> lock(m_lock);

	SetNetConfig(id, data.data(), data.size());

	m_netinfo.Clear();
	m_netinfo.AddValue(METHOD_PARAM, UPDATECONFIGURATION_METHOD);
	m_netinfo.AddValue(NAME_PARAM, id);
	m_netinfo.AddValue(EPCONFIG_PARAM, static_cast<const void*>(data.data()), data.size());
	if (service && *service) m_netinfo.AddValue(SERVICES_PARAM, service);
	return true;
}


bool VS_AppServerData::GetNetInfo(VS_Container &cfg)
{
	std::lock_guard<std::recursive_mutex> lock(m_lock);
	if (m_netinfo.IsEmpty())
		return false;
	return m_netinfo.CopyTo(cfg);
}


bool VS_AppServerData::AddCid(const char * id)
{
	std::lock_guard<std::recursive_mutex> lock(m_lock);
	return m_cids.Insert(id, 0);
}

bool VS_AppServerData::DelCid(const char * id)
{
	std::lock_guard<std::recursive_mutex> lock(m_lock);
	return m_cids.Erase(id)==1;
}

long VS_AppServerData::GetCids(VS_SimpleStr* &cids)
{
	std::lock_guard<std::recursive_mutex> lock(m_lock);
	long res = 0;
	if (m_cids.Size() > 0) {
		cids = new VS_SimpleStr[m_cids.Size()];
		for (VS_Map::Iterator it = m_cids.Begin(); it!=m_cids.End(); ++it) {
			cids[res++] = (char*)((*it).key);
		}
	}
	return res;
}

bool VS_AppServerData::IsBSList(const char *id)
{
	std::lock_guard<std::recursive_mutex> lock(m_lock);
	for(std::map<VS_SimpleStr,std::list<std::string>>::iterator i = m_bs_by_dns.begin(); i!=m_bs_by_dns.end(); i++)
		for(std::list<std::string>::iterator ii = i->second.begin(); ii!=i->second.end(); ii++)
			if (*ii == id)
				return true;
	return false;
}

bool VS_AppServerData::GetFirstBS(VS_SimpleStr &server)
{
	std::lock_guard<std::recursive_mutex> lock(m_lock);
	for(std::map<VS_SimpleStr,std::list<std::string>>::iterator i = m_bs_by_dns.begin(); i!=m_bs_by_dns.end(); i++)
	{
		for(std::list<std::string>::iterator ii = i->second.begin(); ii!=i->second.end(); ii++)
		{
			if(GetState(ii->c_str()))
			{
				server = ii->c_str();
				return true;
			}
		}
	}
	return false;
}


void VS_AppServerData::SetBSList(VS_Container &cnt)
{
	std::lock_guard<std::recursive_mutex> lock(m_lock);
	m_bs_by_dns.clear();
	cnt.Reset();
	VS_SimpleStr cur_domain;
	while(cnt.Next())
	{
		if(!strcasecmp(cnt.GetName(),"domain"))
			cur_domain = cnt.GetStrValueRef();
		else if(!strcasecmp(cnt.GetName(),"BS")&&!!cur_domain)
			m_bs_by_dns[cur_domain].push_front(cnt.GetStrValueRef());
	}
}
// retrive the best domain BS
bool VS_AppServerData::GetBSByDomain(const VS_SimpleStr& domain, VS_SimpleStr& bs)
{
	if (!domain)
		return false;
	bs.Empty();

	std::lock_guard<std::recursive_mutex> lock(m_lock);
	std::map<VS_SimpleStr,std::list<std::string>>::iterator i = m_bs_by_dns.find(domain);
	if (i==m_bs_by_dns.end())
		return false;

	std::list<std::string>::iterator ii = i->second.begin();
	unsigned long bestDist = -1;
	while (ii!=i->second.end()) {
		if (GetState(ii->c_str())) {
			if (!bs)
				bs = ii->c_str();
			unsigned long Dist = GetDistance(ii->c_str());
			if (Dist < bestDist) {
				bestDist = Dist;
				bs = ii->c_str();
			}
		}
		ii++;
	}
	if (!!bs) {
		dprint4("best BS for %s is %s with dist = %ld\n", domain.m_str, bs.m_str, bestDist);
		return true;
	}
	else
		return false;
}

VS_ServerTypes VS_AppServerData::DetermineType(const char * id)
{
	VS_ServerTypes type = ST_UNKNOWN;
	if (id && *id) {
		const char *st = strchr(id, '#');
		if (st) {
			st++;
			if (strcasecmp(st, "as")==0)
				type = ST_AS;
			else if (strcasecmp(st, "bs")==0)
				type = ST_BS;
			else if (strcasecmp(st, "sm")==0)
				type = ST_SM;
			else if (strcasecmp(st, "rs")==0)
				type = ST_RS;
		}
	}
	return type;
}