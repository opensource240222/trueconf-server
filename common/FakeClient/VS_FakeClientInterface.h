#pragma once

#include "std/cpplib/VS_ClientCaps.h"
#include "std/cpplib/VS_Protocol.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/compat/map.h"

#include <boost/signals2/connection.hpp>
#include <boost/signals2/signal.hpp>

#include "std-generic/compat/map.h"
#include "std-generic/compat/memory.h"
#include "std-generic/compat/functional.h"
#include "std-generic/attributes.h"

enum VS_UserPresence_Status : int;
struct VS_ConferenceInfo;
class VS_FakeEndpoint;

class VS_FakeClientInterface
{
public:
	virtual ~VS_FakeClientInterface() { /*stub*/ };

	struct PartInfo{
		PartInfo() : role(PR_EQUAL), device_status(0), is_operator(false), onpodium(false) {}
		std::string	user_id;
		std::string dn;
		long		role;
		long		device_status;
		bool		is_operator;
		bool		onpodium;
	};
	struct VS_ConferenceDescriptor : vs::enable_shared_from_this<VS_ConferenceDescriptor>
	{
		std::string conf_id;
		std::string stream_conf_id;
		std::string topic;
		std::string peer;
		std::string peer_displayName;
		std::string owner;
		std::string owner_displayname;
		std::string my_id;
		std::string password;
		vs::map<std::string, PartInfo, vs::str_less> partList;

		enum ConferenceState
		{
			e_undefined = -1,
			e_conf_active = 0,
			e_conf_inviting,
			e_conf_wait_for_user_response,
			e_terminating,
			e_conf_creating
		};

		ConferenceState m_state;
		VS_Reject_Cause m_last_reject_reason;
		VS_Conference_Type conf_type;
		VS_GroupConf_SubType conf_subtype;
		VS_Participant_Role m_role;
		VS_ClientCaps remoteCaps;
		long m_lfltr;
		uint32_t	m_cmr_flags;
		bool is_incomming;

		bool IsGroup() const { return conf_type == CT_BROADCAST || conf_type == CT_MULTISTREAM; };
		bool IsPublic() const {	return IsGroup() && conf_subtype == GCST_ROLE; } // for now role conference is public

		bool Accept()
		{
			return m_client->Accept(shared_from_this());
		}

		bool Reject(VS_Reject_Cause reason)
		{
			return m_client->Reject(shared_from_this(), reason);
		}

		bool Hangup(bool forall = false)
		{
			return m_client->Hangup(shared_from_this(), forall);
		}

		VS_FakeClientInterface* m_client = nullptr;

	protected:
		VS_ConferenceDescriptor() 
		: m_state(e_undefined)
		, m_last_reject_reason(UNDEFINED_CAUSE)
		, conf_type(CT_UNDEFINED)
		, conf_subtype(GCST_UNDEF)
		, m_role(PR_EQUAL)
		, m_lfltr(0)
		, m_cmr_flags(~0)
		, is_incomming(false)
		{
		}

		// conf type, codecs, etc
	};

	struct ClientState
	{
		std::string trueconfId;
		std::string displayName;
		std::string sessionKey;
		VS_ClientCaps default_caps;
		std::shared_ptr<VS_ConferenceDescriptor> m_conf;

		std::string app_id;
		std::string app_name;
		std::string autologin_key;
		std::string used_for_login_name;
		std::string used_for_login_ip;

		vs::map<std::string, std::string, vs::str_less> app_properties;
		int32_t		rigths = 0;
	};

	virtual std::shared_ptr<VS_ConferenceDescriptor> GetCurrentConference() = 0;
	virtual ClientState GetClientState() = 0;
	virtual std::string GetTrueconfID()
	{
		return GetClientState().trueconfId;
	}

	virtual void LoginUserAsync(string_view login, string_view passwd,
		string_view passwdMD5, VS_ClientType clienttype, string_view ip, const std::vector<std::string> &aliases) = 0;
	virtual void Logout() = 0;

	virtual void SetDefaultCaps(const VS_ClientCaps &caps) = 0;
	virtual bool CreateConference(long maxPart, VS_Conference_Type confType, long subType, const char *name, const std::string &topic, const char *passwd, bool is_public = false, const std::string& transcReserveToken = "") = 0;
	virtual bool JoinAsync(const std::string &to, const VS_ConferenceInfo& info, const std::string& transcReserveToken) = 0;
	virtual bool InviteAsync(const std::string &to, bool force_create = false, const std::string& transcReserveToken = "") = 0;
	virtual void SendChatMessage(const std::string &to, const std::string &msg) = 0;
	virtual void SendChatMessage(const std::string& stream_conf_id, const std::string& to, const std::string& msg) = 0;
	virtual void SendChatCommand(const std::string &to, const std::string &msg, const VS_Container &cnt = {}) = 0;
	virtual void SendFECC(string_view to, eFeccRequestType type, const std::int32_t extraParam) = 0;
	virtual std::string GetAppProperty(const std::string& name) = 0;
	virtual void SetAppProperty(const std::string &name, const std::string &val) = 0;
	virtual void SendAppProperties() = 0;
	virtual const std::string& CID() const = 0;
	virtual void SetAlias(string_view toId) = 0;
	virtual void SetAppId(const char* appname) = 0;
	virtual void UpdateDisplayName(string_view displayName, bool updateImmediately) = 0;
	virtual void SendDeviceStatus(unsigned value) = 0;
	virtual bool ReqInviteAsync(string_view to) = 0;
	virtual bool KickFromConference(string_view to) = 0;
	virtual bool AnswerReqInvite(string_view to, bool accept) = 0;
	// role conf methods
	virtual bool QueryRole(const char *name, long role) = 0;
	virtual bool AnswerRole(const char *name, long role, long result) = 0;
	virtual bool ConnectSender(long fltr) = 0;
	// mixer layout control
	virtual bool ManageLayoutFunc(const char* func, const char* id1, const char* id2) = 0;
	virtual bool ManageLayoutPT(const long pt) = 0;
	// usage stat
	virtual bool SendUsageStat() = 0;

	virtual void DevicesList(string_view type, const std::vector<std::pair<std::string, std::string>> &list) = 0;
	virtual void DeviceChanged(string_view type, string_view id, string_view name) = 0;
	virtual void DeviceMute(string_view type, string_view id, bool mute) = 0;
	virtual void DeviceVolume(string_view type, string_view id, int32_t volume) = 0;

	// Called by FakeClientManager:
	virtual VS_FakeEndpoint& GetEndpoint() const = 0;

	typedef boost::signals2::signal<void(VS_ConferenceDescriptor &d)> JoinStreamConferenceSignalType;
	typedef boost::signals2::signal<void(VS_ConferenceDescriptor &d)> LeaveStreamConferenceSignalType;
	typedef boost::signals2::signal<void(VS_UserLoggedin_Result res)> LoginResponseSignalType;
	typedef boost::signals2::signal<void(VS_UserLoggedout_Cause cause)> LogoutResponseSignalType;
	typedef boost::signals2::signal<void(const char *method, const VS_ConferenceDescriptor &conf)> ConferenceStateChangeSignalType;
	typedef boost::signals2::signal<void(const char *from, const char *command)> CommandSignalType;
	typedef boost::signals2::signal<void(const char *to, const char *from, const char *dname, const char *text)> ChatSignalType;
	typedef boost::signals2::signal<void(const char *to, const char *from, const char *dname, const char *text, const char* fname, const char* link, const char* url, const char* about)> FileSignalType;
	typedef boost::signals2::signal<void(const VS_ConferenceDescriptor &conf)> PartListSignalType;
	typedef boost::signals2::signal<void(const VS_ConferenceDescriptor &conf, VS_RoleEvent_Type type, VS_Participant_Role role, VS_Broadcast_Status bs, VS_RoleInqury_Answer result, const char* user)> RoleEventSignalType;
	typedef boost::signals2::signal<void(void)> RequestKeyFrameSignalType;
	typedef boost::signals2::signal<void(const char *from, const char *to, eFeccRequestType type, long extra_param)> FECCSignalType;
	typedef boost::signals2::signal<void(const char *name, VS_UserPresence_Status status)> UpdateStatusSignalType;
	typedef boost::signals2::signal<void(const char *name)> ReqInviteSignalType;
	typedef boost::signals2::signal<void(string_view type, string_view id, string_view name)>ChangeDeviceType;
	typedef boost::signals2::signal<void(string_view type, string_view id, bool mute)>SetDeviceMuteType;

	JoinStreamConferenceSignalType			m_fireJoinStreamConference;
	LeaveStreamConferenceSignalType			m_fireLeaveStreamConference;
	LoginResponseSignalType					m_fireLoginResponse;
	LogoutResponseSignalType				m_fireLogoutResponse;
	ConferenceStateChangeSignalType			m_fireConferenceStateChange;
	CommandSignalType						m_fireCommand;
	ChatSignalType							m_fireChat;
	FileSignalType							m_fireFile;
	PartListSignalType						m_firePartList;
	RoleEventSignalType						m_fireRoleEvent;
	RequestKeyFrameSignalType				m_fireRequestKeyFrame;
	FECCSignalType							m_fireFECC;
	UpdateStatusSignalType					m_fireUpdateStatus;
	ReqInviteSignalType						m_fireReqInvite;
	ChangeDeviceType						m_fireChangeDevice;
	SetDeviceMuteType						m_fireSetDeviceMute;

protected:

	virtual bool Accept(const std::shared_ptr<VS_ConferenceDescriptor>& conf) = 0;
	virtual bool Reject(const std::shared_ptr<VS_ConferenceDescriptor>& conf, VS_Reject_Cause reason) = 0;
	virtual bool Hangup(const std::shared_ptr<VS_ConferenceDescriptor>& conf, bool forall) = 0;

	virtual void JoinStreamConference(VS_ConferenceDescriptor &d)
	{
		m_fireJoinStreamConference(d);
	};

	virtual void LeaveStreamConference(VS_ConferenceDescriptor &d)
	{
		m_fireLeaveStreamConference(d);
	};

	virtual void onLoginResponse(VS_UserLoggedin_Result res)
	{
		m_fireLoginResponse(res);
	}

	virtual void onLogoutResponse(VS_UserLoggedout_Cause cause)
	{
		m_fireLogoutResponse(cause);
	}

	virtual void onConferenceStateChange(const char *method, const VS_ConferenceDescriptor &conf)
	{
		m_fireConferenceStateChange(method, conf);
	}

	virtual void onCommand(const char *from, const char *command)
	{
		m_fireCommand(from, command);
	}

	virtual void onChat(const char *to, const char *conf, const char *from, const char *dname, const char *text)
	{
		const char* dst = conf ? conf : to;
		m_fireChat(dst, from, dname, text);
	}

	virtual void onFile(const char *to, const char *conf, const char *from, const char *dname, const char *text, const char* fname, const char* link, const char* url, const char* about)
	{
		const char* dst = conf ? conf : to;
		m_fireFile(dst, from, dname, text, fname, link, url, about);
	}

	virtual void onPartList(const VS_ConferenceDescriptor &conf, const char* /*user*/, int32_t /*type*/)
	{
		m_firePartList(conf);
	}

	virtual void onRoleEvent(const VS_ConferenceDescriptor &conf, VS_RoleEvent_Type type, VS_Participant_Role role, VS_Broadcast_Status bs, VS_RoleInqury_Answer result, const char* user)
	{
		m_fireRoleEvent(conf, type, role, bs, result, user);
	}

	virtual void onRequestKeyFrame()
	{
		m_fireRequestKeyFrame();
	}

	virtual void onFECC(const char *from, const char *to, eFeccRequestType type, long extra_param)
	{
		m_fireFECC(from, to, type, extra_param);
	}

	virtual void onUpdateStatus(const char *name, VS_UserPresence_Status status)
	{
		m_fireUpdateStatus(name, status);
	}

	virtual void onReqInvite(const char* from, const char *dn) {
		m_fireReqInvite(from);
	}

	virtual void onDeviceStatus(const char *name, long device_status) {};
	virtual void onListenersFltr(int32_t fltr) {};

	virtual void onChangeDevice(string_view type, string_view id, string_view name) {
		m_fireChangeDevice(type, id, name);
	}
	virtual void onSetDeviceMute(string_view type, string_view id, bool mute) {
		m_fireSetDeviceMute(type, id, mute);
	}
};