#include "VS_VCSConfRestrict.h"

#include "../../ServerServices/VS_ReadLicense.h"
#include "ServerServices/VS_RoamingSettings.h"
#include "../../ServerServices/VS_TorrentService.h"
#include "../../ServerServices/VS_TorrentStarter.h"
#include "AppServer/Services/VS_Storage.h"
#include "../../BaseServer/Services/storage/VS_DBStorageInterface.h"
#include "FakeClient/VS_FakeClientManager.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "../../common/std/cpplib/VS_CallIDUtils.h"
#include "../../common/std/cpplib/VS_Protocol.h"
#include "../../common/std/VS_RegServer.h"
#include "../../common/std/cpplib/VS_Utils.h"
#include "../../common/std/cpplib/VS_Replace.h"
#include "ProtectionLib/Protection.h"
#include "statuslib/VS_ResolveServerFinder.h"
#include "std/cpplib/layout_json.h"

extern std::string g_tr_endpoint_name;

#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace rj = rapidjson;

#include <boost/make_shared.hpp>
#include <boost/regex.hpp>

//Multi conferences
const char MMC_PASS_TAG[]         = "Password";
const char MMC_OWNER_TAG[]        = "Owner";
const char MMC_TYPE_TAG[]         = "Type";
const char MMC_SUBTYPE_TAG[]      = "SubType";
const char MMC_TOPIC_TAG[]        = "Topic";
const char MMC_CONFID_TAG[]		  = "ID";
const char MMC_CASTER_TAG[]       = "Caster";
const char MMC_PODIUMS_TAG[]		= "Podiums";
const char MMC_MAXPARTS_TAG[]		= "Max Participants";
const char MMC_INVITEDTIME[]		= "Invited Time";
const char MMC_INVITATION_DATE[]	= "Invitation Date";
const char MMC_INVITATION_DAY[]		= "Invitation Day";
const char MMC_INVITATION_TIME[]	= "Invitation Time";
const char MMC_INVITATION_TYPE[]	= "Invitation Type";
const char MMC_AUTO_INVITE[]		= "auto_invite";
const char MMC_MULTICAST_IP_TAG[]	= "intercom_ip";
const char MMC_BROADCAST_TAG[]	  = "BroadcastEnabled";
const char MMC_CONFERENCE_RECORDING_TAG[] = "ConferenceRecording";
const char MMC_CODECS_TAG[]       = "Enabled codecs";
const char MMC_RTSP_ANNOUNCE_KEY[] = "RTSP Announce";
const char MMC_RTSP_ANNOUNCE_URL_TAG[] = "URL";
const char MMC_RTSP_ANNOUNCE_USERNAME_TAG[] = "Username";
const char MMC_RTSP_ANNOUNCE_PASSWORD_TAG[] = "Password";
const char MMC_RTSP_ANNOUNCE_RTP_OVER_TCP_TAG[] = "RTP over TCP";
const char MMC_RTSP_ANNOUNCE_KEEPALIVE_TIMEOUT_TAG[] = "Inactive Receiver Timeout";
const char MMC_RTSP_ANNOUNCE_RETRIES_TAG[] = "Retries";
const char MMC_RTSP_ANNOUNCE_RETRY_DELAY_TAG[] = "Retry delay";
const char MMC_RTSP_ANNOUNCE_CODECS_TAG[] = "Enabled codecs";
const char MMC_RTSP_ANNOUNCE_ACTIVE_TAG[] = "Active";
const char MMC_RTSP_ANNOUNCE_REASON_TAG[] = "Reason";
const char MMC_RTSP_HELPER_PROGRAM_TAG[] = "RTSP Helper Program";
const char MMC_WEBPARAMS_KEY[] = "Web";
const char MMC_WEBPARAMS_TYPE[] = "Type";
const char MMC_CLIENTRIGHTS_KEY[] = "ClientRights";
const char MMC_CLIENTRIGHTS_GUEST[] = "guest";
const char MMC_CLIENTRIGHTS_USER[] = "user";
const char MMC_DURATION_TAG[] = "Duration"; // duration of conference in seconds
const char MMC_PLANNEDPARTICIPANTSONLY_TAG[] = "PlannedParticipantsOnly";
const char MMC_ORIGINTYPE_TAG[] = "OriginType";

const char CLIENTCONFORIGIN_NAME[] = "clientapp";

namespace
{
	void ParseSlideShowCmdParam(const std::string &params, const char *paramName, std::string &value);
}

VS_VCSConfRestrict::VS_VCSConfRestrict()
{
	VS_RegistryKey	conf_key(false, MULTI_CONFERENCES_KEY, false);
	VS_RegistryKey	next_conf;
	conf_key.ResetKey();
	std::vector<std::string> spec_confs;
	while(conf_key.NextKey(next_conf, false))
	{
		next_conf.RemoveValue(MMC_CONFID_TAG);
		next_conf.RemoveKey("ManageConference");

		std::string origin;
		bool hot = next_conf.GetString(origin, MMC_ORIGINTYPE_TAG) && origin == CLIENTCONFORIGIN_NAME;

		const char* str = next_conf.GetName();
		if (hot || IsSpecialConf(str))
			spec_confs.emplace_back(str);
	}
	for (const auto& x: spec_confs)
		conf_key.RemoveKey(x);
}
#include "ProtectionLib/OptimizeDisable.h"
long VS_VCSConfRestrict::CheckInsertMultiConf(long tarif_opt, VS_ConferenceDescription &cd, const char *host, bool CheckTarifRestrictions)
{
	long result = VSS_CONF_LIC_LIMITED;
	bool limited = !!g_storage->CountGroupConferences();
SECUREBEGIN_A_FULL
	limited = !VS_CheckLicense(LE_NEWCONFERENCE);
SECUREEND_A_FULL

	if (!limited) {
		limited|= CT_INTERCOM == cd.m_type && !VS_CheckLicense(LE_UDPMULTICAST_ALLOWED);
		limited|= GCST_ROLE==cd.m_SubType && !VS_CheckLicense(LE_ROLE_CONF_ALLOWED);
		limited|= (GCST_ALL_TO_OWNER==cd.m_SubType || GCST_PENDING_ALL_TO_OWNER==cd.m_SubType) && !VS_CheckLicense(LE_ASYMMETRICCONF_ALLOWED);
	}
	if (!limited) {
		int32_t tarif_opts[4] = { 0 };
		auto max_parts_wish = cd.m_MaxParticipants;
		VS_GetLicence_TarifRestrictions(tarif_opts, cd.m_MaxCast, false, cd.m_type == CT_INTERCOM, cd.m_PrivacyType == cd.e_PT_Public);
		if (cd.m_SubType == GCST_FULL) {
			unsigned int pmax = tarif_opts[0];
			cd.m_MaxParticipants = pmax;
			cd.m_MaxCast = cd.m_MaxParticipants;
		}
		else if (cd.m_SubType == GCST_ALL_TO_OWNER || cd.m_SubType == GCST_PENDING_ALL_TO_OWNER) {
			unsigned int pmax = tarif_opts[1];
			cd.m_MaxParticipants = pmax;
			cd.m_MaxCast = cd.m_MaxParticipants;
		}
		else if (cd.m_SubType == GCST_ROLE) {
			unsigned int pcast = tarif_opts[3];
			unsigned int pmax = tarif_opts[2];
			cd.m_MaxParticipants = pmax;
			cd.m_MaxCast = pcast;
		}
		if (!!cd.m_call_id)		// NamedConf at Registry
			cd.m_MaxParticipants = std::min(cd.m_MaxParticipants, max_parts_wish);
		// check duration
		UpdateConfDuration(cd);
		// check ssl streams
		GetSSL(cd.m_symKey);
		// set state
		cd.m_state = cd.PARTICIPANT_ACCEPTED;

		VS_SimpleStr value;
		cd.m_LstatusFlag = GetAppProp("lstatus_set", value) && !!value;

		result = 0;
	}

	return result;
}
#include "ProtectionLib/OptimizeEnable.h"

void VS_VCSConfRestrict::VCS_SetUserStatus(const VS_SimpleStr& call_id,int status, const VS_ExtendedStatusStorage &extStatus, bool set_server,const VS_SimpleStr& server)
{
	if (!call_id || VS_IsNotTrueConfCallID(call_id.m_str))
		return;

	VS_UserData ud;
	if (g_storage->FindUser(SimpleStrToStringView(call_id), ud) && ud.m_client_type==CT_TRANSCODER)
		return;

	// ktrushnikov: call only for VCS (to update registry)
	auto dbStorage = g_dbStorage;
	if (dbStorage)
		dbStorage->SetUserStatus(call_id, status, extStatus, set_server, server);
}

bool VS_VCSConfRestrict::FindUser(const vs_user_id& id, VS_StorageUserData& user) const
{
	auto dbStorage = g_dbStorage;
	if (!dbStorage)
		return false;

	return dbStorage->FindUser(id, user);
}


int  VS_VCSConfRestrict::FindMultiConference(const char* name, VS_ConferenceDescription& conf, VS_Container& cnt, const vs_user_id& from_user, bool FromBS)
{
	if (!name || !*name)
		return VSS_CONF_NOT_FOUND;

	// skip @server.name
	ConferenceInvitation ci;
	VS_SimpleStr tmp;
	long scope(0);

	const char* conf_ep = VS_GetConfEndpoint(name);
	if (conf_ep && strcasecmp(g_tr_endpoint_name.c_str(), conf_ep)) //confernece is at other server.
		return VSS_CONF_NOT_FOUND;


	auto ConfID = VS_RealUserLogin(name).GetUser();

	int res = g_dbStorage->GetNamedConfInfo(name, conf, ci, tmp, scope);
	if (res != VSS_CONF_NOT_FOUND)
		return res;

	if (!IsSpecialConf(ConfID.c_str()))
		return VSS_CONF_NOT_FOUND;
	cnt.GetValueI32(SCOPE_PARAM, scope);
	if (InsertSpecialConference(ConfID.c_str(), from_user, cnt.GetStrValueRef(TOPIC_PARAM), scope) != 0)
		return VSS_CONF_NOT_FOUND;

	std::string key_name;
	key_name.reserve(128);
	key_name += MULTI_CONFERENCES_KEY;
	key_name += '\\';
	key_name += ConfID;
	VS_RegistryKey conf_key(false, key_name, false);

	char buf[256] = {0};
	conf.m_name.Empty();
	conf.m_call_id = ConfID.c_str();

	conf_key.GetValue(buf, sizeof(buf), VS_REG_STRING_VT, MMC_OWNER_TAG);
	VS_RealUserLogin r(buf);
	conf.m_owner = r; conf.m_owner.ToLower();

	conf.m_type = CT_MULTISTREAM;
	conf_key.GetValue(&conf.m_type, sizeof(int), VS_REG_INTEGER_VT, MMC_TYPE_TAG);

	conf_key.GetValue(&conf.m_SubType, sizeof(int), VS_REG_INTEGER_VT, MMC_SUBTYPE_TAG);
	if (conf.m_SubType<GCST_FULL || conf.m_SubType>GCST_ROLE)
		conf.m_SubType = GCST_FULL; // stupid check

	conf_key.GetString(conf.m_topic, MMC_TOPIC_TAG);

	*buf = 0;
	conf_key.GetValue(buf, sizeof(buf), VS_REG_STRING_VT, MMC_CONFID_TAG);
	conf.m_name = buf;

	*buf = 0;
	conf_key.GetValue(buf, sizeof(buf), VS_REG_STRING_VT, MMC_MULTICAST_IP_TAG);
	conf.m_multicast_ip = buf;

	conf.m_MaxCast = 4;
	if (conf.m_SubType == GCST_ROLE)
		conf_key.GetValue(&conf.m_MaxCast, sizeof(int), VS_REG_INTEGER_VT, MMC_PODIUMS_TAG);

	conf.m_MaxParticipants = -1;
	conf_key.GetValue(&conf.m_MaxParticipants, sizeof(int), VS_REG_INTEGER_VT, MMC_MAXPARTS_TAG);

	unsigned long duration = 0xd2d2d; // 10 days
	conf_key.GetValue(&duration, sizeof(unsigned long), VS_REG_INTEGER_VT, MMC_DURATION_TAG);
	conf.SetTimeExp(duration);

	return 0;
}


bool VS_VCSConfRestrict::UpdateMultiConference(VS_ConferenceDescription &cd, bool curr)
{
	std::string key_name;
	key_name.reserve(128);
	key_name += MULTI_CONFERENCES_KEY;
	key_name += '\\';
	key_name += cd.m_call_id.m_str;

	if (curr) {
		if (IsSpecialConf(cd.m_name)) {
			InsertSpecialConference(cd.m_name, cd.m_owner, cd.m_topic.c_str(), cd.m_public);
		}
		else {
			VS_RegistryKey conf_key_test(false, key_name);
			if (!conf_key_test.IsValid()) {
				// suppose it is hot conference from client
				// TODO: use cd to determine OriginType
				VS_RegistryKey conf_key(false, key_name, false, true);

				if (!conf_key.IsValid())
					return false;

				conf_key.SetString(CLIENTCONFORIGIN_NAME, MMC_ORIGINTYPE_TAG);
				conf_key.SetValue(&cd.m_type, sizeof(cd.m_type), VS_REG_INTEGER_VT, MMC_TYPE_TAG);
				conf_key.SetValue(&cd.m_SubType, sizeof(cd.m_SubType), VS_REG_INTEGER_VT, MMC_SUBTYPE_TAG);
				conf_key.SetString(cd.m_owner.m_str, MMC_OWNER_TAG);
				conf_key.SetString(cd.m_topic.c_str(), MMC_TOPIC_TAG);
				conf_key.SetValue(&cd.m_MaxParticipants, sizeof(cd.m_MaxCast), VS_REG_INTEGER_VT, MMC_MAXPARTS_TAG);
				if (cd.m_SubType == GCST_ROLE)
					conf_key.SetValue(&cd.m_MaxCast, sizeof(cd.m_MaxCast), VS_REG_INTEGER_VT, MMC_PODIUMS_TAG);

				std::chrono::system_clock cl;
				int32_t secs_since_epoch = (int32_t)cl.to_time_t(cl.now());
				conf_key.SetValue(&secs_since_epoch, sizeof(secs_since_epoch), VS_REG_INTEGER_VT, "created");

				if (cd.m_public) {
					std::string web_key_name;
					web_key_name = key_name;
					web_key_name += "\\Web";
					VS_RegistryKey web_key(false, web_key_name, false, true);
					if (!web_key.IsValid()) {
						return false;
					}
					long type = 1;
					web_key.SetValue(&type, sizeof(type), VS_REG_INTEGER_VT, MMC_WEBPARAMS_TYPE);
				}
			}
		}
	}

	VS_RegistryKey conf_key(false, key_name, false, false);

	if(!conf_key.IsValid())
		return false;

	if (curr) {
		conf_key.SetString(cd.m_name.m_str, MMC_CONFID_TAG);
		{
			key_name += "\\";
			key_name += "Participants";
			VS_RegistryKey participants_key(false, key_name);
			VS_RegistryKey part_key;
			vs::set<vs_user_id> parts;
			participants_key.ResetKey();
			while (participants_key.NextKey(part_key))
				parts.emplace(VS_RealUserLogin(part_key.GetName()).GetID());
			g_storage->SetPlannedParticipants(cd.m_name, parts);
		}

		// update conf start time (for InvitesStorage)
		{
			char t[256] = {0};
			tu::TimeToRuStr(std::chrono::system_clock::now(), t, 256);
			conf_key.SetString(t, MMC_INVITEDTIME);
		}
	} else {
		std::string origin;
		bool hot = conf_key.GetString(origin, MMC_ORIGINTYPE_TAG) && origin == CLIENTCONFORIGIN_NAME;
		bool special = !!cd.m_call_id && IsSpecialConf(cd.m_call_id);
		if (hot || special) {
			VS_RegistryKey del(false, MULTI_CONFERENCES_KEY, false, false);
			del.RemoveKey(cd.m_call_id.m_str);
		}
		else {
			conf_key.RemoveValue(MMC_CONFID_TAG);
			conf_key.RemoveKey("ManageConference");
		}
	}

	return true;
}

bool VS_VCSConfRestrict::UpdateConference_RTSPAnnounce(VS_ConferenceDescription& cd, string_view announce_id)
{
	auto dbStorage = g_dbStorage;
	if (dbStorage)
		return 0 == dbStorage->UpdateNamedConfInfo_RTSPAnnounce(cd, announce_id);
	else
		return false;
}


unsigned VS_VCSConfRestrict::UpdateConfDuration(VS_ConferenceDescription& conf)
{
	long sec = p_licWrap->GetLicSum().m_trial_conf_minutes*59+p_licWrap->GetLicSum().m_trial_conf_minutes;
	if (sec)
		conf.SetTimeExp(sec);
	return 0;
}

bool VS_VCSConfRestrict::IsPlannedParticipant(VS_ConferenceDescription &cd, const char *user)
{
	std::string normuser(user);
	VS_RemoveTranscoderID(normuser);
	VS_RealUserLogin rl(normuser);

	if (!!cd.m_name && g_storage->IsPlannedParticipant(cd.m_name, rl.GetID()))
		return true;

	bool found = false;
	std::string key_name(MULTI_CONFERENCES_KEY);
	key_name = key_name + '\\' + VS_RealUserLogin(SimpleStrToStringView(cd.m_call_id)).GetUser() + '\\' + "Participants" + '\\';
	if (rl.IsOurSID()) {
		std::string key_name1 = key_name + rl.GetUser();
		VS_RegistryKey part_key(false, key_name1);
		found = part_key.IsValid();
	}
	if (!found) { // try full id
		key_name += rl.GetID();
		VS_RegistryKey part_key(false, key_name);
		found = part_key.IsValid();
	}
	return found;
}


bool VS_VCSConfRestrict::GetSSL(VS_SimpleStr& key)
{
	key.Empty();
	if (VS_CheckLicense(LE_SSL_STREAMS)) {
		key.Resize(100);
		VS_GenKeyByMD5(key);
	}
	return !!key;
}

VS_SimpleStr VS_VCSConfRestrict::GetAnyBSbyDomain(string_view call_id)
{
	if (call_id.empty())
		return StringViewToSimpleStr(VS_RemoveServerType(g_tr_endpoint_name));

	auto resolver = VS_ResolveServerFinder::Instance();
	assert(resolver);
	std::string uplink_server;
	return (resolver->GetServerForResolve(StringViewToSimpleStr(call_id), uplink_server, false) &&
		!uplink_server.empty() &&
		resolver->IsServerOnline(uplink_server)) ?
		uplink_server.c_str() : nullptr;
}

int VS_VCSConfRestrict::InsertSpecialConference(const char* name, const vs_user_id& from_user, const char* topic, bool is_public)
{
	if (!name || !*name || strlen(name) < 4)
		return VSS_CONF_NOT_VALID;

	char tmp[2] = {name[2], 0};
	int type = atoi(tmp);
	tmp[0] = name[3];
	int SubType = atoi(tmp);

	// check supported types and subtypes (bug #7086)
	if ((type!=5 && type!=6) || (SubType<0 || SubType>3))
		return VSS_CONF_NOT_VALID;

	std::string key_name;
	key_name.reserve(128);
	key_name += MULTI_CONFERENCES_KEY;
	key_name += '\\';
	key_name += name;
	VS_RegistryKey conf_key(false, key_name, false, true);

	if(!conf_key.IsValid())
		return VSS_CONF_NOT_VALID;

	bool result=true;
	result&=conf_key.SetValue(&type, sizeof(type), VS_REG_INTEGER_VT, MMC_TYPE_TAG);
	result&=conf_key.SetValue(&SubType, sizeof(SubType), VS_REG_INTEGER_VT, MMC_SUBTYPE_TAG);
	if (from_user.Length())
		result&=conf_key.SetString(from_user.m_str, MMC_OWNER_TAG);
	if (topic && *topic)
		result &= conf_key.SetString(topic, MMC_TOPIC_TAG);

	if (is_public) {
		std::string web_key_name;
		web_key_name = key_name;
		web_key_name += "\\Web";
		VS_RegistryKey web_key(false, web_key_name, false, true);
		if (!web_key.IsValid()) {
			return VSS_CONF_NOT_VALID;
		}
		long type = 1;
		result &= web_key.SetValue(&type, sizeof(type), VS_REG_INTEGER_VT, MMC_WEBPARAMS_TYPE);
	}

	if(!result)
		return VSS_CONF_NOT_VALID;

	return 0;
}

bool VS_VCSConfRestrict::Tarif_CreateConf(VS_Container& cnt, VS_ConferenceDescription& cd, VS_UserData& ud, VS_TransportRouterServiceReplyHelper* caller)
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

bool VS_VCSConfRestrict::OnJoinConf(VS_Container& cnt, VS_ConferenceDescription& cd, vs_user_id user_id, bool FromBS, VS_TransportRouterServiceReplyHelper* caller)
{
	VS_RealUserLogin r(SimpleStrToStringView(user_id));
	if (!r.IsOurSID())									// roaming participant?
	{
		VS_UserData ud;
		if (g_storage->FindUser(SimpleStrToStringView(user_id), ud) && ud.m_client_type == CT_TRANSCODER)
			return true;
		if(!VS_CheckLicense(LE_ROAMING_ON))
			return false;
		if (cd.m_type>CT_PRIVATE_DIRECTLY)				// multi conf?
			g_storage->AddRoamingParticipant();
	}
	return true;
}

void VS_VCSConfRestrict::OnRemovePartEvent(const VS_ParticipantDescription& pd, VS_ConferenceDescription& cd, VS_TransportRouterServiceReplyHelper* caller)
{
	VS_RealUserLogin r(SimpleStrToStringView(pd.m_user_id));
	if (!r.IsOurSID())									// roaming participant?
	{
		if (cd.m_type>CT_PRIVATE_DIRECTLY)				// multi conf?
			g_storage->RemoveRoamingParticipant();
	}
}

bool VS_VCSConfRestrict::CheckInviteMulti_Roaming(vs_user_id user_id)
{
	VS_RealUserLogin r(SimpleStrToStringView(user_id));
	if (!r.IsOurSID())									// roaming participant?
	{
		if (!VS_CheckLicense_CheckRoamingParticipant())	// roaming users < online users in lic?
			return false;
	}
	return true;
}

void VS_VCSConfRestrict::GetFirstBS(const char* dst_user, const char* our_endpoint, VS_SimpleStr &server)
{
	if (!dst_user || !*dst_user || !our_endpoint || !*our_endpoint)
		return ;

	server = our_endpoint;
}

void VS_VCSConfRestrict::SetOfflineChatMessage(VS_Container &cnt)
{
	VS_SimpleStr from	= cnt.GetStrValueRef(FROM_PARAM);
	std::string from_dn; if(auto pFrom_dn	= cnt.GetStrValueRef(DISPLAYNAME_PARAM) != nullptr) from_dn = pFrom_dn;
	VS_SimpleStr to		= cnt.GetStrValueRef(TO_PARAM);
	VS_SimpleStr body_utf8 = cnt.GetStrValueRef(MESSAGE_PARAM);

	auto dbStorage = g_dbStorage;
	if (dbStorage)
		dbStorage->SetOfflineChatMessage(from, to, body_utf8, from_dn, cnt);
}

void VS_VCSConfRestrict::DeleteOfflineChatMessage(VS_Container& cnt)
{
	auto dbStorage = g_dbStorage;
	if (dbStorage)
		dbStorage->DeleteOfflineChatMessage(cnt);
}

void VS_VCSConfRestrict::GetRoamingOfflineMessages(const char* our_sid, VS_ChatMsgs& v)
{
	auto dbStorage = g_dbStorage;
	if (dbStorage)
		dbStorage->GetRoamingOfflineMessages(our_sid, v);
}

bool VS_VCSConfRestrict::IsVCS() const
{
	return true;
}

VS_UserPresence_Status VS_VCSConfRestrict::CheckOfflineStatus( const VS_SimpleStr& call_id, VS_ExtendedStatusStorage & extStatus )
{
	auto dbStorage = g_dbStorage;
	if (dbStorage)
		dbStorage->GetExtendedStatus(call_id, extStatus);
	return USER_LOGOFF;
}
bool VS_VCSConfRestrict::IsRoamingAllowed(const char *for_server_name)
{
	if (!!for_server_name && strcasecmp(RegServerName, for_server_name) == 0)
		return true;
	if (m_roaming_settings && m_roaming_settings->RoamingMode() == RM_DISABLED)
		return false;
	if (!VS_CheckLicense(LE_ROAMING_ON))
		return false;
	if (!for_server_name || !*for_server_name)
		return true;
	return m_roaming_settings && m_roaming_settings->IsRoamingAllowed(for_server_name);
}
void VS_VCSConfRestrict::SetRoamingSettings(const eRoamingMode_t mode, const std::string& params)
{
	if (m_roaming_settings)
		m_roaming_settings->SetRoamingSettings(mode, params);
}

bool VS_VCSConfRestrict::GetAppProp(const char* prop_name, VS_SimpleStr& value)
{
	bool res(false);
	auto dbStorage = g_dbStorage;
	if (dbStorage)
		res = dbStorage->GetAppProperty(0, prop_name, value);
	return res;
}

VS_SimpleStr VS_VCSConfRestrict::GetLocalMultiConfID(const char* conf_id) const
{
	if (!conf_id || !*conf_id)
		return 0;
	return VS_RealUserLogin(conf_id).GetUser().c_str();
}

bool VS_VCSConfRestrict::CheckSessionID(const char *pass) const
{
	auto dbStorage = g_dbStorage;
	if (dbStorage)
		return dbStorage->CheckSessionID(pass);
	else
		return false;
}

boost::signals2::connection VS_VCSConfRestrict::Connect_AliasesChanged(const AliasesChangedSlot &slot)
{
	boost::signals2::connection tmp;
	if (!g_dbStorage)
		return tmp;
	return g_dbStorage->Connect_AliasesChanged(slot);
}
void VS_VCSConfRestrict::UpdateAliasList()
{
	if (g_dbStorage)
		g_dbStorage->UpdateAliasList();
}

bool VS_VCSConfRestrict::DoWriteConference(const VS_ConferenceDescription &cd)
{
	unsigned ConferenceRecording(0);
	bool res = false;
	VS_RegistryKey cfg(false, CONFIGURATION_KEY);
	if (cd.m_type == CT_MULTISTREAM) {
		cfg.GetValue(&ConferenceRecording, 4, VS_REG_INTEGER_VT, "ConferenceRecording");
		if (ConferenceRecording == 1 || ConferenceRecording == 2) // multi and all in 4.3.9
			res = true;
		else if (ConferenceRecording == 3) // custom
			res = cd.m_need_record;
	}
	else if (cd.m_type == CT_PRIVATE) {
		cfg.GetValue(&ConferenceRecording, 4, VS_REG_INTEGER_VT, "CallRecording");
		res = ConferenceRecording != 0;
	}
	return res;
}


bool VS_VCSConfRestrict::DoBroadcastConference(const VS_ConferenceDescription& cd)
{
	return cd.m_isBroadcastEnabled && VS_CheckLicense(LE_CONF_BROADCAST);
}

bool VS_VCSConfRestrict::GetRecordState(const VS_ConferenceDescription& cd, VS_ConfRecordingState &state)
{
	if (!cd.m_call_id)
		return false;

	std::string key_name;
	key_name.reserve(128);
	key_name += MULTI_CONFERENCES_KEY;
	key_name += '\\';
	key_name += cd.m_call_id.m_str;

	VS_RegistryKey conf_key(false, key_name);
	if (!conf_key.IsValid())
		return false;

	key_name += "\\ManageConference";

	state = RS_NO_RECORDING;
	VS_RegistryKey state_key(false, key_name);
	if (!state_key.IsValid())
		return true;
	return state_key.GetValue(&state, sizeof(state), VS_REG_INTEGER_VT, "RecordingState") >= 0;
}

bool VS_VCSConfRestrict::SetRecordState(const VS_ConferenceDescription& cd, VS_ConfRecordingState state)
{
	if (!cd.m_call_id)
		return false;

	std::string key_name;
	key_name.reserve(128);
	key_name += MULTI_CONFERENCES_KEY;
	key_name += '\\';
	key_name += cd.m_call_id.m_str;

	VS_RegistryKey conf_key(false, key_name);
	if (!conf_key.IsValid())
		return false;

	key_name += "\\ManageConference";

	VS_RegistryKey state_key(false, key_name, false, true);
	return state_key.SetValue(&state, sizeof(state), VS_REG_INTEGER_VT, "RecordingState");
}

// update id in json with RealUserLogin: add @server.name
void UpdateIDinJSON(std::string& json)
{
	rj::Document doc;
	if (doc.Parse(json.c_str()).GetParseError() != rj::kParseErrorNone || !doc.IsObject())
		return;
	auto slots = doc.FindMember(layout_json::slots);
	if (slots == doc.MemberEnd() || !slots->value.IsArray())
		return;
	for (auto&& arr : slots->value.GetArray())
	{
		if (!arr.IsObject())
			continue;
		auto slot = arr.GetObject();
		string_view id;
		auto x = slot.FindMember(layout_json::id);
		if (x != slot.MemberEnd() && !x->value.IsNull())
			id = string_view(x->value.GetString(), x->value.GetStringLength());
		if (id.empty() || VS_IsNotTrueConfCallID(id))
			continue;
		x->value.SetString(VS_RealUserLogin(id).GetID(), doc.GetAllocator());
	}
	rj::StringBuffer buffer;
	rj::Writer<rj::StringBuffer> writer(buffer);
	doc.Accept(writer);
	json = buffer.GetString();
}

// copy reg keys from \\VideoLayouts to \\ManageConference\\VideoLayouts, updating id to id@server.name
void ConvertVideoLayoutsToRTM(const std::string& conf_key_name)
{
	if (VS_RegistryKey(false, conf_key_name + "\\ManageConference\\VideoLayouts").IsValid())
		return;
	VS_RegistryKey conf_key(false, conf_key_name + "\\VideoLayouts");
	if (!conf_key.IsValid())
		return;

	auto update_key = [](const VS_RegistryKey& src_key, VS_RegistryKey&& dst_key)
	{
		for (const char* name : { "CommonLayoutJSON", "MixerLayoutJSON", "LayoutJSON" })
		{
			std::string json;
			if (!src_key.GetString(json, name) || json.empty())
				continue;
			UpdateIDinJSON(json);
			dst_key.SetValue(json.c_str(), 0, VS_REG_STRING_VT, name);
		}
	};
	update_key(conf_key, VS_RegistryKey(false, conf_key_name + "\\ManageConference\\VideoLayouts", false, true));

	VS_RegistryKey src_key;
	conf_key.ResetKey();
	while (conf_key.NextKey(src_key))
		update_key(src_key, VS_RegistryKey(false, conf_key_name + "\\ManageConference\\VideoLayouts" + "\\" + VS_RealUserLogin(src_key.GetName()).GetID(), false, true));
}

std::string VS_VCSConfRestrict::ReadLayout(string_view cid, layout_for f, string_view part)
{
	std::string layout;

	std::string key_name;
	key_name.reserve(128);
	key_name += MULTI_CONFERENCES_KEY;
	key_name += '\\';
	key_name += cid;

	std::string key_name_curr = key_name + "\\ManageConference\\VideoLayouts";

	ConvertVideoLayoutsToRTM(key_name);

	VS_RegistryKey conf_key_curr(false, key_name_curr);
	if (!conf_key_curr.IsValid())
		return {};

	conf_key_curr.GetString(layout, "CommonLayoutJSON");
	if (f == layout_for::all) {
	}
	else if (f == layout_for::mixer) {
		conf_key_curr.GetString(layout, "MixerLayoutJSON");
	}
	else if (f == layout_for::individual) {
		if (!part.empty()) {
			key_name_curr += '\\';
			key_name_curr += part;
			VS_RegistryKey part_key_curr(false, key_name_curr);
			part_key_curr.GetString(layout, "LayoutJSON");
		}
	}
	return layout;
}

bool VS_VCSConfRestrict::UpdateLayout(string_view cid, string_view new_layout, layout_for f, string_view part)
{
	std::string key_name;
	key_name.reserve(128);
	key_name += MULTI_CONFERENCES_KEY;
	key_name += '\\';
	key_name += cid;
	key_name += "\\ManageConference\\VideoLayouts";

	VS_RegistryKey conf_key(false, key_name, false, true);
	if (f == layout_for::all) {
		if (new_layout.empty())
			return conf_key.RemoveValue("CommonLayoutJSON");
		else
			return conf_key.SetString(std::string(new_layout).c_str(), "CommonLayoutJSON");
	}
	else if (f == layout_for::mixer) {
		if (new_layout.empty())
			return conf_key.RemoveValue("MixerLayoutJSON");
		else
			return conf_key.SetString(std::string(new_layout).c_str(), "MixerLayoutJSON");
	}
	else if (f == layout_for::individual) {
		if (!part.empty()) {
			if (new_layout.empty()) {
				conf_key.RemoveKey(part);
				return true;
			}
			else {
				key_name += '\\';
				key_name += part;
				VS_RegistryKey part_key(false, key_name, false, true);
				return part_key.SetString(std::string(new_layout).c_str(), "LayoutJSON");
			}
		}
	}
	return false;
}

std::vector<std::string> VS_VCSConfRestrict::GetUsersWithIndividualLayouts(string_view cid)
{
	std::string key_name;
	key_name.reserve(128);
	key_name += MULTI_CONFERENCES_KEY;
	key_name += '\\';
	key_name += cid;
	key_name += "\\ManageConference\\VideoLayouts";

	VS_RegistryKey conf_key(false, key_name);
	if (!conf_key.IsValid())
		return {};
	std::vector<std::string> v;
	VS_RegistryKey	next_user;
	conf_key.ResetKey();
	while (conf_key.NextKey(next_user, false))
	{
		v.emplace_back(next_user.GetName());
	}
	return v;
}

bool VS_VCSConfRestrict::LogSlideShow(VS_Container &cnt)
{
	const char* conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (!conf||!*conf)
		return false;
	const char *from = cnt.GetStrValueRef(FROM_PARAM);
	if (!from||!*from)
		return false;
	const char *msg = cnt.GetStrValueRef(MESSAGE_PARAM);
	if (!msg||!*msg)
		return false;

	bool res = false;
	auto dbStorage = g_dbStorage;
	if (dbStorage)
	{
		if (!strcasecmp(msg,END_SLIDESHOW_COMMAND))
			dbStorage->LogSlideShowEnd(conf, from);
		else
		{
			std::string params(msg);
			std::string url, mimeType, about, slideIndex, slidesCount, width, height, size;

			ParseSlideShowCmdParam(params, "URL_PARAM", url);
			ParseSlideShowCmdParam(params, "TYPE_PARAM", mimeType);
			ParseSlideShowCmdParam(params, "ABOUT_PARAM", about);
			ParseSlideShowCmdParam(params, "SLIDE_N", slideIndex);
			ParseSlideShowCmdParam(params, "SLIDE_COUNT", slidesCount);
			ParseSlideShowCmdParam(params, "WIDTH_PARAM", width);
			ParseSlideShowCmdParam(params, "HEIGHT_PARAM", height);
			ParseSlideShowCmdParam(params, "SIZE_PARAM", size);

			res = dbStorage->LogSlideShowCmd(conf, from, url.c_str(), mimeType.c_str(), atoi(slideIndex.c_str()), atoi(slidesCount.c_str()), about.c_str(),
											 atoi(width.c_str()), atoi(height.c_str()), atoi(size.c_str()));
		}
	}

	return res;
}

bool VS_VCSConfRestrict::LogGroupChat(VS_Container &cnt)
{
	const char* conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (!conf||!*conf)
		return false;
	const char *from = cnt.GetStrValueRef(FROM_PARAM);
	if (!from||!*from)
		return false;
	const char *msg = cnt.GetStrValueRef(MESSAGE_PARAM);
	if (!msg||!*msg)
		return false;

	bool res = false;
	auto dbStorage = g_dbStorage;
	if (dbStorage)
		res = dbStorage->LogGroupChat(conf, from, msg);

	return res;
}

void VS_VCSConfRestrict::LogRecordStart(const vs_conf_id& conf_id, const std::string& filename, std::chrono::system_clock::time_point started_at,
	VS_TransportRouterServiceReplyHelper* caller)
{
	if (!caller)
		return;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, LOGRECORDSTART_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, conf_id);
	cnt.AddValue(FILENAME_PARAM, filename.c_str());
	cnt.AddValue(START_TIME_PARAM, started_at);
	caller->PostRequest(caller->OurEndpoint(), 0, cnt, 0, LOG_SRV);
}

void VS_VCSConfRestrict::LogRecordStop(const vs_conf_id& conf_id, std::chrono::system_clock::time_point stopped_at, uint64_t file_size,
	VS_TransportRouterServiceReplyHelper* caller)
{
	if (!caller)
		return;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, LOGRECORDSTOP_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, conf_id);
	cnt.AddValue(STOP_TIME_PARAM, stopped_at);
	cnt.AddValueI64(SIZE_PARAM, file_size);
	caller->PostRequest(caller->OurEndpoint(), 0, cnt, 0, LOG_SRV);
}

bool VS_VCSConfRestrict::IsOperator(const vs_user_id& user)
{
	auto dbStorage = g_dbStorage;
	return (dbStorage)? dbStorage->IsOperator(user): false;
}

CMRFlags VS_VCSConfRestrict::GetCMRFlagsByLicense()
{
	CMRFlags flags = CMRFlags::ALL;
	if (!VS_CheckLicense(LE_SLIDESHOW))			flags &= ~CMRFlags::SS_SEND;
	if (!VS_CheckLicense(LE_WHITEBOARD))		flags &= ~CMRFlags::WB_SEND;
	if (!VS_CheckLicense(LE_FILETRANSFER))		flags &= ~CMRFlags::FT_SEND;
	if (!VS_CheckLicense(LE_DSHARING))			flags &= ~CMRFlags::DS_ALLOWED;
	if (!VS_CheckLicense(LE_VIDEORECORDING))	flags &= ~CMRFlags::REC_ALLOWED;
	return flags;
}

vs_conf_id VS_VCSConfRestrict::NewConfID(VS_SimpleStr _serverID)
{
	vs_conf_id newConf;

	assert(g_dbStorage);
	if (g_dbStorage)
	{
		newConf = g_dbStorage->NewConfID();
	}
	else
	{
		newConf.Append("00000000");
	}
	newConf.Append("@");
	newConf.Append(_serverID);
	return newConf;
}

namespace
{

void ParseSlideShowCmdParam(const std::string &params, const char *paramName, std::string &value)
{
	size_t pos1, pos2;

	value.clear();
	pos1 = params.find(paramName);
	if (pos1 != std::string::npos)
	{
		pos1 += strlen(paramName) + 1; // 1 is for '=' sign after param name
		pos2 = params.find("\n", pos1);

		if (pos2 != std::string::npos)
			value = params.substr(pos1, pos2 - pos1);
		else
			value = params.substr(pos1);
	}
}

}