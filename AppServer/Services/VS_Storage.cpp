/**
 ****************************************************************************
 * (c) 2007 Visicron Systems, Inc.  http://www.visicron.net/
 *
 * Project: Visicron server services
 *
 * $Revision: 47 $
 * $History: VS_Storage.cpp $
 *
 * *****************  Version 47  *****************
 * User: Mushakov     Date: 3.05.12    Time: 18:52
 * Updated in $/VSNA/Servers/AppServer/Services
 * - do not check mobile users in licence (restrictions are same as common
 * users)
 *
 * *****************  Version 46  *****************
 * User: Ktrushnikov  Date: 15.12.11   Time: 10:37
 * Updated in $/VSNA/Servers/AppServer/Services
 * - count CT_TRANSCODER separatly from online_users
 *
 * *****************  Version 45  *****************
 * User: Mushakov     Date: 2.06.11    Time: 21:11
 * Updated in $/VSNA/Servers/AppServer/Services
 * - '{' '}' in define of armadillo secure sections
 *
 * *****************  Version 44  *****************
 * User: Dront78      Date: 2.06.11    Time: 18:54
 * Updated in $/VSNA/Servers/AppServer/Services
 * - secured section fixed
 *
 * *****************  Version 43  *****************
 * User: Dront78      Date: 25.05.11   Time: 18:53
 * Updated in $/VSNA/Servers/AppServer/Services
 * - armadillo optimizations disabled totally
 *
 * *****************  Version 42  *****************
 * User: Ktrushnikov  Date: 11.02.11   Time: 16:58
 * Updated in $/VSNA/Servers/AppServer/Services
 * VCS 3.2
 * - License: add Roaming users to OnlineUsers of our server
 * - Join & ReqInvite check for license for Roaming users
 *
 * *****************  Version 41  *****************
 * User: Ktrushnikov  Date: 5.08.10    Time: 15:03
 * Updated in $/VSNA/Servers/AppServer/Services
 * - fix of null-time at LogConfStart
 *
 * *****************  Version 40  *****************
 * User: Mushakov     Date: 24.06.10   Time: 16:15
 * Updated in $/VSNA/Servers/AppServer/Services
 * - opt disabled when SECUREBEGIN_
 * - locator bs removed
 *
 * *****************  Version 39  *****************
 * User: Ktrushnikov  Date: 22.06.10   Time: 23:41
 * Updated in $/VSNA/Servers/AppServer/Services
 * Arch 3.1: NamedConfs added
 * - BS has Conference service
 * - Join_Method can create/join conf (if came from BS) or post request to
 * BS (if came from user)
 *
 * *****************  Version 38  *****************
 * User: Mushakov     Date: 19.04.10   Time: 19:49
 * Updated in $/VSNA/Servers/AppServer/Services
 * - 7281 (rights computation bug)
 *
 * *****************  Version 37  *****************
 * User: Mushakov     Date: 8.04.10    Time: 16:03
 * Updated in $/VSNA/Servers/AppServer/Services
 * - seporate counter for online_user realized
 *
 * *****************  Version 36  *****************
 * User: Ktrushnikov  Date: 23.03.10   Time: 14:08
 * Updated in $/VSNA/Servers/AppServer/Services
 * - CountOf Mobile, Guest, GW fixed (called twice for one user)
 * - Pass ClientType as CLIENTTYPE_PARAM (not as TYPE_PARAM)
 * - Pass ClientType for Authorize_Method (LDAP)
 *
 * *****************  Version 35  *****************
 * User: Mushakov     Date: 19.03.10   Time: 17:59
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - ClientType is received from client
 * - Endpoint function removed from code
 * - arr_key logged inRegistryServer
 *
 * *****************  Version 34  *****************
 * User: Mushakov     Date: 26.02.10   Time: 15:48
 * Updated in $/VSNA/servers/appserver/services
 *  - mobile client supported (AuthService)
 * - new cert
 *
 * *****************  Version 33  *****************
 * User: Dront78      Date: 22.02.10   Time: 14:10
 * Updated in $/VSNA/Servers/AppServer/Services
 * finished secured sections
 *
 * *****************  Version 32  *****************
 * User: Smirnov      Date: 15.02.10   Time: 18:50
 * Updated in $/VSNA/Servers/AppServer/Services
 * - licence restrictions reorganization
 * - SECUREBEGIN_A temporally removed
 *
 * *****************  Version 31  *****************
 * User: Smirnov      Date: 12.02.10   Time: 13:13
 * Updated in $/VSNA/Servers/AppServer/Services
 * - named group conf alfa
 *
 * *****************  Version 30  *****************
 * User: Ktrushnikov  Date: 28.01.10   Time: 17:32
 * Updated in $/VSNA/Servers/AppServer/Services
 * VCS:
 * - Interface added to ConfRestriction for SetUserStatus in Registry
 * - don't delete MultiConf registry key at startup of VCS (AS)
 *
 * *****************  Version 29  *****************
 * User: Mushakov     Date: 11.12.09   Time: 17:50
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - remive VCS_BUILD from VS_Storage
 *
 * *****************  Version 28  *****************
 * User: Stass        Date: 25.11.09   Time: 15:13
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 27  *****************
 * User: Ktrushnikov  Date: 6.11.09    Time: 19:55
 * Updated in $/VSNA/Servers/AppServer/Services
 * VS_ReadLicense
 * - fix with break
 * - LE_GUEST_LOGIN=9 added
 * VS_Storage
 * - m_num_guest - count of loggedin guests
 * VCSAuthService
 * - DeleteUser: guest from map ("registry")
 * VCSStorage
 * - DeleteUser added to interface
 * VS_RegistryStorage
 * - Guest login supported
 * - Check Guest password
 * - Check Count of Guests by License
 * - Read Shared Key from registry
 *
 * *****************  Version 26  *****************
 * User: Smirnov      Date: 5.11.09    Time: 15:50
 * Updated in $/VSNA/Servers/AppServer/Services
 * - leaks removed
 *
 * *****************  Version 25  *****************
 * User: Mushakov     Date: 4.11.09    Time: 18:49
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 2.11.09    Time: 14:51
 * Updated in $/VSNA/Servers/AppServer/Services
 * -store ab in vs_map
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 1.10.09    Time: 15:34
 * Updated in $/VSNA/Servers/AppServer/Services
 * - autoblock users with bad rating (bugfix#6354)
 *
 * *****************  Version 22  *****************
 * User: Ktrushnikov  Date: 3.09.09    Time: 12:03
 * Updated in $/VSNA/Servers/AppServer/Services
 * - AS abooks moved to VS_Storage
 *
 * *****************  Version 21  *****************
 * User: Mushakov     Date: 31.03.09   Time: 19:16
 * Updated in $/VSNA/Servers/AppServer/Services
 * Reg server added
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 27.12.08   Time: 20:19
 * Updated in $/VSNA/Servers/AppServer/Services
 * - fixed error with null user in resolve
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 27.09.08   Time: 21:43
 * Updated in $/VSNA/Servers/AppServer/Services
 * - removed groupconference storage
 * - removed unnecessary conference logging
 * - create conference and join rewrited
 *
 * *****************  Version 18  *****************
 * User: Mushakov     Date: 26.09.08   Time: 20:03
 * Updated in $/VSNA/Servers/AppServer/Services
 * - New Group Conf's atributes supported;
 *
 * *****************  Version 17  *****************
 * User: Mushakov     Date: 16.09.08   Time: 21:22
 * Updated in $/VSNA/Servers/AppServer/Services
 * - multiconf methods synchronized
 * - Checking conf_name in UpdateConference added
 *
 * *****************  Version 16  *****************
 * User: Mushakov     Date: 25.07.08   Time: 18:11
 * Updated in $/VSNA/Servers/AppServer/Services
 * - logging multiconf owner debbuged
 * - loging APP_ID added in multiconf
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 30.06.08   Time: 21:23
 * Updated in $/VSNA/Servers/AppServer/Services
 *  - bugfix #4519
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 24.06.08   Time: 17:15
 * Updated in $/VSNA/Servers/AppServer/Services
 * - some fix for group conf
 *
 * *****************  Version 13  *****************
 * User: Ktrushnikov  Date: 14.05.08   Time: 21:25
 * Updated in $/VSNA/Servers/AppServer/Services
 * - added ktrushnikov_Redirect
 *
 * *****************  Version 12  *****************
 * User: Stass        Date: 4.03.08    Time: 16:27
 * Updated in $/VSNA/Servers/AppServer/Services
 * new conf id to registry
 *
 * *****************  Version 11  *****************
 * User: Ktrushnikov  Date: 3.03.08    Time: 18:37
 * Updated in $/VSNA/Servers/AppServer/Services
 * - switch context from TransportRouter_Imp Thread to Service Thread by
 * using VS_ServiceHelper::PostLocalRequest()
 * - VS_Storage::FindUser(): check of input params added
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 4.02.08    Time: 16:27
 * Updated in $/VSNA/Servers/AppServer/Services
 * new id
 *
 * *****************  Version 9  *****************
 * User: Dront78      Date: 28.12.07   Time: 18:04
 * Updated in $/VSNA/Servers/AppServer/Services
 * Service updates.
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 27.11.07   Time: 18:54
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 26.11.07   Time: 19:55
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 26.11.07   Time: 17:51
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 26.11.07   Time: 16:08
 * Updated in $/VSNA/Servers/AppServer/Services
 * fixed storage
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 26.11.07   Time: 15:17
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 26.11.07   Time: 15:15
 * Updated in $/VSNA/Servers/AppServer/Services
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 22.11.07   Time: 20:34
 * Updated in $/VSNA/Servers/AppServer/Services
 * new iteration
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 15.11.07   Time: 15:28
 * Created in $/VSNA/Servers/AppServer/Services
 * updates
 *
 ****************************************************************************/

/**
 * \file VS_Storage.cpp
 * Server Database Storage class implementation functions
 *
 */

#include "VS_Storage.h"
#include "std/cpplib/VS_IntConv.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/VS_Utils.h"
#include "std/debuglog/VS_Debug.h"
#include "std/CallLog/VS_ParticipantDescription.h"

/// Conference  registry key
const char CONFERENCES_KEY[]			= "Conferences";

#define MONEY_WARN_MSG_INIT			  "You are about to run out of credit";

VS_Storage*	g_storage=0;

#define DEBUG_CURRENT_MODULE VS_DM_RSTOR

//Multi conferences
const char MMC_PASS_TAG[]         = "Password";
const char MMC_OWNER_TAG[]        = "Owner";
const char MMC_APPID_TAG[]        = "AppId";
const char MMC_TYPE_TAG[]         = "Type";
const char MMC_SUBTYPE_TAG[]      = "SubType";
const char MMC_FLAGS_TAG[]        = "Flags";
const char MMC_MAXPARTS_TAG[]     = "Max Participants";
const char MMC_CONFID_TAG[]		  = "ID";
const char MMC_PUBLIC_TAG[]		  = "Public";
const char MMC_TOPIC_TAG[]		  = "Topic";
const char MMC_LANG_TAG[]		  = "Lang";


VS_Storage::~VS_Storage()
{
	m_users.Clear();
};

static void TEstDes(void* p) {delete (VS_Storage::VS_UserBooks*)p;}
static void* VoidFactory ( const void* x ) {	return (void*) x; }


VS_Storage::VS_Storage(const char* serverID)
	: m_serverID(serverID)
	, m_money_warn_time(0)
	, m_money_warn_period(0)
	, m_money_warn_send_time(0)
	, m_tick(0)
	, m_groupConfNum(0)
	, m_num_guest(0)
	, m_num_roamingparts(0)
{
	memset(m_num_client_types, 0, sizeof(m_num_client_types));
	m_money_warn_msg = MONEY_WARN_MSG_INIT;

	VS_RegistryKey cfg_root(false, CONFIGURATION_KEY, false, true);
	char buff[1024];

	if (cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, CONF_MONEY_WARN_TAG)> 0) {
		unsigned int val = atou_s(buff);
		unsigned int min = atou_s(CONF_MONEY_WARN_MIN);
		unsigned int max = atou_s(CONF_MONEY_WARN_MAX);
		val = val>max?max:val<min?min:val;
		m_money_warn_time = val;
	}
	else m_money_warn_time = atou_s(CONF_MONEY_WARN_INIT);

	if (cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, CONF_MONEY_WARN_PERIOD_TAG)> 0) {
		unsigned int val = atou_s(buff);
		unsigned int min = atou_s(CONF_MONEY_WARN_PERIOD_MIN);
		unsigned int max = atou_s(CONF_MONEY_WARN_PERIOD_MAX);
		val = val>max?max:val<min?min:val;
		m_money_warn_period = val;
	}
	else m_money_warn_period = atou_s(CONF_MONEY_WARN_PERIOD_INIT);

	////////////////////////////////////////////////////////////
	//conferences
	//	Verify Registry Structure
	//VS_RegistryKey Mconf(false, "", false);
	//if (Mconf.IsValid())
	//	Mconf.RemoveKey(MULTI_CONFERENCES_KEY);

	m_users.SetPredicate(VS_Map::StrPredicate);
	m_users.SetKeyFactory(VS_Map::StrFactory, VS_Map::StrDestructor);
	m_users.SetDataFactory(VoidFactory, TEstDes);
}

void VS_Storage::SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface>& confResctrict)
{
	m_confResctrict = confResctrict;
}

void VS_Storage::SetUserStatus(const VS_SimpleStr& call_id,int status,const VS_ExtendedStatusStorage &extStatus)
{
	if (m_confResctrict)
		m_confResctrict->VCS_SetUserStatus(call_id, status,extStatus);
}

int VS_Storage::Init()
{
	int error_code=0;
	// Check !!!
	vs_conf_id cid;
	NewConfID(cid);
	if (cid==0)
	{
		error_code=VSS_CANT_MAKE_CONF_ID; // Can't generate new unique name
	}

	/// rights
	m_physRights= (VS_UserData::UserRights)
		(VS_UserData::UR_APP_MASK
		|VS_UserData::UR_COMM_PASSWORDMULTI
		|VS_UserData::UR_COMM_MULTI
		|VS_UserData::UR_COMM_EDITAB
		|VS_UserData::UR_COMM_UPDATEAB
		|VS_UserData::UR_COMM_SEARCHEXISTS
		|VS_UserData::UR_COMM_APPCALLLOG
		|VS_UserData::UR_COMM_CALL
		|VS_UserData::UR_COMM_FILETRANSFER
		|VS_UserData::UR_COMM_WHITEBOARD
		|VS_UserData::UR_COMM_SLIDESHOW
		|VS_UserData::UR_COMM_BROADCAST
		|VS_UserData::UR_COMM_CREATEMULTI );

	return error_code;
}

void VS_Storage::GetServerTime(std::chrono::system_clock::time_point& ftime)
{
	ftime = std::chrono::system_clock::now();
}

////////////////////////////////////////////////////
// conference management

void VS_Storage::NewConfID(vs_conf_id& newConf)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	while (true)
	{
		assert(m_confResctrict);
		if (m_confResctrict)
		{
			newConf = m_confResctrict->NewConfID(m_serverID);
		}
		// Check uniqueness
		if (m_conferences.find(SimpleStrToStringView(newConf)) == m_conferences.end())
			return;
	}
}


bool VS_Storage::FindConference(string_view cid, VS_ConferenceDescription& conf)
{
	if (cid.empty())
		return false;

	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	const auto it = m_conferences.find(cid);
	if (it == m_conferences.end())
		return false;

	conf = it->second;
	return true;
};

void VS_Storage::GetCurrentConferences(std::vector<VS_ConferenceDescription> &OUT_confs, const std::string& transceiverName) const{

	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	for (const auto& kv : m_conferences)
	{
		if (!kv.second.m_name)
			continue;
		if (!transceiverName.empty() && kv.second.m_transceiverName != transceiverName)
			continue;
		OUT_confs.emplace_back(kv.second);
	}
}

bool VS_Storage::FindConferenceByUser(string_view user, VS_ConferenceDescription& conf)
{
	if (user.empty())
		return false;

	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	const auto it = m_partIdx.find(user);
	if (it == m_partIdx.end())
		return false;

	return FindConference(SimpleStrToStringView(it->second.m_conf_id), conf);
}


vs_conf_id VS_Storage::FindConfIDByUser(string_view user)
{
	if (user.empty())
		return {};

	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	const auto it = m_partIdx.find(user);
	if (it == m_partIdx.end())
		return {};

	return it->second.m_conf_id;
}


int VS_Storage::InsertConference(VS_ConferenceDescription& conf)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	int error_code = VSS_CONF_ACCESS_DENIED;

	if (!conf.m_name) {
		NewConfID(conf.m_name);
	} else {
		if (m_conferences.find(SimpleStrToStringView(conf.m_name)) != m_conferences.end()) {
			auto pServerPart = VS_GetConfEndpoint(conf.m_name);
			const char * pConfId = conf.m_name;

			if (!pConfId || !pServerPart || pConfId >= pServerPart) return 0;
			vs_conf_id new_id(pServerPart - pConfId - 1, pConfId);

			new_id.Append("_0000");
			int counter = 1;
			char *p = new_id;
			p += new_id.Length() - 4;
			while (m_conferences.find(SimpleStrToStringView(new_id)) != m_conferences.end() && counter < 10000) {
				sprintf(p, "%04i", counter++);
			}

			new_id.Append("@");
			new_id.Append(pServerPart);
			conf.m_name = new_id;
		}
	}
	if (!!conf.m_name) {
		if (!conf.m_call_id) {
			union {
				uint64_t t64;
				uint8_t  t8[16];
			} s1;

			do VS_GenKeyByMD5(s1.t8); while (s1.t64 < 10e12);
			auto id = std::to_string(s1.t64);
			id.erase(id.begin(), id.end() - 12);
			conf.m_call_id = id.c_str();
		}
		GetServerTime(conf.m_logStarted);
		bool ret = m_conferences.try_emplace(std::string(SimpleStrToStringView(conf.m_name)), conf).second;
		if (ret) {
			if(conf.m_type>CT_PRIVATE_DIRECTLY)
				m_groupConfNum++;
		}
		ret = ret && m_confPartMap.emplace(conf.m_name, ConfPartInfo()).second;
		if (ret)
			error_code = 0;
	}
	return error_code;
}

bool VS_Storage::UpdateConference(const VS_ConferenceDescription& conf)
{
	if (!conf.m_name)
		return false;

	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	const auto it = m_conferences.find(SimpleStrToStringView(conf.m_name));
	if (it == m_conferences.end())
		return false;

	it->second = conf;
	return true;
}

bool VS_Storage::DeleteConference(VS_ConferenceDescription& conf)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	GetServerTime(conf.m_logEnded); // Needs to be under the lock because it retrieves current time.
	const auto it = m_conferences.find(SimpleStrToStringView(conf.m_name));
	if (it == m_conferences.end())
		return false;

	m_conferences.erase(it);
	if (conf.m_type > CT_PRIVATE_DIRECTLY)
			m_groupConfNum--;

	return m_confPartMap.erase(conf.m_name) == 1;
}

/*INS*********************************************************************************/

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning( disable : 4312 4311) //< void* <-> int conversion
#endif
int VS_Storage::NextConferenceMinute (VS_ParticipantDescription* &pd, int dif, VS_SystemMessage* serv)
{
	int NumOfPart = 0, j = 0;
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	m_tick+=dif;
	const bool send = m_tick > m_money_warn_send_time;
	if (send)
		m_money_warn_send_time = m_tick + m_money_warn_period*1000;

	double msec_passed = dif;

	static auto timeout = std::chrono::seconds(30);		// 30ms

	if (!m_partIdx.empty()) {
		pd = new VS_ParticipantDescription[m_partIdx.size()];
		for (auto& kv : m_partIdx) {
			double limit = kv.second.m_limit - kv.second.m_decLimit*msec_passed/60000.;
			double warn1_limit = kv.second.m_decLimit*m_money_warn_time/60.;

			if (send && limit<warn1_limit || limit < 0.0) {
				ReadParticipantLimit(kv.second);
				limit = kv.second.m_limit - 0.00001;
			}
			if (send && limit<warn1_limit) {
				//send warn
				if (!GetUserProperty(kv.second.m_user_id, "money_warn_msg", m_money_warn_msg))
					m_money_warn_msg = MONEY_WARN_MSG_INIT;

				char message[256];
				snprintf(message, 256, m_money_warn_msg.c_str(), (int)limit);
				serv->SendSystemMessage(message, kv.second.m_user_id);
			}
			kv.second.m_limit = limit;
			if (limit < 0.0) { // for remove
				kv.second.m_cause = kv.second.DISCONNECT_LIMIT;
				pd[j] = kv.second; j++;
			}
			if (kv.second.m_addledTick + timeout < std::chrono::steady_clock::now()) {	/// SMirnovK: removed && kv.second.m_version > 8
				kv.second.m_cause = kv.second.DISCONNECT_BYADDLING;
				pd[j] = kv.second; j++;
			}
		}
		if (!j) {
			delete[] pd; pd = NULL;
		}
	}

	return j;
}


int VS_Storage::AddParticipant(VS_ParticipantDescription& pd)
{
	int error_code = VSS_CONF_ACCESS_DENIED;
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	VS_ConferenceDescription cd;
	if (!FindConference(SimpleStrToStringView(pd.m_conf_id), cd)) {
		error_code = VSS_DB_MEM_ERROR;
	}
	else {
		auto i = m_confPartMap.find(pd.m_conf_id);
		auto &cpi = i->second;     // exists!

		if (cpi.confparts.size() >= cd.m_MaxParticipants) {
			error_code = VSS_CONF_MAX_PART_NUMBER;
		}
		else {
			if (m_partIdx.find(SimpleStrToStringView(pd.m_user_id)) != m_partIdx.end()) {
				error_code = VSS_DB_MEM_ERROR;        // participant exists
			}
			else {
				error_code = GetParticipantLimit(pd);
				if (error_code == 0)
				{
					pd.m_addledTick = std::chrono::steady_clock::now();
					GetServerTime(pd.m_joinTime);
					auto ins_res = m_partIdx.try_emplace(std::string(SimpleStrToStringView(pd.m_user_id)), pd);
					if (ins_res.second) {
						cpi.confparts[pd.m_user_id] = &ins_res.first->second;
						cpi.seq_id++;
					}
					error_code = 0;
				}
			}
		}
	}

	return error_code;
}

bool VS_Storage::AddParticipantToIgnoreList(const vs_conf_id& conf, const vs_user_id& user)
{
  bool ret = false;
  if (!conf || !user) return ret;

  {
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	auto it = m_confPartMap.find(conf);
	if (it != m_confPartMap.end())
		ret = it->second.ignore_list.emplace(user).second;
  }
  return ret;
}

bool VS_Storage::IsParticipantIgnored(const vs_conf_id& conf, const vs_user_id& user)
{
  bool ret = false;
  if (!conf || !user) return ret;

  {
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	auto it = m_confPartMap.find(conf);
	if (it != m_confPartMap.end())
		ret = it->second.ignore_list.count(user) != 0;
  }

  return ret;
}


bool VS_Storage::FindParticipant(string_view user, VS_ParticipantDescription& pd)
{
	if (user.empty())
		return false;

	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	const auto it = m_partIdx.find(user);
	if (it == m_partIdx.end())
		return false;

	pd = it->second;
	return true;
}


bool VS_Storage::UpdateParticipant(const VS_ParticipantDescription& pd)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	if (m_confPartMap.find(pd.m_conf_id) == m_confPartMap.end())
		return false;

	const auto it = m_partIdx.find(SimpleStrToStringView(pd.m_user_id));
	if (it == m_partIdx.end())
		return false;

	if (it->second.m_conf_id != pd.m_conf_id) {
		dstream2 << "g_storage::UpdateParticipant failed:" << it->second.m_conf_id.m_str << " changed to " << pd.m_conf_id.m_str;
		return false;
	}
	it->second = pd;
	return true;
}


bool VS_Storage::DeleteParticipant (VS_ParticipantDescription& pd)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	auto i = m_confPartMap.find(pd.m_conf_id);
	if (i != m_confPartMap.end()) {
		i->second.confparts.erase(pd.m_user_id);
		i->second.seq_id++;
	}
	const auto it = m_partIdx.find(SimpleStrToStringView(pd.m_user_id));
	if (it != m_partIdx.end())
		m_partIdx.erase(it);
	return true;
}

int VS_Storage::GetParticipants(const vs_conf_id & conf, std::vector<tPartServer> & part_server)
{
	int NumOfPart = 0;
	if (!conf) return 0;
	{std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	auto i = m_confPartMap.find(conf);
	if (i != m_confPartMap.end()) {
		auto &partMap = i->second.confparts;
		if ((NumOfPart = partMap.size()) > 0) {
			for (auto &ii : partMap) {
				VS_ParticipantDescription* ppd = ii.second;
				part_server.emplace_back(ppd->m_user_id, ppd->m_server_id);
			}
		}
	}}

	return NumOfPart;
}


int  VS_Storage::GetParticipants(const vs_conf_id& conf,  VS_ParticipantDescription* &pd, int rights)
{

	int NumOfPart = 0, j = 0;
	if (!conf) return j;
	{std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	auto i = m_confPartMap.find(conf);
	if (i != m_confPartMap.end()) {
		auto &partMap = i->second.confparts;
		if ((NumOfPart = partMap.size()) > 0) {
			pd = new VS_ParticipantDescription[NumOfPart];
			for (auto &ii: partMap) {
				VS_ParticipantDescription* ppd = ii.second;
				if (ppd->m_rights&rights) {
					pd[j] = *ppd; j++;
				}
			}
			if (!j) {
				delete[] pd; pd = NULL;
			}
		}
	}}

	return j;
}

void  VS_Storage::GetParticipants(const std::string &conf, std::vector<part_start_info> &OUT_call_ids, const int rights) const {
	{std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	const auto it = m_confPartMap.find(conf.c_str());
	if (it != m_confPartMap.end()) {
		for (auto &i : it->second.confparts) {
			VS_ParticipantDescription* p = i.second;
			if (!p) continue;

			const char *usId = p->m_user_id.m_str;
			if (usId && (p->m_rights&rights)) OUT_call_ids.emplace_back(usId, p->m_caps, p->m_lfltr, !p->m_displayName.empty() ? p->m_displayName : std::string(), p->m_role);
		}
	}
	}
}

std::string VS_Storage::GetOneTrascoderPartId(const vs_conf_id & conf)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	const auto it = m_confPartMap.find(conf);
	if (it != m_confPartMap.end()) {
		for (auto &i : it->second.confparts) {
			VS_ParticipantDescription* p = i.second;
			if (!!p->m_user_id && (p->m_ClientType == CT_TRANSCODER || p->m_ClientType == CT_TRANSCODER_CLIENT || p->m_ClientType == CT_WEB_CLIENT))
				return p->m_user_id.m_str;
		}
	}
	return std::string();
}

unsigned int VS_Storage::GetNumOfParts(const vs_conf_id& conf)
{
	int NumOfPart = 0;
	if (!conf) return NumOfPart;
	{std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	auto i = m_confPartMap.find(conf);
	if (i != m_confPartMap.end())
		NumOfPart = i->second.confparts.size();
	}
	return NumOfPart;
}

uint32_t VS_Storage::GetSeqId(const vs_conf_id & conf, bool inc)
{
	if (!conf)
		return 0;
	uint32_t seq = 0;
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	auto i = m_confPartMap.find(conf);
	if (i != m_confPartMap.end()) {
		if (inc)
			i->second.seq_id++;
		seq = i->second.seq_id;
	}
	return seq;
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
/*END INS*****************************************************************************/

void VS_Storage::SetPlannedParticipants(const vs_conf_id &conf, vs::set<vs_user_id> &set)
{
	if (set.empty())
		return;

	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	auto i = m_confPartMap.find(conf);
	if (i != m_confPartMap.end())
		i->second.planned_parts = std::move(set);
}

bool VS_Storage::GetPlannedParticipants(const vs_conf_id &conf, vs::set<vs_user_id> &set)
{
	set.clear();
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	auto i = m_confPartMap.find(conf);
	if (i != m_confPartMap.end())
		set = i->second.planned_parts;
	return !set.empty();
}

bool VS_Storage::IsPlannedParticipant(const vs_conf_id &conf, const vs_user_id &user)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	auto i = m_confPartMap.find(conf);
	if (i != m_confPartMap.end())
		return i->second.planned_parts.find(user) != i->second.planned_parts.end();
	return false;
}


int VS_Storage::FindConferences(VS_ConferenceDescription* &conf, const VS_SimpleStr& query)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	if (m_conferences.empty())
		return 0;

	if (query == "ALL")
	{
		const auto n_conf = m_conferences.size();
		conf = new VS_ConferenceDescription[n_conf];
		size_t i = 0;
		for (const auto& kv : m_conferences)
			conf[i++] = kv.second;
		return n_conf;
	}
	else if (query == "OLD")
	{
		std::chrono::system_clock::time_point current = std::chrono::system_clock::now();
		size_t n_conf = 0;
		for (const auto& kv : m_conferences)
			if (kv.second.m_timeExp < current)
				++n_conf;
		if (n_conf == 0)
			return 0;

		conf = new VS_ConferenceDescription[n_conf];
		size_t i = 0;
		for (const auto& kv : m_conferences)
			if (kv.second.m_timeExp < current)
				conf[i++] = kv.second;
		return n_conf;

	}
	return 0;
}


int VS_Storage::CountConferences()
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	return (int)m_conferences.size();
}

int VS_Storage::CountGroupConferences()
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	return m_groupConfNum;
}

int VS_Storage::CountParticipants()
{
	int ret = 0;
	{std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	ret = (int)m_partIdx.size();

	}
	return ret;
}

/// users
bool VS_Storage::FindUser(string_view id, VS_UserData& ud)
{
	if (id.empty())
		return false;

	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	const auto it = m_user_data.find(id);
	if (it == m_user_data.end())
		return false;

	ud = it->second;
	return true;
}

void VS_Storage::UpdateUser(string_view id, const VS_UserData& ud)
{
	/**
	do not check mobile users temporarily
	*/
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	const auto res = m_user_data.insert_or_assign(std::string(id), ud);
	if (res.second) // inserted new element
	{
		if (ud.m_type == VS_UserData::UT_GUEST)
			m_num_guest++;
		auto client_type = ud.m_client_type;
		if (client_type < CT_SIMPLE_CLIENT || client_type > CT_TRANSCODER_CLIENT)
			client_type = CT_SIMPLE_CLIENT;
		m_num_client_types[client_type]++;
	}
}

bool VS_Storage::DeleteUser(string_view id)
{
	/**
	do not check mobile users temporarily
	*/

	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	const auto it = m_user_data.find(id);
	if (it == m_user_data.end())
		return false;

	if (it->second.m_type == VS_UserData::UT_GUEST)
		m_num_guest--;
	auto client_type = it->second.m_client_type;
	if (client_type < CT_SIMPLE_CLIENT || client_type > CT_TRANSCODER_CLIENT)
		client_type = CT_SIMPLE_CLIENT;
	m_num_client_types[client_type]--;

	m_user_data.erase(it);
	return true;
}

int  VS_Storage::CountOnlineUsers()
{
	int num(0);
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	for (const auto& kv : m_user_data)
		if (!VS_IsNotTrueConfCallID(kv.first))
			++num;
	return num;
}


int VS_Storage::GetParticipantLimit (const vs_user_id& user_id, VS_ParticipantDescription::Type type, int& rights, double& limit, double& decLimit)
{
	VS_UserData ud;
	int error_code=VSS_CONF_ACCESS_DENIED;

	limit=0.;
	decLimit=0.;
	if(!FindUser(SimpleStrToStringView(user_id), ud))
	{
	if( type != VS_ParticipantDescription::PRIVATE_MEMBER
		&&type != VS_ParticipantDescription::MULTIS_MEMBER
		&&type != VS_ParticipantDescription::INTERCOM_MEMBER
		&&type != VS_ParticipantDescription::PUBLIC_MEMBER)
	{
	  return VSS_CONF_ACCESS_DENIED;
	}
	}
	VS_UserData::UserRights urights=ud.UR_NONE;
	FetchRights(ud, urights);


    // set rights
	rights = VS_ParticipantDescription::RIGHTS_NORMAL;
	bool allow=false;

	switch(type)
	{
	case VS_ParticipantDescription::PRIVATE_HOST:
	case VS_ParticipantDescription::HPRIVATE_HOST:
    if(urights&ud.UR_COMM_CALL)
    {
		rights|= VS_ParticipantDescription::RIGHTS_RCV_CHAT;
		rights|= VS_ParticipantDescription::RIGHTS_RCV_LIST;
		limit=1.;
		allow=true;
    }
    else
		error_code=VSS_CONF_ACCESS_DENIED;

		break;

	case VS_ParticipantDescription::PUBLIC_HOST:
    if(urights&VS_UserData::UR_COMM_BROADCAST)
    {
		rights|= VS_ParticipantDescription::RIGHTS_RCV_LIST;
		rights|= VS_ParticipantDescription::RIGHTS_RCV_CHAT;
	    limit=1.;
		allow=true;
    }
    else
		error_code=VSS_CONF_ACCESS_DENIED;

		break;

	case VS_ParticipantDescription::PRIVATE_MEMBER:
	case VS_ParticipantDescription::PUBLIC_MEMBER:
	case VS_ParticipantDescription::HPRIVATE_MEMBER:
	case VS_ParticipantDescription::MULTIS_MEMBER:
	case VS_ParticipantDescription::INTERCOM_MEMBER:
		rights|= VS_ParticipantDescription::RIGHTS_RCV_CHAT;
		rights|= VS_ParticipantDescription::RIGHTS_RCV_LIST;
		limit=1.;
		allow=true;
		break;

	case VS_ParticipantDescription::HPRIVATE_GUEST:
		limit=1.;
		allow=true;
		break;

	default:
		limit=1.;
		allow=true;
		break;
  }
	if(allow)
		error_code=0;

	return error_code;
}

bool  VS_Storage::FetchRights(const VS_UserData& ud, VS_UserData::UserRights& rights)
{
	uint32_t grpRights=ud.UR_NONE;
	uint32_t srvRights  =m_physRights;
	rights=ud.UR_NONE;

	/// server capable of following rights, based on types
	if(ud.m_type==ud.UT_PERSON)
		srvRights|=ud.UR_APP_COMMUNICATOR;



	rights=(VS_UserData::UserRights)(rights|ud.UR_LOGIN);

	rights= (VS_UserData::UserRights)
		(rights | (srvRights) );

	return true;
}

int VS_Storage::GetParticipantLimit(VS_ParticipantDescription& pd)
{
  return GetParticipantLimit(pd.m_user_id,(VS_ParticipantDescription::Type)pd.m_type,
                              pd.m_rights, pd.m_limit, pd.m_decLimit);
}

int VS_Storage::CheckParticipant(const vs_user_id& user_id,VS_ParticipantDescription::Type type,const vs_user_id& owner,const vs_user_id& party)
{
  int rights;
  double limit, decLimit;

  return GetParticipantLimit(user_id,type,
                              rights, limit, decLimit);
}


void VS_Storage::CountOfAll(int &OUsers, int &Confs, int &Parts)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	OUsers = (int)m_user_data.size();
	Confs = (int)m_conferences.size();
	Parts = (int)m_partIdx.size();

}

//----------------------------------------------------------------------------------
int VS_Storage::FindConferenceByName(string_view user_id, VS_ConferenceDescription& conf)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	for (const auto& kv : m_conferences)
		if (SimpleStrToStringView(kv.second.m_owner) == user_id)
		{
			conf = kv.second;
			return 0;
		}
	return VSS_CONF_NOT_FOUND;
}

int VS_Storage::FindConferenceByCallID(string_view call_id, VS_ConferenceDescription& conf)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	for (const auto& kv : m_conferences)
		if (SimpleStrToStringView(kv.second.m_call_id) == call_id)
		{
			conf = kv.second;
			return 0;
		}
	return VSS_CONF_NOT_FOUND;
}

bool VS_Storage::IsConferenceRecording(string_view conf_id) const
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	const auto it = m_conferences.find(conf_id);
	return it != m_conferences.end() ? it->second.m_need_record : false;
}

VS_Conference_Type VS_Storage::GetConfType(string_view conf_id) const
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	const auto it = m_conferences.find(conf_id);
	return it != m_conferences.end() ? static_cast<VS_Conference_Type>(it->second.m_type) : CT_UNDEFINED;
}

uint32_t VS_Storage::GetUsers(vs_user_id* &users)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	size_t i = 0;
	users = new vs_user_id[m_user_data.size()];
	for (const auto& kv : m_user_data)
		users[i++] = kv.second.m_name;
	return m_user_data.size();
}

uint32_t VS_Storage::GetUsersFiltered(uint32_t n, const VS_ClientType type, std::vector<vs_user_id>& OUTusers, bool fetch_guests, std::vector<vs_user_id>* except_of)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	uint32_t found = 0;
	for (const auto& kv : m_user_data)
	{
		if (n == 0)
			break;
		if (except_of && std::find(except_of->begin(), except_of->end(), kv.second.m_name) != except_of->end())
			continue;

		if (fetch_guests ? kv.second.m_type == VS_UserData::UT_GUEST : kv.second.m_client_type == type)
		{
			OUTusers.push_back(kv.second.m_name);
			--n;
			++found;
		}
	}
	return found;
}

VS_Storage::VS_UserBook* VS_Storage::GetUserBook(const char* callId, VS_AddressBook ab, bool doCreate)
{
	if (!callId || !*callId)
		return 0;

	VS_UserBooks *books = 0;

	VS_Map::Iterator i = m_users.Find(callId);
	if (i==m_users.End()) {
		if (doCreate) {
			books = new VS_UserBooks;
			m_users.Insert(callId, books);
		} else
			return 0;
	}
	else {
		books = (VS_UserBooks*)i->data;
	}

	if (!books)
		return 0;

	if (ab == AB_COMMON)
		return &books->ab_common;
	else if(ab == AB_BAN_LIST)
		return &books->ab_ban;
	else if(ab == AB_INVERSE)
		return &books->ab_inverse;
	else
		return 0;
}

void VS_Storage::StartInvitationProcess(const InviteInfo &info)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	if (std::find(m_pendingInvites.begin(), m_pendingInvites.end(), info) == m_pendingInvites.end())
		m_pendingInvites.push_back( info );
}
bool VS_Storage::FindInvitation(const vs_conf_id &conf, const vs_user_id &uid, InviteInfo &info)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	auto res = std::find_if(m_pendingInvites.begin(), m_pendingInvites.end(), [&conf, &uid](const InviteInfo &x)
	{
		return x.conf == conf && uid == x.user;
	});
	auto uid_sv = SimpleStrToStringView(uid);
	if (res == m_pendingInvites.end() && !VS_IsRTPCallID(uid_sv))  // re-try with hunt_group
	{
		auto hunt_group = VS_RemoveTranscoderID_sv(SimpleStrToStringView(uid));
		res = std::find_if(m_pendingInvites.begin(), m_pendingInvites.end(), [&conf, hunt_group](const InviteInfo &x)
		{
			return x.conf == conf && SimpleStrToStringView(x.user) == hunt_group;
		});
	}
	if (res == m_pendingInvites.end())
		return false;
	info = *res;
	return true;
}
bool VS_Storage::EndInvitationProcess(const vs_conf_id &conf, const vs_user_id &uid, InviteInfo &erased)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);

	auto res = std::find_if(m_pendingInvites.begin(), m_pendingInvites.end(), [&conf, &uid](const InviteInfo &x)
	{
		return x.conf == conf && uid == x.user;
	});
	auto uid_sv = SimpleStrToStringView(uid);
	if (res == m_pendingInvites.end() && !VS_IsRTPCallID(uid_sv))  // re-try with hunt_group
	{
		auto hunt_group = VS_RemoveTranscoderID_sv(uid_sv);
		res = std::find_if(m_pendingInvites.begin(), m_pendingInvites.end(), [&conf, hunt_group](const InviteInfo &x)
		{
			return x.conf == conf && SimpleStrToStringView(x.user) == hunt_group;
		});
	}
	if (res == m_pendingInvites.end())
		return false;
	erased = std::move(*res);
	m_pendingInvites.erase(res);
	return true;
}
bool VS_Storage::EndInvitationProcess(const vs_conf_id &conf, const vs_user_id &uid)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	auto sz = m_pendingInvites.size();
	auto hunt_group = VS_RemoveTranscoderID_sv(SimpleStrToStringView(uid));
	auto it = std::remove_if(m_pendingInvites.begin(), m_pendingInvites.end(), [&conf, &uid, hunt_group](const InviteInfo &x) {
		return x.conf == conf && x.user == uid;
	});
	if (it != m_pendingInvites.end()) {
		m_pendingInvites.erase(it, m_pendingInvites.end());
	} else if (!VS_IsRTPCallID(hunt_group)) { // re-try with hunt_group
		m_pendingInvites.erase(std::remove_if(m_pendingInvites.begin(), m_pendingInvites.end(), [&conf, hunt_group](const InviteInfo &x) {
			return x.conf == conf && SimpleStrToStringView(x.user) == hunt_group;
		}), m_pendingInvites.end());
	}
	return sz != m_pendingInvites.size();
}
void VS_Storage::GetTimedoutInvites( std::vector<InviteInfo> &res, InviteInfo::ConfType type)
{
	std::lock_guard<decltype(m_common_lock)> _(m_common_lock);
	unsigned j(0);
	res.clear();

	for (unsigned i = 0; i < m_pendingInvites.size(); i++)
		if ( m_pendingInvites[i].timeout <= std::chrono::steady_clock::now() && m_pendingInvites[i].conf_type == type)
			res.push_back( m_pendingInvites[i] );
		else m_pendingInvites[ j++ ] = m_pendingInvites[ i ];

		m_pendingInvites.resize( j );
}