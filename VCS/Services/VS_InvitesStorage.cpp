#include "VS_InvitesStorage.h"

#include <wchar.h>
#include <iomanip>
#include <sstream>
#include <cstdint>

#include "ProtectionLib/Protection.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "../../common/statuslib/VS_CallIDInfo.h"
#include "std/cpplib/curl_deleters.h"
#include "std-generic/clib/strcasecmp.h"
#include "std-generic/clib/vs_time.h"
#include "std-generic/cpplib/string_view.h"

#include <curl/curl.h>

// for 100-nanoseconds intervals (http://support.microsoft.com/kb/188768)
#define _SECOND ((int64_t) 10000000)
#define _MINUTE (60 * _SECOND)
#define _HOUR   (60 * _MINUTE)
#define _DAY    (24 * _HOUR)

#define DEBUG_CURRENT_MODULE VS_DM_MCS

const char MMC_INVITATIONUPDATE[]	= "Invitation Update";
const char MMC_INVITEDTIME[]		= "Invited Time";
const char MMC_MISSEDCONFTIME[]		= "Missed Conf Email Time";
const char MMC_CONFID_TAG[]			= "ID";
const char MMC_OWNER_TAG[]			= "Owner";
const char MMC_LASTINVITEDTOSTREAMID_TAG[] = "LastInvitedToStreamID";

static char admin_email[256]		= {0};
const int VS_InvitesStorage::INVITE_INTERVAL = 15;		// in minutes

#define NEXT_DAY_OF_WEEK(day) (day < (1<<6))? day << 1: 1

static std::string GetStreamID(string_view conf_call_id) noexcept
{
	if (conf_call_id.empty())
		return {};
	std::string key_name;
	key_name.reserve(128);
	key_name += MULTI_CONFERENCES_KEY;
	key_name += '\\';
	key_name += conf_call_id;
	std::string value;
	VS_RegistryKey(false, key_name).GetString(value, MMC_CONFID_TAG);
	return value;

}

static bool IsConf(string_view conf_call_id) noexcept
{
	return !GetStreamID(conf_call_id).empty();
}

VS_InvitesStorage::VS_InvitesStorage(): m_last_update_time(0)
{
	{
		std::lock_guard<std::mutex> lock(m_lock);

		VS_RegistryKey key(false, CONFIGURATION_KEY);
		key.GetValue(admin_email, sizeof(admin_email) - 1, VS_REG_STRING_VT, SMTP_ADMIN_EMAIL_TAG);

		m_confs.SetPredicate(vs_conf_id::Predicate);
		m_confs.SetKeyFactory(vs_conf_id::Factory, vs_conf_id::Destructor);
		m_confs.SetDataFactory(ConferenceInvitation::Factory, ConferenceInvitation::Destructor);
	}
	if (IsRegistryChanged())
		ReadFromRegistry();
}

VS_InvitesStorage::~VS_InvitesStorage()
{
	std::lock_guard<std::mutex> lock(m_lock);
	m_confs.Clear();
}

#include "ProtectionLib/OptimizeDisable.h"
void VS_InvitesStorage::OnTimer_SendInvites(std::vector<BSEvent>& v)
{
	if (IsRegistryChanged()) ReadFromRegistry();
	std::lock_guard<std::mutex> lock(m_lock);
	dprint3("INV: Tick. Confs to invite: %zu\n", m_confs.Size());
	VS_RealUserLogin r_adm("Administrator");

SECUREBEGIN_A_FULL;
	VS_Map::Iterator it = m_confs.Begin();
	while (it!=m_confs.End())
	{
		ConferenceInvitation *cd = (ConferenceInvitation*)(*it).data;
		if (cd->m_invitation_type < 0 && !cd->m_force_invite)
		{
			++it;
			continue;
		}

		const char* from_email(0);
		const char* ownerDN(0);
		if (cd->m_owner.empty()) {
			from_email = *admin_email ? admin_email : (const char*)r_adm;
			ownerDN = from_email;
		}
		else {
			from_email = cd->m_owner.c_str();
			VS_UserData ud;
			ownerDN = g_storage->FindUser(cd->m_owner, ud) ? ud.m_displayName.c_str() : from_email;
		}

		auto createCnt = [&](const char* touser, std::chrono::system_clock::time_point & tp) {
			VS_Container* cnt = new VS_Container;
			cnt->AddValue(METHOD_PARAM, SENDMAIL_METHOD);
			cnt->AddValueI32(TYPE_PARAM, 4);

			cnt->AddValue(CALLID_PARAM, from_email);
			cnt->AddValue(DISPLAYNAME_PARAM, ownerDN);
			cnt->AddValue(CALLID2_PARAM, touser);
			cnt->AddValue(TIME_PARAM, tp);

			cnt->AddValue(CONFERENCE_PARAM, cd->m_conf_id.c_str());
			cnt->AddValue(TOPIC_PARAM, cd->m_topic);
			cnt->AddValue(PASSWORD_PARAM, cd->m_password);
			return cnt;
		};

		if (IsInviteNeeded(*cd))
		{
			dprint3("INV: Try Send Invites for conf: %s\n", cd->m_conf_id.c_str());

			BSEvent ev;
			ev.cnt = new VS_Container;
			std::vector<std::string> offline_parts;

			FillParticipants(cd, ev, offline_parts);

			if (ev.cnt->IsValid())
			{
				ev.to_service = CONFERENCE_SRV;
				ev.cnt->AddValue(METHOD_PARAM, INVITEUSERS_METHOD);
				ev.cnt->AddValue(CONFERENCE_PARAM, cd->m_conf_id);
				ev.cnt->AddValue(PASSWORD_PARAM, cd->m_password);
				v.push_back(ev);
			} else {
				delete ev.cnt;
			}

			// email offline participants
			std::chrono::system_clock::time_point ft;
			g_storage->GetServerTime(ft);
			for (std::vector<std::string>::iterator it=offline_parts.begin(); it!=offline_parts.end(); ++it)
			{
				BSEvent ev_offline;
				ev_offline.to_service = LOG_SRV;
				ev_offline.cnt = createCnt(it->c_str(), ft);
				v.push_back(ev_offline);
			}
		}

		// need send email with file_ics?
		if (cd->m_send_invitation==1 && !cd->m_send_invitation_done && !cd->m_file_ics.empty())
		{
			std::string key_name;
			key_name.reserve(128);
			key_name += MULTI_CONFERENCES_KEY;
			key_name += '\\';
			key_name += cd->m_conf_id;
			VS_RegistryKey key_check(false, key_name);
			if (key_check.IsValid())
			{
				// send email
				for(InvPartsMap::iterator it=cd->m_invitation_parts.begin(); it!=cd->m_invitation_parts.end(); ++it)
				{
					const char* user = it->first.GetID();
					if (!user || !*user)
						continue;

					BSEvent ev_offline;
					ev_offline.to_service = LOG_SRV;
					ev_offline.cnt = createCnt(user, cd->m_invitation_start_time);
					ev_offline.cnt->AddValue("file_ics", cd->m_file_ics);
					v.push_back(ev_offline);
				}

				// mark as sent
				cd->m_send_invitation_done = true;
				VS_RegistryKey key(false, key_name, false, true);
				key.SetValue(&cd->m_send_invitation_done, sizeof(cd->m_send_invitation_done), VS_REG_INTEGER_VT, "Send Invitation Done");
			}
		}

		cd->m_force_invite = false;
		/////////

		++it;
	}
SECUREEND_A_FULL;
}

// Temporary hack to fix VMProtect on Linux64.
// VMProtect 3.4.0 (build 1166) fails to correctly find the end of the function.
// As a workaround we instruct compiler to inline this function at its (single) call site.
#if defined(ENABLE_VMPROTECT_BUILD) && defined(__GNUC__) && defined(__x86_64__)
__attribute__ ((always_inline)) inline
#endif
void VS_InvitesStorage::FillParticipants(ConferenceInvitation* cd, BSEvent& ev, std::vector<std::string>& offline_users)
{
	if (!cd)
		return;

	std::unique_ptr<CURL, CURL_deleter> curl(::curl_easy_init());
	if (!curl) return;

	char time_str[256] = {0};
	tu::TimeToRuStr(std::chrono::system_clock::now(), time_str, 256);

	auto stream_id = GetStreamID(cd->m_conf_id);

SECUREBEGIN_A_FULL;
	InvPartsMap::iterator it = cd->m_invitation_parts.begin();
	while (it != cd->m_invitation_parts.end())
	{
		const auto& r = it->first;
		std::string user = (r.IsOurSID()) ? r.GetUser() : r.GetID();
		std::unique_ptr<char, curl_free_deleter> escaped_user(::curl_easy_escape(curl.get(), user.c_str(), user.length()));
		if (!escaped_user) { ++it; continue; }

		std::string key_name;
		key_name.reserve(128);
		key_name += MULTI_CONFERENCES_KEY;
		key_name += '\\';
		key_name += cd->m_conf_id;
		key_name += "\\Participants\\";
		key_name += escaped_user.get();
		VS_RegistryKey key(false, key_name, false, false);

		VS_CallIDInfo ci;
		VS_SimpleStr tmp = user.c_str();
		VS_UserPresence_Status status = m_presenceService->Resolve(tmp, ci, true, 0, false);

		bool IsNotTC = VS_IsNotTrueConfCallID(tmp.m_str);
		bool IsRoaming = !VS_RealUserLogin(SimpleStrToStringView(tmp)).IsOurSID();
		bool DoCheckStatus = !IsNotTC && !IsRoaming;		// check status only for local TrueConf users

		bool DoInvite = true;
		if (DoCheckStatus)
			if (status!=USER_AVAIL)
				DoInvite = false;

		std::string last_conf;
		key.GetString(last_conf, MMC_LASTINVITEDTOSTREAMID_TAG);
		const bool new_conf = (last_conf != stream_id);

		if (DoInvite && (IsOutside(time_str, GetInvitedTime(it).c_str()) || cd->m_force_invite || new_conf)) {		// TODO: check user invited time with now_time and now_time+INVITE_INTERVAL
			dprint3("INV: Invite user %s to %s conference\n", user.c_str(), cd->m_conf_id.c_str());

			ev.cnt->AddValue(CALLID_PARAM, user);
			key.SetString(time_str, MMC_INVITEDTIME);
			key.SetString(stream_id.c_str(), MMC_LASTINVITEDTOSTREAMID_TAG);

			it->second = make_pair(std::string(time_str), std::string(time_str));
		} else if (status!=USER_AVAIL && IsOutside(time_str, GetMissedConfEmailTime(it).c_str())) {
			dprint3("INV: Notify %s(%d) by e-mail about %s conference\n", user.c_str(), status, cd->m_conf_id.c_str());
			offline_users.push_back(user);
			key.SetString(time_str, MMC_MISSEDCONFTIME);

			it->second = make_pair(GetInvitedTime(it), std::string(time_str));
		}

		++it;
	}
SECUREEND_A_FULL;
}

void VS_InvitesStorage::ReadFromRegistry()
{
	dprint3("INV: ReadFromRegistry()\n");

	VS_RegistryKey root(false, MULTI_CONFERENCES_KEY);
	VS_RegistryKey key;
	root.ResetKey();

	if (!root.IsValid())
		return ;

	std::unique_ptr<CURL, CURL_deleter> curl(::curl_easy_init());
	if (!curl) return;

	std::lock_guard<std::mutex> lock(m_lock);
	m_confs.Clear();

SECUREBEGIN_A_FULL;
	while (root.NextKey(key))
	{
		ConferenceInvitation cd;
		if (key.GetName() != nullptr)
		{
			cd.m_conf_id = key.GetName();
		}
		key.GetString(cd.m_file_ics, "file_ics");
		key.GetString(cd.m_topic, "Topic");
		key.GetValue(&cd.m_invitation_type, sizeof(cd.m_invitation_type), VS_REG_INTEGER_VT, "Invitation Type");
		key.GetValue(&cd.m_invitation_time, sizeof(cd.m_invitation_time), VS_REG_INTEGER_VT, "Invitation Time");
		key.GetValue(&cd.m_invitation_day, sizeof(cd.m_invitation_day), VS_REG_INTEGER_VT, "Invitation Day");
		key.GetValue(&cd.m_send_invitation, sizeof(cd.m_send_invitation), VS_REG_INTEGER_VT, "Send Invitation");
		key.GetValue(&cd.m_send_invitation_done, sizeof(cd.m_send_invitation_done), VS_REG_INTEGER_VT, "Send Invitation Done");
		key.GetValue(&cd.m_auto_invite, sizeof(cd.m_auto_invite), VS_REG_INTEGER_VT, "auto_invite");

		if (cd.m_invitation_type==0)	// regular
		{
			auto utc_now = time(nullptr);
			std::tm tm_utc_now;
			gmtime_r(&utc_now, &tm_utc_now);

			std::tm tm_conf_time = tm_utc_now;
			tm_conf_time.tm_hour = cd.m_invitation_time / 60;
			tm_conf_time.tm_min = cd.m_invitation_time % 60;
			tm_conf_time.tm_sec = 0;

			auto utc_conf_time = std::mktime(&tm_conf_time);
			gmtime_r(&utc_conf_time, &tm_conf_time);

			int when = cd.m_invitation_day;
			int day = 1 << tm_conf_time.tm_wday;
			int i =0;
			while (i < 10)
			{
				if (day & when)
				{
					if (i == 0)	// today
					{
						if (tm_utc_now.tm_hour <= tm_conf_time.tm_hour && tm_utc_now.tm_min <= tm_conf_time.tm_min)	// if it will be conf today, else try next day
							break;
					}else{
						break;
					}
				}
				day = NEXT_DAY_OF_WEEK(day);
				++i;
				utc_conf_time += 3600 * 24;	// add 24 hours
			}
			cd.m_invitation_start_time = std::chrono::system_clock::from_time_t(utc_conf_time);
		} else if (cd.m_invitation_type == 1) {		// one-time conf
			std::string invitation_date;
			if (key.GetString(invitation_date, "Invitation Date") && !invitation_date.empty())
			{
				cd.m_invitation_start_time = ConferenceInvitation::CalculateInvitationStartTime(cd, invitation_date.c_str());
			}
		}

		{
			// TODO: read Participants
			std::string key_name;
			key_name.reserve(128);
			key_name += MULTI_CONFERENCES_KEY;
			key_name += '\\';
			key_name += key.GetName();
			key_name += "\\Participants";
			VS_RegistryKey parts_root(false, key_name);
			VS_RegistryKey part;
			parts_root.ResetKey();

			while(parts_root.NextKey(part))
			{
				const char* unescaped_call_id = part.GetName();
				int out_len;
				std::unique_ptr<char, curl_free_deleter> call_id(::curl_easy_unescape(curl.get(), unescaped_call_id, strlen(unescaped_call_id), &out_len));

				if (call_id && *call_id)
				{
					std::string invited;
					part.GetString(invited, MMC_INVITEDTIME);
					std::string emailed;
					part.GetString(emailed, MMC_MISSEDCONFTIME);
					cd.m_invitation_parts[VS_RealUserLogin(call_id.get())] = make_pair(std::move(invited), std::move(emailed));
				}
			}

			if (!cd.m_invitation_parts.size())
				continue;

			{
				std::unique_ptr<char, free_deleter> tmp_buf;
				key.GetValue(tmp_buf, VS_REG_STRING_VT, MMC_OWNER_TAG);
				if (tmp_buf)
				{
					cd.m_owner = tmp_buf.get();
				}
			}
/*			if (*owner)
			{
				VS_StorageUserData ude;
				g_dbStorage->FindUser(owner, (VS_StorageUserData&) ude);
				cd.m_owner = ude.m_email;
			}
*/
			// add to map
			{
				VS_SimpleStr conf_id = cd.m_conf_id.c_str();
				VS_Map::ConstIterator i = m_confs.Find(conf_id);
				if (i == m_confs.End())
					m_confs.Insert(conf_id, &cd);
			}
		}
	}
	root.GetValue(&m_last_update_time, sizeof(m_last_update_time), VS_REG_INTEGER_VT, MMC_INVITATIONUPDATE);
SECUREEND_A_FULL;
}
#include "ProtectionLib/OptimizeEnable.h"

bool VS_InvitesStorage::IsRegistryChanged() const
{
	VS_RegistryKey key(false, MULTI_CONFERENCES_KEY, false, false);

	uint32_t last_update = 0;
	if (!key.GetValue(&last_update, sizeof(last_update), VS_REG_INTEGER_VT, MMC_INVITATIONUPDATE))
		return false;

	{
		std::lock_guard<std::mutex> lock(m_lock);
		if (m_last_update_time == last_update)
			return false;
	}

	return true;
}

void VS_InvitesStorage::OnUserLoggedIn(const std::string& call_id, VS_Container& cnt)
{
	if (call_id.empty())
		return ;

	std::unique_ptr<CURL, CURL_deleter> curl(::curl_easy_init());
	if (!curl) return;

	dprint3("INV: OnUserLoggedIn(%s)\n", call_id.c_str());

	char time_str[256] = {0};
	tu::TimeToRuStr(std::chrono::system_clock::now(), time_str, 256);

	std::lock_guard<std::mutex> lock(m_lock);

	for(VS_Map::Iterator it = m_confs.Begin(); it != m_confs.End(); ++it)
	{
		if (!it->data)
			continue;

		ConferenceInvitation* cd = (ConferenceInvitation*) it->data;
		if (!IsInviteNeeded(*cd))
			continue;

		for(InvPartsMap::iterator it2=cd->m_invitation_parts.begin(); it2!=cd->m_invitation_parts.end(); ++it2)
		{
			std::string invited_time = GetInvitedTime(it2);
			if (!invited_time.empty())
			{
				if (!IsOutside(time_str, invited_time.c_str()))		//     
					continue;
			}

			if (call_id == it2->first.GetID())
			{
				dprint3("INV: Invite user %s to %s conference\n", call_id.c_str(), cd->m_conf_id.c_str());

				cnt.AddValue(CALLID_PARAM, call_id);
				cnt.AddValue(METHOD_PARAM, INVITEUSERS_METHOD);
				cnt.AddValue(CONFERENCE_PARAM, cd->m_conf_id);
				cnt.AddValue(PASSWORD_PARAM, cd->m_password);

				std::unique_ptr<char, curl_free_deleter> escaped_call_id(::curl_easy_escape(curl.get(), call_id.c_str(), call_id.length()));
				if (!escaped_call_id) continue;

				std::string key_name;
				key_name.reserve(128);
				key_name += MULTI_CONFERENCES_KEY;
				key_name += '\\';
				key_name += cd->m_conf_id;
				key_name += "\\Participants\\";
				key_name += escaped_call_id.get();
				VS_RegistryKey key(false, key_name, false, false);

				key.SetString(time_str, MMC_INVITEDTIME);

				it2->second = make_pair(std::string(time_str), GetMissedConfEmailTime(it2));
			}
		}
	}
}

bool VS_InvitesStorage::IsOutside(const char* now, const char* to_check)
{
	// strings in format "dd.mm.yyyy hh:mm:ss"
	if (!now || !to_check || strlen(now)<19 || strlen(to_check)<19)
		return true;

	if (strncasecmp(now, to_check, 10)!=0)		// compare date
		return true;

	int nh = atoi(now+11);
	int nm = atoi(now+14);
	int n = nh*60 + nm;

	int th = atoi(to_check+11);
	int tm = atoi(to_check+14);
	int t = th*60 + tm;

	if (n>=t && n<=(t+INVITE_INTERVAL))
		return false;							// already invited

	return true;
}

bool VS_InvitesStorage::IsInviteNeeded(const ConferenceInvitation& cd)
{
	if (cd.m_force_invite) {
		return true;
	}

	if (cd.m_invitation_type == 0)
		return IsInviteNeeded_Regular(cd);
	else if (cd.m_invitation_type == 1)
		return IsInviteNeeded_Once(cd);
	else
		return false;
}

/*#include <iostream>
#include <ctime>*/
bool VS_InvitesStorage::IsInviteNeeded_Once(const ConferenceInvitation& cd)
{
	auto end = cd.m_invitation_start_time;	end += std::chrono::minutes(INVITE_INTERVAL);
	auto now = std::chrono::system_clock::now();

	/*{
		auto tnow = std::chrono::system_clock::to_time_t(now);
		auto tstart = std::chrono::system_clock::to_time_t(cd.m_invitation_start_time);
		auto tend = std::chrono::system_clock::to_time_t(end);
		std::string snow = ctime(&tnow);
		std::string sstart = ctime(&tstart);
		std::string send = ctime(&tend);
		std::cout << "IsInviteNeeded_Once: now = " << snow << ", invitation start = "
			<< sstart << ", end = " << send << std::endl;
	}*/

	bool time_to_invite = (cd.m_invitation_start_time < now && now < end);
	if (!time_to_invite)
		return false;

	char ft_now[256] = {0};
	if (!tu::TimeToRuStr(now, ft_now, 256)) return false;

	std::string invited_time;		GetConfInviteTime(cd.m_conf_id.c_str(), invited_time);
	bool is_invited = (!invited_time.empty())? !IsOutside(ft_now, invited_time.c_str()): false;

	return !is_invited || IsConf(cd.m_conf_id); //     
}

bool VS_InvitesStorage::IsInviteNeeded_Regular(const ConferenceInvitation& cd)
{
	char ft_now[256] = {0};
	tu::TimeToRuStr(std::chrono::system_clock::now(), ft_now, 256);

	time_t curt;
	time(&curt);
	tm curt_tm;
	localtime_r(&curt, &curt_tm);
	long time_now = curt_tm.tm_hour * 60 + curt_tm.tm_min;
	long wday = 1 << curt_tm.tm_wday;
	bool is_day = (wday & cd.m_invitation_day) > 0;
	bool is_time = time_now>=cd.m_invitation_time && time_now<=(cd.m_invitation_time+INVITE_INTERVAL);

	bool time_to_invite = is_day && is_time &&					//       
						  cd.m_invitation_parts.size()>0;		//  ,      
	if (!time_to_invite)
		return false;

	std::string invited_time;		GetConfInviteTime(cd.m_conf_id.c_str(), invited_time);

	bool is_invited = (!invited_time.empty())? !IsOutside(ft_now, invited_time.c_str()): false;

	return !is_invited || IsConf(cd.m_conf_id); //     
}

void VS_InvitesStorage::GetConfInviteTime(const char* conf_call_id, std::string &invited_time)
{
	if (!conf_call_id)
		return ;

	std::string key_name;
	key_name.reserve(128);
	key_name += MULTI_CONFERENCES_KEY;
	key_name += '\\';
	key_name += conf_call_id;
	VS_RegistryKey conf_key(false, key_name);

	if(!conf_key.IsValid())
		return ;

	conf_key.GetString(invited_time, MMC_INVITEDTIME);
}

void VS_InvitesStorage::ResetAutoInvites(const char *conf_id, const char *user_id)
{
	if (!conf_id || !*conf_id || !user_id || !*user_id)
		return;

	if (IsRegistryChanged()) ReadFromRegistry();
	std::lock_guard<std::mutex> lock(m_lock);

	VS_Map::Iterator it = m_confs.Begin();
	while (it != m_confs.End()) {
		ConferenceInvitation *cd = (ConferenceInvitation*)(*it).data;

		if (strcasecmp(cd->m_conf_id.c_str(), conf_id) == 0) {
			switch (cd->m_auto_invite)
			{
			case ConferenceInvitation::AutoInviteParam::no_auto_invite:
				break;
			case ConferenceInvitation::AutoInviteParam::by_conf_users_only: {
				VS_RealUserLogin user(user_id);
				if (cd->m_invitation_parts.find(user) != end(cd->m_invitation_parts)) {
					cd->m_force_invite = true;
				}
			} break;
			case ConferenceInvitation::AutoInviteParam::by_any_user: {
				cd->m_force_invite = true;
			}	break;
			default:
				break;
			}
		}

		++it;
	}
}

std::string VS_InvitesStorage::GetInvitedTime(const InvPartsMap::iterator &it)
{
	return it->second.first;
}

std::string VS_InvitesStorage::GetMissedConfEmailTime(const InvPartsMap::iterator &it)
{
	return it->second.second;
}

void VS_InvitesStorage::OnInviteUsersReply(VS_Container& cnt)
{
	auto stream_id = cnt.GetStrValueView(ID_PARAM);
	auto cid = cnt.GetStrValueView(CONFERENCE_PARAM);
	if (stream_id.empty() || cid.empty())
		return;
	std::string key_name;
	key_name.reserve(128);
	key_name += MULTI_CONFERENCES_KEY;
	key_name += '\\';
	key_name += cid;
	key_name += "\\Participants\\";
	cnt.Reset();
	while (cnt.Next())
	{
		string_view name = cnt.GetName();
		if (name == CALLID_PARAM)
		{
			auto user = cnt.GetStrValueView();
			if (user.empty())
				continue;
			VS_RealUserLogin r(user);
			VS_RegistryKey(false, key_name + ((r.IsOurSID())? r.GetUser().c_str(): user.c_str()), false).SetString(stream_id.c_str(), MMC_LASTINVITEDTOSTREAMID_TAG);
		}
	}
}

std::chrono::system_clock::time_point ConferenceInvitation::CalculateInvitationStartTime(const ConferenceInvitation &ci, const char * invitation_dateStr)
{
	if (!invitation_dateStr) return std::chrono::system_clock::time_point();
	std::tm tm = {};
	unsigned chars_parsed = 0;
	if (sscanf(invitation_dateStr, "%2d.%2d.%4d%n", &tm.tm_mday, &tm.tm_mon, &tm.tm_year, &chars_parsed) != 3 || chars_parsed != strlen(invitation_dateStr))
		return {}; // Parsing failed
	tm.tm_year -= 1900;
	tm.tm_mon -= 1;
	tm.tm_hour = ci.m_invitation_time / 60;
	tm.tm_min = ci.m_invitation_time % 60;
	tm.tm_isdst = -1;						// std::mktime will find out
	auto time = std::mktime(&tm);
	/*std::string stime = ctime(&time);
	std::cout << "Invitation time: " << stime << std::endl;*/
	return std::chrono::system_clock::from_time_t(time);	// std::mktime will convert localtime to utc
}
