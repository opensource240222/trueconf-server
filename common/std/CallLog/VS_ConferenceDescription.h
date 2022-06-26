#ifndef VS_CONFERENCEDESCRIPTION_H
#define VS_CONFERENCEDESCRIPTION_H

#include "../cpplib/VS_ConferenceID.h"
#include "../cpplib/VS_UserID.h"
#include "../cpplib/VS_WideStr.h"
#include "../cpplib/VS_Protocol.h"
#include "std-generic/cpplib/StrCompare.h"

#include "std-generic/compat/map.h"
#include <set>
#include <chrono>

class VS_ConferenceDescription
{
public:
	enum ConferenceState {
		CONFERENCE_CREATED,
		PARTICIPANT_INVITED,
		PARTICIPANT_ACCEPTED,
		CONFERENCE_ENDED,
		CONFERENCE_HOLD
	};
	enum TerminationCause {
		UNKNOWN,
		TERMINATED_BY_STREAMS,			// stream rourer notification
		TERMINATED_BY_PART_LEAVE,		// user left the conf
		TERMINATED_BY_DELETE,			// client auto delete query
		TERMINATED_BY_REJECT,			// user reject
		TERMINATED_BY_EXPIR,			// lifetime expired
		TERMINATED_BY_SERVER_RESTART,	// Service Destroy
		TERMINATED_BY_PART_ACCDEN,		// in public conf host rigts to create was restricted
		TERMINATED_BY_CREATE_NEW,		// new conf begin while user(public host) yet in conf
		TERMINATED_BY_LOW_RESOURCES,	// NOT USED NOW
		TERMINATED_BY_HUNGUP,			// user hungup
		TERMINATED_BY_INVITE_REJECT,	// Invite Was not finihed successfully
		TERMINATED_BY_INVITE_NOT_FOUND,	// Invite user not found
		TERMINATED_BY_INVITE_OFFLINE,	// Invite user is offline
		TERMINATED_BY_INVITE_BUSY,		// Invite user is busy
		TERMINATED_BY_ADMIN				// Conference was terminated by admin
	};


	VS_ConferenceDescription(void);

	static void* Factory(const void* cd){return new VS_ConferenceDescription(*(VS_ConferenceDescription*)cd);}
	static void Destructor(void* cd) {			delete (VS_ConferenceDescription*) cd; 	}
	static void SplitConfID(const vs_conf_id& conf_id, int& conference, std::string& OUT_broker_id);
	static bool GetMultyConfInfo(const VS_SimpleStr& call_id, bool& OUT_isPublic, int32_t& OUT_guest_flags);

	bool IsValid( void ) const {return m_name.Length() != 0;}
	void SetTimeExp(long duration);	// duration in seconds
// Fields
	vs_conf_id		m_name;
	std::chrono::system_clock::time_point		m_timeExp;
	vs_user_id		m_owner;
	VS_SimpleStr	m_appID;

	vs_user_id		m_party;
	ConferenceState m_state;
	unsigned int	m_MaxParticipants;
	int				m_type;
	int				m_SubType;
	// Log inforation

	std::chrono::system_clock::time_point		m_logStarted;
	std::chrono::system_clock::time_point		m_logEnded;
	TerminationCause m_logCause;

	//MultiConference info
	VS_SimpleStr	m_call_id;
	unsigned int	m_MaxCast;
	bool			m_public;
	std::string		m_topic;
	VS_SimpleStr	m_lang;
	long			m_LstatusFlag;
	bool			m_isBroadcastEnabled;
	bool			m_need_record;
	bool			m_PlannedPartsOnly;

	// RTSP broadcast info
	std::string m_rtspEnabledCodecs;
	struct RTSPAnnounce
	{
		std::string url;
		std::string username;
		std::string password;
		std::string enabled_codecs;
		unsigned keepalive_timeout;
		unsigned retries;
		unsigned retry_delay;
		bool rtp_over_tcp;
		bool active;
		std::string reason;
	};
	vs::map<std::string, RTSPAnnounce, vs::str_less> m_rtspAnnounces;
	std::string m_rtspHelperProgram;


	//MultiCast info
	VS_SimpleStr	m_password;
	VS_SimpleStr	m_multicast_ip;

	// ssl stream key
	VS_SimpleStr	m_symKey;

	// svc mode
	unsigned int	m_svc_mode;

	// privacy type
	enum e_PrivacyType {
		e_PT_Private = 0,
		e_PT_Public = 1
	};
	e_PrivacyType m_PrivacyType;
	CMRFlags m_CMR_guest;
	CMRFlags m_CMR_user;

	std::map<vs_user_id, VS_Participant_Role>	m_last_roles;
	std::set<vs_user_id>						m_moderators;
	std::string	m_transceiverName;
};

#endif // VS_CONFERENCEDESCRIPTION_H
