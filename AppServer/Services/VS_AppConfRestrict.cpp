#include "VS_AppConfRestrict.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_Utils.h"
#include "VS_AppServerData.h"
#include "AppServer/Services/VS_Storage.h"

// WebRTC PeerConnection
#include "FakeClient/VS_FakeClientManager.h"
#include "std/debuglog/VS_Debug.h"
#include <boost/make_shared.hpp>
#define DEBUG_CURRENT_MODULE VS_DM_OTHER

const char CONFERENCES_KEY[] = "Conferences";
const char LASTCONFERENCENAME_TAG[] = "Last Conference Name";
const int	LASTCONFERENCENAME_INIT = 1;

VS_AppConfRestrict::VS_AppConfRestrict()
{
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	const unsigned long buff_sz(512);
	char buff[buff_sz] = {0};
	if (key.IsValid() && key.GetValue(buff, buff_sz, VS_REG_STRING_VT, "first_missed_call_mail_server_com") && buff && *buff)
		m_bs_com = buff;
	memset(buff, 0, buff_sz);
	if (key.IsValid() && key.GetValue(buff, buff_sz, VS_REG_STRING_VT, "first_missed_call_mail_server_ru") && buff && *buff)
		m_bs_ru = buff;

	// Get Last Conference Name
	uint32_t lastConfName = 0;

	VS_RegistryKey	c_root(false, CONFERENCES_KEY);
	if (c_root.IsValid()) {
		char buff[1024];
		if (c_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, LASTCONFERENCENAME_TAG) > 0) {
			char* pos = strstr(buff, "@");
			if (pos)
				lastConfName = std::stoul(std::string(&buff[0], pos), nullptr, 16);
		}
	}

	m_lastConferenceName = lastConfName ? lastConfName : LASTCONFERENCENAME_INIT;
}


long VS_AppConfRestrict::CheckInsertMultiConf(long tarif_opt, VS_ConferenceDescription &cd, const char *host, bool FromBS)
{
	cd.m_state = cd.PARTICIPANT_ACCEPTED;
	if (FromBS)	// named_conf
		return 0;

	if (cd.m_SubType == GCST_FULL) {
		unsigned int pmax = (tarif_opt&0xff);
		cd.m_MaxParticipants = pmax;
		cd.m_MaxCast = cd.m_MaxParticipants;
	}
	else if (cd.m_SubType == GCST_ALL_TO_OWNER || cd.m_SubType == GCST_PENDING_ALL_TO_OWNER) {
		unsigned int pmax = (tarif_opt&0xff00) >> 8;
		cd.m_MaxParticipants = pmax;
		cd.m_MaxCast = cd.m_MaxParticipants;
	}
	else if (cd.m_SubType == GCST_ROLE) {
		unsigned int pmax = (tarif_opt&0xff0000) >> 16;
		unsigned int pcast = (tarif_opt&0xff000000) >> 24;
		cd.m_MaxCast = pcast;
		cd.m_MaxParticipants = pmax;
	}
	cd.m_LstatusFlag = 1; // allow L-statuses
	return 0;
}



void VS_AppConfRestrict::VCS_SetUserStatus(const VS_SimpleStr& call_id,int status, const VS_ExtendedStatusStorage &extStatus,bool set_server,const VS_SimpleStr& server)
{
}

bool VS_AppConfRestrict::FindUser(const vs_user_id&, VS_StorageUserData&) const
{
	return false;
}


int VS_AppConfRestrict::FindMultiConference(const char* name, VS_ConferenceDescription& conf, VS_Container& cnt, const vs_user_id& from_user, bool FromBS)
{
	if (!name || !FromBS)
		return VSS_CONF_NOT_FOUND;

	// accept confs from BS only
	VS_SimpleStr server = VS_GetConfEndpoint(name);
	if (!server || VS_GetServerType(server.m_str) != ST_BS)
		return VSS_CONF_NOT_FOUND;

	if (g_storage->FindConferenceByCallID(name, conf) == 0)
		return 0;

	int32_t maxParticipants = 1, maxCast = 1, type = CT_PRIVATE, subType = GCST_FULL, scope = GS_PERSONAL;
	cnt.GetValue(MAXPARTISIPANTS_PARAM, maxParticipants);
	cnt.GetValue(MAXCAST_PARAM, maxCast);
	cnt.GetValue(TYPE_PARAM, type);
	cnt.GetValue(SUBTYPE_PARAM, subType);
	cnt.GetValue(SCOPE_PARAM, scope);
	const char* lang = cnt.GetStrValueRef(LANG_PARAM);
	const char* topic = cnt.GetStrValueRef(TOPIC_PARAM);

	// end_time
	size_t size = 0;
	const void * time = cnt.GetBinValueRef(TIME_PARAM, size);
	if (time && size==sizeof(int64_t))
	{
		conf.m_timeExp = tu::WindowsTickToUnixSeconds(*static_cast<const int64_t*>(time));
	};

	conf.m_type = type;
	conf.m_SubType = subType;
	if (topic && *topic)
		conf.m_topic = topic;
	else
		conf.m_topic = name;
	conf.m_public = scope==GS_PUBLIC;
	conf.m_lang = lang;
	conf.m_MaxParticipants = maxParticipants;
	conf.m_MaxCast = maxCast;
	conf.m_owner = cnt.GetStrValueRef(OWNER_PARAM);
	conf.m_call_id = name;
	conf.m_LstatusFlag = 1; // allow L-statuses
	return 0;
}

bool VS_AppConfRestrict::UpdateMultiConference(VS_ConferenceDescription &cd, bool curr)
{
	return false;
}

bool VS_AppConfRestrict::UpdateConference_RTSPAnnounce(VS_ConferenceDescription& cd, string_view announce_id)
{
	return false;
}

unsigned VS_AppConfRestrict::UpdateConfDuration(VS_ConferenceDescription& conf)
{
	return 0;
}

bool VS_AppConfRestrict::GetSSL(VS_SimpleStr& key)
{
	key.Empty();
	return false;
}

VS_SimpleStr VS_AppConfRestrict::GetAnyBSbyDomain(string_view call_id)
{
	VS_SimpleStr bs;
	if (call_id.empty())
		return bs;
	if (g_appServer)
	{
		VS_RealUserLogin r(call_id);
		VS_SimpleStr domain = StringViewToSimpleStr(r.GetDomain());
		if (!!domain)
			g_appServer->GetBSByDomain(domain, bs);
	}
	if (!bs)	// not found in map
	{
		if (call_id.length() >= 3 && call_id.substr(call_id.length() - 3) == ".ru") // check call_id for ".ru"
			bs = m_bs_ru;

		if (!bs)						// try EN BS
		{
			if (!!m_bs_com)
				bs = m_bs_com;
			else
				bs = "bs.video-port.com#bs";
		}
	}
	return bs;
}

bool VS_AppConfRestrict::Tarif_CreateConf(VS_Container& cnt, VS_ConferenceDescription& cd, VS_UserData& ud, VS_TransportRouterServiceReplyHelper* caller)
{
	if(cd.m_type != CT_MULTISTREAM)
		return true;

	bool res = true;
	if (cd.m_SubType == GCST_FULL) {
		if ((ud.m_tarif_restrictions & 0x000000ff) == 0)
			res = false;
	} else if ((cd.m_SubType == GCST_ALL_TO_OWNER) || (cd.m_SubType == GCST_PENDING_ALL_TO_OWNER)) {
		if ((ud.m_tarif_restrictions & 0x0000ff00) == 0)
			res = false;
	} else if (cd.m_SubType == GCST_ROLE) {
		if ((ud.m_tarif_restrictions & 0x00ff0000) == 0)
			res = false;
	}

	return res;
}

bool VS_AppConfRestrict::OnJoinConf(VS_Container& cnt, VS_ConferenceDescription& cd, vs_user_id user_id, bool FromBS, VS_TransportRouterServiceReplyHelper* caller)
{
	return true;
}

void VS_AppConfRestrict::OnRemovePartEvent(const VS_ParticipantDescription& pd, VS_ConferenceDescription& cd, VS_TransportRouterServiceReplyHelper* caller)
{
	VS_UserData ud;
	if (!g_storage->FindUser(SimpleStrToStringView(pd.m_user_id), ud) && caller)		// is roaming, then clean PeerCfg
	{
		VS_Container cnt;
		cnt.AddValue(METHOD_PARAM, UPDATE_PEERCFG_METHOD);
		cnt.AddValue(CALLID_PARAM, pd.m_user_id);
		caller->PostRequest(caller->OurEndpoint(), 0, cnt, 0, AUTH_SRV);
	}
}

bool VS_AppConfRestrict::CheckInviteMulti_Roaming(vs_user_id user_id)
{
	return true;
}

void VS_AppConfRestrict::GetFirstBS(const char* dst_user, const char* our_endpoint, VS_SimpleStr &server)
{
	if (!dst_user || !*dst_user)
		return ;

	VS_UserData ud;
	if (g_storage->FindUser(dst_user,ud))
		server = ud.m_homeServer;

	if (!server && g_appServer)
		g_appServer->GetFirstBS(server);
}

void VS_AppConfRestrict::SetOfflineChatMessage(VS_Container &cnt)
{

}

void VS_AppConfRestrict::DeleteOfflineChatMessage(VS_Container& cnt)
{

}

void VS_AppConfRestrict::GetRoamingOfflineMessages(const char* our_sid, VS_ChatMsgs& v)
{

}
bool VS_AppConfRestrict::IsVCS() const
{
	return false;
}
bool VS_AppConfRestrict::GetAppProp(const char* prop_name, VS_SimpleStr& value)
{
	if (prop_name && *prop_name) {
		if (strcasecmp(prop_name, "lstatus_set")==0) {
			value = "1";
			return true;
		}
	}
	return false;
}

VS_SimpleStr VS_AppConfRestrict::GetLocalMultiConfID(const char* conf_id) const
{
	return conf_id;
}

boost::signals2::connection VS_AppConfRestrict::Connect_AliasesChanged(const AliasesChangedSlot &slot)
{
	boost::signals2::connection tmp;
	return tmp;
}
void VS_AppConfRestrict::UpdateAliasList()
{
}

bool VS_AppConfRestrict::DoWriteConference(const VS_ConferenceDescription &cd)
{
	return false;
}

bool VS_AppConfRestrict::DoBroadcastConference(const VS_ConferenceDescription& cd)
{
	return false;
}

bool VS_AppConfRestrict::LogSlideShow(VS_Container &cnt)
{
	return false;
}

bool VS_AppConfRestrict::LogGroupChat(VS_Container &cnt)
{
	return false;
}

void VS_AppConfRestrict::SetRoamingSettings(const eRoamingMode_t mode, const std::string& params)
{
//	VS_RoamingSettings::SetRoamingSettings(mode, params);
}

vs_conf_id VS_AppConfRestrict::NewConfID(VS_SimpleStr _serverID)
{
	m_lastConferenceName++;
	vs_conf_id newConf;
	newConf.Resize(_serverID.Length() + 10);
	sprintf(newConf.m_str, "%08x@%s", m_lastConferenceName, _serverID.m_str);
	// Update registry
	VS_RegistryKey	root(false, CONFERENCES_KEY, false, true);
	if (!root.IsValid())
	{
		dprint0("Registry error - can't write conf id\n");
	}
	else
	{
		root.SetString(newConf, LASTCONFERENCENAME_TAG);
	}

	return newConf;
}
