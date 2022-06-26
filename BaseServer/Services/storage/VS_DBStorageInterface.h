#pragma once
#include "../../../common/tools/Watchdog/VS_Testable.h"
#include "../../../VCS/Services/VS_InvitesStorage.h"
#include "../../../common/std/cpplib/netutils.h"
#include "../../../common/std/CallLog/VS_DBStorage_CallLogInterface.h"
#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "transport/VS_ChatMsg.h"
#include "transport/typedefs.h"
#include <vector>
#include <memory>

static const char CONFERENCE[] = "conference";
static const char DB_OWNER[] = "owner";
static const char DB_DURATION[] = "duration";
static const char DB_ALLOW[] = "allow";
static const char DB_MAX_PARTICIPANTS[] = "max_participants";

static const char DB_CONFERENCE_PARAM[] = "@conference";
static const char DB_OWNER_PARAM[] = "@owner";
static const char CONF_PARTY_PARAM[] = "@party";
static const char CONF_TERM_PARAM[] = "@term_reason";
static const char DB_END_TIME_PARAM[] = "@end_time";
static const char DB_MAX_PARTICIPANTS_PARAM[] = "@max_participants";
static const char DB_MAX_CAST_PARAM[] = "@max_cast";
static const char DB_REMOTE_PARAM[] = "@remote";
static const char DB_MAX_USERS_PARAM[] = "@maxusers";
static const char DB_PUBLIC_PARAM[] = "@public";
static const char DB_TOPIC_PARAM[] = "@topic";
static const char DB_LANG_PARAM[] = "@lang";
static const char DB_CONFERENCE_PIC_PARAM[] = "@pic";
static const char DB_SCOPE_PARAM[] = "@scope";
static const char DB_IS_PUBLIC_PARAM[] = "@is_public";

static const char DB_TYPE_PARAM[] = "@type";
static const char DB_SUBTYPE_PARAM[] = "@subtype";
static const char DB_STATUS_PARAM[] = "@status";
static const char DB_TIME_PARAM[] = "@time";

static const char DB_SERVER[] = "server";
static const char DB_SERVER_PARAM[] = "@server";

static const char APP_ID_PARAM[] = "@app_id";
static const char USER_ID_PARAM[] = "@user_id";
static const char CALL_ID_PARAM[] = "@call_id";
static const char CALL_ID_PARAM_SP[] = "_call_id";
static const char CALL_ID2_PARAM[] = "@call_id2";
static const char NAME_PARAM_SP[] = "_name";
static const char VALUE_PARAM_SP[] = "_value";

static const char PART_PRICE[] = "price";
static const char PART_CHARGE1[] = "charge1";
static const char PART_CHARGE2[] = "charge2";
static const char PART_CHARGE3[] = "charge3";

static const char PART_JOIN_DB_TIME_PARAM[] = "@join_time";
static const char PART_LEAVE_DB_TIME_PARAM[] = "@leave_time";
static const char PART_LEAVE_REASON_PARAM[] = "@leave_reason";
static const char PART_BYTES_SENT_PARAM[] = "@bytes_sent";
static const char PART_BYTES_RECEIVED_PARAM[] = "@bytes_received";
static const char PART_RECON_SND_PARAM[] = "@reconnect_snd";
static const char PART_RECON_RCV_PARAM[] = "@reconnect_rcv";

static const char PART_PRICE_PARAM[] = "@price";
static const char PART_CHARGE1_PARAM[] = "@charge1";
static const char PART_CHARGE2_PARAM[] = "@charge2";
static const char PART_CHARGE3_PARAM[] = "@charge3";




class VS_TransportRouterService;
class VS_DBObjects;
class VS_ExtendedStatusStorage;

class VS_VCSDBStorageInterface
{
public:
	VS_VCSDBStorageInterface(){}
	virtual ~VS_VCSDBStorageInterface(){}

	enum SrvPropertyType
	{
		PROP_DB				=1,
		PROP_SERVER		=2,
		PROP_GLOBALAPP=3
	};

	virtual bool OnPropertiesChange(const char* /*pass*/)
	{ return false; }
	virtual int  GetServerProperties(VS_Container& /*prop*/,SrvPropertyType /*type*/)
	{return 0;}
	virtual bool GetWebManagerProperty(const VS_SimpleStr& /*prop_name*/, std::string& /*value*/)
	{return 0;}
	virtual bool GetWebManagerProperty(const VS_SimpleStr& /*prop_name*/, unsigned long& /*value*/)
	{return 0;}
	virtual bool DeleteUser(const vs_user_id& /*id*/)
	{ return false; }
	virtual bool IsAutoAuthAvailable()
	{return false;} // implemented in LDAP version
	virtual bool Authorize(const VS_SimpleStr& /*ep_id*/, VS_Container* /*in_cnt*/, VS_Container& /*out_cnt*/, bool& /*request*/, VS_UserData& /*ud*/, const VS_ClientType &/*client_type*/ = CT_SIMPLE_CLIENT)
	{return false;} // implemented in LDAP version
	virtual void UpdateAliasList()
	{ }
	virtual bool OnUserChange(const char* /*user*/, long /*type*/, const char* /*pass*/)
	{ return false; }
	virtual void UpdateUsersGroups(const std::function<bool(void)>& /*is_stopping*/)
	{ }
	virtual void Timer(unsigned long /*ticks*/, VS_TransportRouterServiceHelper* /*caller*/)		// For LDAP
	{ }
	virtual bool IsDisabledUser(const char* /*user*/)
	{ return false; }
	virtual void ProcessConfStat(VS_Container& /*cnt*/)
	{	}
	virtual bool CheckSessionID(const char* /*password*/)
	{
		return false;
	}
	virtual const char* GetSessionID() const
	{
		return 0;
	}

	virtual void GetRoamingOfflineMessages(const char* /*our_sid*/, VS_ChatMsgs& /*v*/)
	{	}
	virtual void DeleteOfflineChatMessage(VS_Container& /*cnt*/)
	{	}
	virtual bool IsLDAP() const
	{return false;}
	virtual bool CheckDigestByRegistry(const VS_SimpleStr& /*login*/, const VS_SimpleStr& /*password*/)
	{return false;}

	virtual boost::signals2::connection Connect_AliasesChanged(const AliasesChangedSlot &/*slot*/)
	{
		boost::signals2::connection tmp;
		return tmp;
	}
#ifdef _SVKS_M_BUILD_
	virtual bool AuthByECP(VS_Container& cnt, VS_Container& rCnt, VS_UserData& ud, VS_SimpleStr& autoKey, VS_Container& prop_cnt, VS_ClientType client_type)
	{
		return false;
	}
#endif
	virtual std::string VCS_ResolveHomeServer(const VS_RealUserLogin& query_r);

	virtual bool IsOperator(const vs_user_id& /*user*/)
	{
		return false;
	}
};

class VS_DBStorageInterface : public VS_Testable,
							  public VS_VCSDBStorageInterface,
							  public VS_DBStorage_CallLogInterface
{
public:
	VS_SimpleStr	m_serverID;
	int error_code;
	VS_DBStorageInterface() :error_code(0)
	{
	}

	virtual bool Init(const VS_SimpleStr& broker_name) = 0;
	virtual void CleanUp(){ }

	//users
	virtual bool FindUser(const vs_user_id& /*id*/, VS_StorageUserData& /*user*/)
	{ return false; }
	virtual bool FindUserByAlias(const std::string& /*alias*/, VS_StorageUserData& /*user*/)
	{ return false; }
	virtual bool FindUser(const vs_user_id& id, VS_UserData& user, bool find_by_call_id_only = true) = 0;
	virtual void SetUserStatus(const VS_SimpleStr& call_id,int status, const VS_ExtendedStatusStorage &extStatus, bool set_server,const VS_SimpleStr& server) = 0;
	virtual bool GetExtendedStatus(const char* call_id, VS_ExtendedStatusStorage &sticky) = 0;
	virtual void ClearStatuses(const VS_SimpleStr& /*server*/){ }
	virtual int	LoginUser(const VS_SimpleStr& login,const VS_SimpleStr& password, const VS_SimpleStr& appID, VS_SimpleStr& autoKey, const VS_SimpleStr& appServer, VS_UserData& user, VS_Container& prop_cnt, const VS_ClientType &client_type =CT_SIMPLE_CLIENT) = 0;
	virtual bool LogoutUser(const VS_SimpleStr& /*login*/)
	{	return false;	}

	//address books
	virtual int	FindUsers(VS_Container& cnt, int& entries, VS_AddressBook ab, const vs_user_id& owner, const std::string& query, long client_hash, VS_Container& in_cnt) = 0;
	virtual int	AddToAddressBook(VS_AddressBook ab,const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt, VS_SimpleStr& add_call_id, std::string& add_display_name, VS_TransportRouterServiceHelper* srv=0 ) = 0;
	virtual int	RemoveFromAddressBook(VS_AddressBook ab,const vs_user_id& user_id1,const vs_user_id& user_id2, VS_Container &cnt, long& hash, VS_Container &rCnt) = 0;
	virtual int	UpdateAddressBook(VS_AddressBook ab, const vs_user_id& user_id1, const char* call_id2, VS_Container& cnt, long& hash, VS_Container &rCnt) = 0;

	//properties:
	//applications
	virtual int GetAppProperties(VS_Container& prop,const VS_SimpleStr& app_name) = 0;
	virtual int GetAllAppProperties(VS_AppPropertiesMap &prop_map) = 0;

    virtual bool SetAppProperty(const VS_SimpleStr& /*prop_name*/, const VS_SimpleStr &/*value*/) { return false; }

	virtual bool GetAppProperty	(const VS_SimpleStr& app_name,const VS_SimpleStr& prop_name,VS_SimpleStr& value) = 0;
	virtual bool GetAppProperty	(const VS_SimpleStr& app_name,const VS_SimpleStr& prop_name,VS_WideStr& value) = 0;
	virtual bool GetServerProperty(const std::string& prop_name, std::string &value) = 0;
	virtual bool SetServerProperty(const VS_SimpleStr& name,const VS_SimpleStr& value) = 0;

	virtual bool GetServerTime(std::chrono::system_clock::time_point& ft) = 0;
	//endpoint
	virtual bool SetEndpointProperty(const char* ep_id, const char* name, const char* value) = 0;
	virtual bool SetEndpointProperty(const char* ep_id, const char* name, const wchar_t* value) = 0;
	virtual bool SetAllEpProperties(const char* app_id, const int prot_version, const short int type, const wchar_t* version,
									const /*char*/wchar_t *app_name, const /*char*/wchar_t* sys_conf, const /*char*/wchar_t* processor, const /*char*/ wchar_t *directX,
									const /*char*/wchar_t* hardwareConfig, const /*char*/wchar_t* AudioCapture, const /*char*/wchar_t *VideoCapture, const /*char**/wchar_t* AudioRender, const char* call_id) = 0;
	virtual bool GetEndpointProperty(const VS_SimpleStr& ep_id,const VS_SimpleStr& _name,VS_SimpleStr& value) = 0;
	//user
	virtual bool GetUserProperty(const vs_user_id& user_id,const std::string& name, std::string& value) = 0;
	virtual int  SaveTempLoginData(const char* /*login*/, const char* /*app_id*/, VS_Container &/*data*/) { return SEARCH_FAILED; };

	//logging, moved to VS_DBStorage_CallLogInterface
	//virtual bool LogConferenceStart(const VS_ConferenceDescription& conf,bool remote=false) = 0;
	//virtual bool LogConferenceEnd  (const VS_ConferenceDescription& conf) = 0;
	//virtual bool LogParticipantInvite(const vs_conf_id& conf_id,const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,
	//								  const VS_FileTime& time=NULL,VS_ParticipantDescription::Type type=VS_ParticipantDescription::PRIVATE_HOST) = 0;
	//virtual bool LogParticipantJoin (const VS_ParticipantDescription& pd, const VS_SimpleStr& callid2=NULL) = 0;
	//virtual bool LogParticipantLeave (const VS_ParticipantDescription& pd) = 0;
	//virtual bool LogParticipantStatistics (const VS_ParticipantDescription& pd) = 0;
	//virtual long SetRegID(const char* /*call_id*/, const char* /*reg_id*/, VS_RegID_Register_Type /*reg_type*/) { return -1; }
	virtual bool LogSlideShowCmd(const char *confId, const char *from, const char *url, const char *mimeType, size_t slideIndex, size_t slidesCount, const char *about,
								 size_t width, size_t height, size_t size) = 0;
	virtual bool LogSlideShowEnd(const char *confId, const char *from) = 0;
	virtual bool LogGroupChat(const char *confId, const char *from, const char *text) = 0;

	virtual bool UpdateConferencePic(VS_Container &cnt) = 0;

	virtual bool GetMissedCallMailTemplate(const std::chrono::system_clock::time_point missed_call_time, const char* fromId, std::string& inOutFromDn, const char * toId, std::string& inOutToDn, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string& subj_templ, std::string &body_templ) = 0;
	virtual bool GetInviteCallMailTemplate(const std::chrono::system_clock::time_point missed_call_time, const char *fromId, std::string& inOutFromDn, const char * toId, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ) = 0;
	virtual bool GetMissedNamedConfMailTemplate(const char* fromId, std::string& inOutFromDn, const char * toId, std::string& inOutToDn, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ) = 0;

	virtual int GetOfflineChatMessages(const char* call_id, std::vector<VS_Container> &vec) = 0;
	virtual bool SetOfflineChatMessage(const VS_SimpleStr& from_call_id, const VS_SimpleStr& to_call_id, const VS_SimpleStr& body_utf8, const std::string& from_dn, const VS_Container &cont) = 0;

	virtual void GetBSEvents(std::vector<BSEvent> &vec) = 0;
	virtual void GetUpdatedExtStatuses(std::map<std::string, VS_ExtendedStatusStorage>&) = 0;

	//named confs
	virtual int GetNamedConfInfo(const char* /*conf_id*/, VS_ConferenceDescription& /*cd*/, ConferenceInvitation& /*ci*/, VS_SimpleStr& /*as_server*/, long& /*scope*/)
	{ return VSS_CONF_NOT_FOUND; }
	virtual int UpdateNamedConfInfo_RTSPAnnounce(const VS_ConferenceDescription& /*cd*/, string_view /*announce_id*/ = {})
	{ return VSS_CONF_NOT_FOUND; }
	virtual void SetNamedConfServer(const char* /*named_conf_id*/, const char* /*stream_conf_id*/)
	{ return; }
	virtual int GetNamedConfParticipants(const char* /*conf_id*/, ConferenceInvitation& /*ci*/)
	{ return VSS_CONF_NOT_FOUND; }
	virtual int InitNamedConfInvitaions(std::vector<std::string> &/*v*/)
	{ return 0; }

	virtual VS_SimpleStr GetLoginSessionSecret()
	{	return ""; }

	virtual bool CheckDBConnection()
	{	return true;	}

	virtual bool ManageGroups_CreateGroup(const vs_user_id& owner, const VS_WideStr& gname, long& gid, long& hash) = 0;
	virtual bool ManageGroups_DeleteGroup(const vs_user_id& owner, const long gid, long& hash) = 0;
	virtual bool ManageGroups_RenameGroup(const vs_user_id& owner, const long gid, const VS_WideStr& gname, long& hash) = 0;
	virtual bool ManageGroups_AddUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash) = 0;
	virtual bool ManageGroups_DeleteUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash) = 0;
	virtual vs_conf_id NewConfID() { return vs_conf_id(); };

	virtual long ChangePassword(const char* /*call_id*/, const char* /*old_pass*/, const char* /*new_pass*/, const VS_SimpleStr& /*from_app_id*/)
	{	return -1;	}
	virtual long UpdatePerson(const char* /*call_id*/, VS_Container& /*cnt*/)
	{	return -1;	}
	virtual long NotifyWeb_MissedCall(VS_Container& /*cnt*/)
	{	return -1;	}
	virtual void GetSipProviderByCallId(const char* /*call_id*/, std::vector<VS_ExternalAccount>& /*external_accounts*/, VS_DBObjects* /*dbo*/=0)
	{				}
#ifdef _DEBUG
	virtual void SetAuthThreadID() {}
#endif

	static bool IsValidUserSID(string_view call_id)
	{
		if (!netutils::IsValidDomainName(VS_RealUserLogin(call_id).GetDomain().data(), true, false))	// here it is safe to call sv.data(), because original strings call_id and r_login are null-terminated
		{
			return false;
		}

		return true;
	}
};

extern std::shared_ptr<VS_DBStorageInterface>	g_dbStorage;