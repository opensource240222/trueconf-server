#pragma once
#include "AppServer/Services/VS_PresenceService.h"
#include "std/cpplib/VS_Map.h"
#include "std/cpplib/VS_MapTpl.h"
#include "ldap_core/common/Common.h"
#include "std/cpplib/VS_ConferenceID.h"

#include <vector>
#include <string>
#include <mutex>
#include <cstdint>

typedef std::map<VS_RealUserLogin, std::pair<std::string, std::string>> InvPartsMap;

struct ConferenceInvitation
{
	std::string		m_conf_id;
	std::string		m_topic;
	std::string		m_password;
	std::string 	m_owner;

	int32_t			m_invitation_type;				// 1 - one-time conf; 0 - regular conf; -1 - not invitation
	std::chrono::system_clock::time_point		m_invitation_start_time;		// date and time, when to start a one-time conf

	int32_t			m_invitation_time;				// time in minutes ("-1" = not set; "0" = 00:00)
	int32_t			m_invitation_day;				// day of week ("0" = not set; "1" = Sunday; "2" = Monday)

	InvPartsMap		m_invitation_parts;
//	VS_StrIStrMap	m_invitation_parts;				// pairs of call_d and invited_time
	std::chrono::system_clock::time_point		m_invited_time;					// time, when last invited to conf (currently works in BS 3.1 only)

	int32_t			m_email_minutes;				// time before conf, when need to send email (currently works in BS 3.1 only)
	std::chrono::system_clock::time_point		m_email_sent_time;				// time, when last email was sent (currently works in BS 3.1 only)

	std::string		m_file_ics;
	int32_t			m_send_invitation;				// checkbox - if set, then immediately send email with file_ics
	int32_t			m_send_invitation_done;			// flag - if set, then email with file_ics was already sent

	enum class AutoInviteParam : int32_t
	{
		no_auto_invite		= 0,
		by_any_user			= 1,
		by_conf_users_only	= 2
	};

	AutoInviteParam	m_auto_invite;					// send invite to all the participants at conference start (bug 42215)
	mutable bool	m_force_invite;					// force send invites

	ConferenceInvitation(): m_invitation_type(0), m_invitation_time(-1), m_invitation_day(-1), m_email_minutes(0),
		m_send_invitation(0), m_send_invitation_done(0), m_auto_invite(AutoInviteParam::no_auto_invite), m_force_invite(false) {};

	bool IsInvitaion() const
	{
		bool days_ok = m_invitation_day>0 && m_invitation_day<(1 << 8);
		return days_ok && m_invitation_time>=0;
	}

	bool IsOneDayInvitation() const
	{
		return m_invitation_start_time != decltype(m_invitation_start_time)();
	}

	static void* Factory(const void* cd){return new ConferenceInvitation(*(ConferenceInvitation*)cd);}
	static void Destructor(void* cd) {			delete (ConferenceInvitation*) cd; 	}
	static std::chrono::system_clock::time_point CalculateInvitationStartTime(const ConferenceInvitation & ci, const char * invitation_dateStr);
};

class VS_InvitesStorage: public VS_PresenceServiceMember
{
	static const int INVITE_INTERVAL;

	VS_Map			m_confs;		// хранит конференции для инвайта
	uint32_t		m_last_update_time;
	mutable std::mutex		m_lock;
private:
	static bool IsOutside(const char* now, const char* to_check);			// false: if to_check in interval from now to now+INVITE_INTERVAL
	static bool IsInviteNeeded(const ConferenceInvitation& cd);
	static bool IsInviteNeeded_Regular(const ConferenceInvitation& cd);
	static bool IsInviteNeeded_Once(const ConferenceInvitation& cd);
	static void GetConfInviteTime(const char* conf_call_id, std::string &invited_time);
	static std::string GetInvitedTime(const InvPartsMap::iterator & it);
	static std::string GetMissedConfEmailTime(const InvPartsMap::iterator &it);

	bool IsRegistryChanged() const;
	void ReadFromRegistry();
	void FillParticipants(ConferenceInvitation* cd, BSEvent& ev, std::vector<std::string>& offline_users);
public:
	VS_InvitesStorage();
	virtual ~VS_InvitesStorage();

//	Init();			// прочитать с реестра в map

	void OnTimer_SendInvites(std::vector<BSEvent>& v);		// пройтись по всем conf и вызвать INVITETO для нужных
	void OnUserLoggedIn(const std::string& call_id, VS_Container& cnt);
	void ResetAutoInvites(const char *conf_id, const char *user_id);
	void OnInviteUsersReply(VS_Container& cnt);
};