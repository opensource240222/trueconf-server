#ifdef _WIN32
#include "../../ServerServices/Common.h"
#include "AppServer/Services/VS_Storage.h"
#include "VS_AppConfigurationService.h"
#include "../../common/net/EndpointRegistry.h"
#include "../../common/std/cpplib/VS_MemoryLeak.h"

#define DEBUG_CURRENT_MODULE VS_DM_CONFIGS

VS_AppConfigurationService::~VS_AppConfigurationService()
{
	for(VS_AppPropertiesMap::iterator i = m_prop_map.begin();i!=m_prop_map.end();i++)
	{
		VS_Container *cnt = i->second;
		delete cnt;
	}
	m_prop_map.clear();
}


bool VS_AppConfigurationService::Init( const char *our_endpoint, const char *our_service, const bool permittedAll )
{
	// add our server to list
	g_appServer->Set(our_endpoint, false, ST_AS);
	return g_appServer->SetNetInfo(our_endpoint, 0);
}


bool VS_AppConfigurationService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)	// Skip
		return true;
	else
		m_recvMess = recvMess.get();

	VS_Container cnt;
	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
			const char* method = 0;
			if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0) {
				dprint3("Processing %20s; cid:%s user:%s srv:%s \n", method, recvMess->SrcCID(), recvMess->SrcUser(), recvMess->SrcServer());
				// Process methods
				if (_stricmp(method, UPDATECONFIGURATION_METHOD) == 0) {
					UpdateConfiguration_Method(cnt);
				}
				else if (_stricmp(method, GETAPPPROPERTIES_METHOD) == 0) {
					GetAppProperties_Method(cnt);
				}
				else if (_stricmp(method, SETPROPERTIES_METHOD) == 0) {
					if (recvMess->Type() == transport::MessageType::Reply)
						SetProperties_Method(cnt);
					else
						SetEpProperties_Method(cnt,std::move(recvMess));
				}
			}
		}
		break;
	case transport::MessageType::Notify:
		break;
	}
	m_recvMess = nullptr;
	return true;
}


void VS_AppConfigurationService::UpdateConfiguration_Method(VS_Container &cnt)
{
	const char * srcServer = m_recvMess->SrcServer();
	if (_stricmp(srcServer, OurEndpoint())!=0) {
		// update from other server
		size_t size = 0;
		const void *cfg = cnt.GetBinValueRef(EPCONFIG_PARAM, size);
		if (cfg)
			g_appServer->SetNetConfig(srcServer, cfg, size);
	}
	else {
		// update from our user
		VS_UserData ud;
		const auto user_id = m_recvMess->SrcUser_sv();
		if (!g_storage->FindUser(user_id, ud))
			return;

		// Remove all IP
		ud.m_IPconfig.Empty();
		int IPCnt = 0;
		void *IP;
		size_t size;
		VS_Container rcnt;

		// Enumerate all configuration
		cnt.Reset();
		char local_ip[1024] = {0};
		while (cnt.Next() && IPCnt < 8) {
			if (_stricmp(cnt.GetName(), IPCONFIG_PARAM) == 0) {
				IPCnt++;
				IP = (void *)cnt.GetBinValueRef(size);
				rcnt.AddValue(IPCONFIG_PARAM, IP, size);
				net::endpoint::ConnectTCP tcp;
				tcp.Deserialize(IP, size);
				sprintf(local_ip + strlen(local_ip), "%s:%d, ", tcp.host.c_str(), tcp.port);
			}
			if (_stricmp(cnt.GetName(), IP6CONFIG_PARAM) == 0) {
				IPCnt++;
				IP = (void *)cnt.GetBinValueRef(size);
				rcnt.AddValue(IP6CONFIG_PARAM, IP, size);
				net::endpoint::ConnectTCP tcp;
				tcp.Deserialize(IP, size);
				sprintf(local_ip + strlen(local_ip), "%s:%d, ", tcp.host.c_str(), tcp.port);
			}
		}
		if (IPCnt && rcnt.SerializeAlloc(IP, size)) {
			ud.m_IPconfig.Set(IP, size);
			free(IP);
		}
		g_storage->UpdateUser(user_id, ud);
		// update ep props
		if (IPCnt) {
			int len = (int)strlen(local_ip);
			if (len > 2)
				local_ip[len-2] = 0;
			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, SETPROPERTIES_METHOD);
			cnt.AddValue(HASH_PARAM, ud.m_appID);
			cnt.AddValue(NAME_PARAM, "Local Ip");
			cnt.AddValue(PROPERTY_PARAM, local_ip);

			PostRequest(ud.m_homeServer, 0, cnt, 0, CONFIGURATION_SRV);
		}
		// update our ep config
		if (g_appServer->GetNetInfo(rcnt)) {
			PostRequest(0, user_id.c_str(), rcnt);
		}
	}
}


void VS_AppConfigurationService::GetAppProperties_Method(VS_Container &cnt)
{
	const char* app_name = cnt.GetStrValueRef(NAME_PARAM);
	const char* ver = cnt.GetStrValueRef(CLIENTVERSION_PARAM);
	auto it = app_name ? m_prop_map.find(app_name) : m_prop_map.end();
	if (it != m_prop_map.end()) {
		auto props = it->second;
		PostReply(*props);
	}
	dprint2("CID %s app %s %s \n", m_recvMess->SrcCID(), app_name ? app_name : "---", ver ? ver : "0.0.0.0");
}


void VS_AppConfigurationService::SetEpProperties_Method(VS_Container &cnt, std::unique_ptr<VS_RouterMessage> &&mess)
{
	VS_UserData ud;
	if (!g_storage->FindUser(m_recvMess->SrcUser_sv(), ud) || ud.m_homeServer.IsEmpty())
		return;
	if (!mess->SetDstServer(ud.m_homeServer.m_str))
		return;
	auto m = mess.release();
	if (!PostMes(m))
		delete m;
}


void VS_AppConfigurationService::SetProperties_Method(VS_Container &cnt)
{
	VS_Container	*pCnt(0);
	VS_SimpleStr name = cnt.GetStrValueRef("app_name");
	auto it = m_prop_map.find(name);
	if (it != m_prop_map.end())
	{
		delete it->second;
		m_prop_map.erase(it);
	}
	pCnt = new VS_Container;
	cnt.CopyTo(*pCnt);
	if (!pCnt->GetStrValueRef("server_protocol_version"))
		pCnt->AddValue("server_protocol_version", SERVER_PROTOCOL_VERSION_STR);
	m_prop_map[name] = pCnt;
}
#endif