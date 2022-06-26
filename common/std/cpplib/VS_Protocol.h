/**
 **************************************************************************
 * \file VS_Protocol.h
 * (c) 2002-2014 TrueConf
 * \brief Client-to-Server Protocol
 *
 * \b Project Standart Libraries
 * \author SMirnovK
 * \author StasS
 * \date 18.06.03
 *
 ****************************************************************************/
#ifndef VS_PROTOCOL_H
#define VS_PROTOCOL_H

/****************************************************************************
 * Defines
 ****************************************************************************/
/// initial protocol version
#define INIT_PROTOCOL_VERSION	2
#define SERVER_PROTOCOL_VERSION_STR	"440"

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VS_Errors.h"
#include "std-generic/cpplib/macro_utils.h"

#include <cstdint>

#define VCS_SERVER_VERSION				"362"
#define VCS_CLIENT_VERSION				"361"

#define ARM_LICENSE_VERSION				"440"

//@{ \name Services
//extern const char USERREGISTRATION_SRV[];
//extern const char ENDPOINTREGISTRATION_SRV[];
extern const char AUTH_SRV[];
extern const char ADDRESSBOOK_SRV[];
extern const char CONFIGURATION_SRV[];
extern const char CONFERENCE_SRV[];
extern const char CHAT_SRV[];
extern const char CHATV2_SRV[];
extern const char OFFLINECHAT_SRV[];
extern const char PING_SRV[];
extern const char CLIENT_SRV[];
extern const char PRESENCE_SRV[];
extern const char MANAGER_SRV[];
extern const char MULTICONF_SRV[];
extern const char LOG_SRV[];
extern const char RESOLVE_SRV[];
extern const char CHECKLIC_SRV[];
extern const char VERIFY_SRV[];
extern const char LOCATE_SRV[];
extern const char REGISTRATION_SRV[];
extern const char SERVERCONFIGURATOR_SRV[];
extern const char WEBRTC_PEERCONNECTION_SRV[];
extern const char CALL_CFG_UPDATER_SRV[];
extern const char DS_CONTROL_SRV[];

extern const char TRANSCODERSDISPATCHER_SRV[];
extern const char GATEWAY_SRV[];
extern const char TORRENT_SRV[];
extern const char SIPCALL_SRV[];

///Gateway
extern const char TRANSCODERINIT_METHOD[];
extern const char PREPARETRANSCODERFORCALL_METHOD[];
extern const char SETMEDIACHANNELS_METHOD[];
extern const char CLEARCALL_METHOD[];
//@}

//@{ \name Commom for all messages parameters
extern const char METHOD_PARAM[];
extern const char RESULT_PARAM[];
extern const char TYPE_PARAM[];
extern const char DISCONNECT_REASON_PARAM[];
extern const char GTYPE_PARAM[];
extern const char NAME_PARAM[];
extern const char TIME_PARAM[];
extern const char FILETIME_PARAM[];
extern const char DATA_PARAM[];
extern const char SUBTYPE_PARAM[];
extern const char SERVER_PARAM[];
extern const char SIGN_PARAM[];
extern const char TRANSPORT_SRCUSER_PARAM[];
extern const char TRANSPORT_SRCSERVER_PARAM[];
extern const char TRANSPORT_SRCSERVICE_PARAM[];
extern const char TRANSPORT_DSTUSER_PARAM[];
extern const char GID_PARAM[];
extern const char GNAME_PARAM[];
extern const char CMD_PARAM[];
extern const char ID_PARAM[];
extern const char EDITABLE_PARAM[];
extern const char VALID_UNTIL_PARAM[];
//@}

/// prefix for multiconference name
extern const char MUTIPREFIX_CONST[];

//@{ \name Endpoint Registartrion Service messages
//extern const char ENDPOINTIP_METHOD[];
//@}

//@{ \name Endpoint Registartrion Service messages parameters
extern const char CLIENTVERSION_PARAM[];
extern const char CLIENTTYPE_PARAM[];
extern const char PROTOCOLVERSION_PARAM[];
extern const char PROPERTY_PARAM[];
extern const char APPNAME_PARAM[];
//@}


/// User Registartrion messages

//@{ \name User attributes
extern const char LOGIN_PARAM[];
extern const char USERNAME_PARAM[];
extern const char FIRSTNAME_PARAM[];
extern const char LASTNAME_PARAM[];
extern const char EMAIL_PARAM[];
extern const char PASSWORD_PARAM[];
extern const char DISPLAYNAME_PARAM[];
extern const char CALLID_PARAM[];
extern const char CALLID2_PARAM[];
extern const char ALIAS_PARAM[];
extern const char REALID_PARAM[];
extern const char USERCOMPANY_PARAM[];
extern const char USERPHONE_PARAM[];
extern const char APPID_PARAM[];
extern const char PUBLIC_PARAM[];
extern const char TOPIC_PARAM[];
extern const char LANG_PARAM[];
extern const char TARIF_PARAM[];
extern const char TARIFNAME_PARAM[];
extern const char SESSION_PARAM[];
extern const char TARIFRESTR_PARAM[];
extern const char USER_DEFAULT_DOMAIN[];
extern const char EXTERNAL_ACCOUNT[];
extern const char REGID_PARAM[];
extern const char LOGINSTRATEGY_PARAM[];
extern const char APPLICATION_SETTINGS_PARAM[];
extern const char PROTOCOL_VERSION_PARAM[];
extern const char NEW_STATUS_PARAM[];
extern const char OLD_STATUS_PARAM[];
extern const char RELOAD_CONFIGURATION[];
extern const char RESERVATION_TOKEN[];


enum class eLoginStrategy
{
	LS_DEFAULT,
	LS_FROM_BSEVENT
};
//@}


//@{ \name Network Configuration Service massages
extern const char UPDATECONFIGURATION_METHOD[];
extern const char CONFIGURATIONUPDATED_METHOD[];
extern const char GETAPPPROPERTIES_METHOD[];
extern const char GETALLAPPPROPERTIES_METHOD[];
extern const char SETPROPERTIES_METHOD[];
extern const char SETALLAPPPROPERTIES_METHOD[];
extern const char REDIRECTTO_METHOD[];
/// for configurator
extern const char ONAPPPROPSCHANGE_METHOD[];
// for Amazon DDNS
extern const char EXTERNAL_IP_DETERMINED_METHOD[];
//@}

//@{ \name Enpoint attributes
extern const char IPCONFIG_PARAM[];
extern const char IP6CONFIG_PARAM[];
extern const char EPCONFIG_PARAM[];
extern const char HOST_PARAM[];
extern const char PORT_PARAM[];
extern const char MULTICAST_IP_PARAM[];
extern const char RESTRICTFLAGS_PARAM[];
extern const char IPADDRESSFAMILY_PARAM[];
//@}

//@{ \name User Login Service messages
extern const char LOGINUSER_METHOD[];
extern const char LOGINUSER2_METHOD[];
extern const char USERLOGGEDIN_METHOD[];
extern const char LOGOUTUSER_METHOD[];
extern const char USERLOGGEDOUT_METHOD[];
extern const char CHECKUSERLOGINSTATUS_METHOD[];
extern const char AUTHORIZE_METHOD[];
extern const char UPDATEACCOUNT_METHOD[];
extern const char REQUPDATEACCOUNT_METHOD[];
extern const char SETREGID_METHOD[];
extern const char UPDATE_PEERCFG_METHOD[];
extern const char SETUSERENDPAPPINFO_METHOD[];
enum VS_LoginAuth_Type
{
  LA_COMMON=0,
  LA_LDAP,
  LA_NTLM
};

/// for configurator
extern const char ONUSERCHANGE_METHOD[];
//@}

//@{ \name User Login Service messages parameters
extern const char CAUSE_PARAM[];
extern const char RIGHTS_PARAM[];
extern const char RATING_PARAM[];
extern const char SEPARATIONGROUP_PARAM[];
extern const char SEQUENCE_PARAM[];

extern const char AUTH_BY_ECP_METHOD[];
extern const char TICKET_ID_PARAM[];
extern const char TICKET_BODY_PARAM[];
extern const char SIGNED_TICKET_BODY_PARAM[];
//@}

/// User Login operation result
enum VS_UserLoggedin_Result : int32_t {
	USER_LOGGEDIN_OK,				// 0 - login successful, otherwise error code
	USER_ALREADY_LOGGEDIN,			// answer on CheckUserLoginStatus_Method, if current CID is already authorized at TransportRouter
	NO_USER_LOGGEDIN,				// answer on CheckUserLoginStatus_Method, if current CID is not authorized at TransportRouter - can try to login
	ACCESS_DENIED,					// incorrect password or other problems with DB
	SILENT_REJECT_LOGIN,			// client shouldn't show error to user (example: incorrect AutoLoginKey)
	LICENSE_USER_LIMIT,				// license restriction of online users reached, server cannot login you
	USER_DISABLED,					// user exist, but he is disabled to use this server
	RETRY_LOGIN,					// client should retry login after timeout (value in container or default), due to server busy or other server problems
	INVALID_CLIENT_TYPE,			// user cannot login using this client app (should use other type of client app)
	SESSION_WAIT,					// user waits email with link or code to confirm registration
	USER_PASSWORD_EXPIRED = 65213	// user password expired
};

/// User LogOut operation result
enum VS_UserLoggedout_Result {
	USER_LOGGEDOUT_OK,
	USER_ALREADY_LOGGEDOUT
};

/// Cause of user Logout from server
enum VS_UserLoggedout_Cause {
	USER_LOGGEDOUT_BY_REQUEST,
	USER_LOGGEDIN,
};

//@{ \name Conference Service messages
extern const char CREATECONFERENCE_METHOD[];
extern const char CONFERENCECREATED_METHOD[];
extern const char DELETECONFERENCE_METHOD[];
extern const char CONFERENCEDELETED_METHOD[];
extern const char USERACCESSIBLEON_METHOD[];
extern const char USERREGISTRATIONINFO_METHOD[];
extern const char INVITE_METHOD[];
extern const char ACCEPT_METHOD[];
extern const char REJECT_METHOD[];
extern const char HANGUP_METHOD[];
extern const char RESTART_METHOD[];
extern const char JOIN_METHOD[];
extern const char SENDPARTSLIST_METHOD[];
extern const char KICK_METHOD[];
extern const char IGNORE_METHOD[];
extern const char NOTIFYMONEYUPDATE_METHOD[];
extern const char CONNECTRECEIVER_METHOD[];
extern const char RECEIVERCONNECTED_METHOD[];
extern const char CONNECTSENDER_METHOD[];
extern const char CONNECTSERVICES_METHOD[];
extern const char SENDERCONNECTED_METHOD[];
extern const char PLACECALL_METHOD[];
extern const char REQINVITE_METHOD[];
extern const char INVITETOMULTI_METHOD[];
extern const char DELETEPARTICIPANT_METHOD[];
extern const char UPDATEPARTICIPANT_METHOD[];
extern const char LISTENERSFLTR_METHOD[];
extern const char ROLEEVENT_METHOD[];
extern const char DEVICESTATUS_METHOD[];
extern const char INVITEREPLY_METHOD[];
extern const char INVITEUSERS_METHOD[];
extern const char SETLAYERS_METHOD[];
extern const char CONNECTFAILURE_METHOD[];
extern const char UPDATESETTINGS_METHOD[];
extern const char MANAGELAYOUT_METHOD[];
extern const char MANAGECONEFRENCE_METHOD[];
extern const char CHANGEDEVICE_METHOD[];
extern const char SETDEVICESTATE_METHOD[];
extern const char DEVICESLIST_METHOD[];
extern const char DEVICECHANGED_METHOD[];
extern const char DEVICESTATE_METHOD[];
extern const char QUERYDEVICES_METHOD[];

extern const char SENDMAIL_METHOD[];

extern const char UPDATECONFERENCEPIC_METHOD[];
extern const char SETMYLSTATUS_METHOD[];
extern const char CLEARALLLSTATUSES_METHOD[];
extern const char SENDSLIDESTOUSER_METHOD[];

extern const char AUTOINVITE_METHOD[];
//@}

//@{ \name Conference Service messages parameters
extern const char OWNER_PARAM[];
extern const char DURATION_PARAM[];
extern const char MAXPARTISIPANTS_PARAM[];
extern const char MAXCAST_PARAM[];
extern const char DESTROYCONDITION_PARAM[];
extern const char CONFERENCE_PARAM[];
extern const char ENDPOINT_PARAM[];
extern const char BROKER_PARAM[];
extern const char PRODUCTID_PARAM[];
extern const char DIRECTCONNECT_PARAM[];
extern const char NEEDCONNECTINFO_PARAM[];
extern const char MEDIAFILTR_PARAM[];
extern const char SERVICES_PARAM[];
extern const char CLIENTCAPS_PARAM[];
extern const char RESTRICTBITRATE_PARAM[];
extern const char RESTRICTBITRATESVC_PARAM[];
extern const char ROLE_PARAM[];
extern const char PREVIEW_BODY_PARAM[];
extern const char PREVIEW_TYPE_PARAM[];
extern const char IS_OPERATOR_PARAM[];
extern const char DEVICESTATUS_PARAM[];
extern const char SCOPE_PARAM[];
extern const char PICTURE_PARAM[];
extern const char SYMKEY_PARAM[];
extern const char SVCMODE_PARAM[];
extern const char REQESTKEYFRAME_PARAM[];
extern const char SYSLOAD_PARAM[];
extern const char FRAMESIZEMBUSER_PARAM[];
extern const char LAYOUT_PARAM[];
extern const char SERVERNAME_PARAM[];
extern const char SERVERID_PARAM[];
extern const char LSTATUS_PARAM[];
extern const char MUSTJOIN_PARAM[];
extern const char UCALL_PARAM[];
extern const char PVIEW_PARAM[];
extern const char FUNC_PARAM[];
extern const char PERMISSIONS_PARAM[];
extern const char SOFT_KICK_PARAM[];
extern const char VOLUME_PARAM[];
extern const char MUTE_PARAM[];
extern const char ACTIVE_PARAM[];
extern const char START_TIME_PARAM[];
extern const char STOP_TIME_PARAM[];
//@}

/// Type of stream connection in conference
enum VS_DirectConnect_Type {
	NO_DIRECT_CONNECT,
	DIRECT_ACCEPT,
	DIRECT_CONNECT,
	DIRECT_SELF,
	DIRECT_UNKNOWN,
	DIRECT_NHP,
	DIRECT_UDP
};

/// Replay to Conference Create message
enum VS_ConferenceCreated_Result {
	NO_ENOUGH_RESOURCES_FOR_CONFERENCE,
	CONFERENCE_CREATED_OK,
	CREATE_ACCESS_DENIED,
	CREATE_HAVE_NO_MONEY
};

/// Conference Delete message parameter
enum VS_ConferenceDeleted_Result {
	CONFERENCE_DOESNT_EXIST,
	CONFERENCE_DELETED_OK
};

/// Cause of Reject message
enum VS_Reject_Cause {
	UNDEFINED_CAUSE = -1,
	REJECTED_BY_PARTICIPANT =0,
	CONFERENCE_IS_BUSY,
	PARTISIPANT_IS_BUSY,
	PARTISIPANT_NOT_AVAILABLE_NOW,
	INVALID_CONFERENCE,
	INVALID_PARTISIPANT,
	JOIN_OK,
	REACH_MONEY_LIMIT,
	REJECTED_BY_ACCESS_DENIED,
	REJECTED_BY_LOGOUT,
	REJECTED_BY_RESOURCE_LIMIT,
	REJECTED_BY_LOCAL_RES,
	CONFERENCE_PASSWORD_REQUIRED,
	REJECTED_BY_WRONG_PASSWORD,
	REJECTED_BY_NOFRIEND,
	REJECTED_BY_BADRATING,
	REJECTED_BY_TIMEOUT
  };

/// Multiconference receiver operation result
enum VS_StreamConnected_Result {
	SCR_UNKNOWN,
	SCR_FAILED,
	SCR_CONNECTED_OK,
	SCR_DISCONNECTED_OK,
	SCR_CHANGED_OK,
};

/// Type of stream connection in multiconference
enum VS_ReceiverConnect_Type {
	RCT_UNKNOWN,
	RCT_ROUTER,
	RCT_INTERCOM
};

/// Conference Type
enum VS_Conference_Type : int {
	CT_UNDEFINED = -1,
	CT_PRIVATE = 0,
	CT_PRIVATE_DIRECTLY,
	CT_PUBLIC,
	CT_HALF_PRIVATE,
	CT_BROADCAST,
	CT_MULTISTREAM,
	CT_INTERCOM, //multicast
	CT_VIPPUBLIC
};

/// GroupConference SubType
enum VS_GroupConf_SubType : int {
	GCST_UNDEF					= -1,
	GCST_FULL					= 0,
	GCST_ALL_TO_OWNER			= 1,
	GCST_PENDING_ALL_TO_OWNER	= 2,
	GCST_ROLE					= 3
};

/// Conference Scope
enum VS_Conference_Scope {
	GS_PERSONAL					= 0,
	GS_PUBLIC					= 1
};


///
enum VS_Participant_Role {
	PR_EQUAL,
	PR_COMMON,
	PR_LEADER,
	PR_PODIUM,
	PR_REPORTER,
	PR_REMARK
};

/// Role Event
enum VS_RoleEvent_Type {
	RET_INQUIRY,
	RET_ANSWER,
	RET_CONFIRM,
	RET_NOTIFY,
};

/// Role Inquiry answers
enum VS_RoleInqury_Answer {
	RIA_POSITIVE,
	RIA_COMMON,
	RIA_ROLE_BUSY,
	RIA_BY_PARTICIPANT,
	RIA_PATRICIPANT_BUSY,
	RIA_PARTICIPANT_ABSENT
};

/// Device statuses
enum VS_Device_Status {
	DVS_SND_UNKNOWN				= 0x0001,
	DVS_SND_NOTPRESENT			= 0x0002,
	DVS_SND_NOTCHOSEN			= 0x0004,
	DVS_SND_NOTWORK				= 0x0008,
	DVS_SND_PAUSED				= 0x0010,
	DVS_RCV_CONFRULE_NOTALOW	= 0x0100,
	DVS_RCV_DECODER_NOTPRESENT	= 0x0200,
	DVS_RCV_TURNEDOF_BYBAND		= 0x0400,
	DVS_RCV_PAUSED				= 0x0800,
	DVS_MASK_SND				= 0x00ff,
	DVS_MASK_RCV				= 0xff00
};

/// broadcast statuses
enum VS_Broadcast_Status {
	BS_SND_PAUSED				= 0x0001,
	BS_TRACK5_ON				= 0x0800,//Раньше использовался 0x100. Заменен для сохранения совместимости со старыми клиентами - теперь они не смогут получить сеанс SS в групповой конференции
	BS_OWNER_SS				    = 0x0200,
    BS_OWNER_DS				    = 0x0400
};

/// type of reply to invite
enum VS_InviteReply_Type {
	IRT_ACCEPT,
	IRT_REJECT,
	IRT_TIMEOUT,
};

enum VS_ParticipantListType {
	PLT_OLD = 0,
	PLT_ALL,
	PLT_ADD,
	PLT_DEL,
	PLT_UPD,
};

enum VS_ConfereneceLayoutType {
	CLT_UNKN = 0,
	CLT_AUTO,
	CLT_FLEXIBLE,
	CLT_FIXED,
};

enum class layout_for : uint8_t
{
	all,
	mixer,
	individual,
};

enum class DeviceChangingCause
{
	UNKNOWN,
	START,
	USER,
	AUTO,
	MODERATOR,
	SERVER
};

// Desktop Sharing Control Methods
extern const char VIDEOSOURCETYPE_METHOD[];
extern const char DSCONTROL_REQUEST_METHOD[];
extern const char DSCONTROL_RESPONSE_METHOD[];
extern const char DSCONTROL_FINISH_METHOD[];
extern const char DSCONTROL_FINISHED_METHOD[];
extern const char DSCOMMAND_METHOD[];

extern const char VIDEOTYPE_PARAM[];

enum VS_VideoSourceType {
	VST_UNKNOWN,
	VST_CAMERA,
	VST_DESKTOP,
	VST_APPLICATION,
	VST_IMAGE
};

enum VS_DSControlResult {
	DSCR_UNKNOWN,
	DSCR_ALLOW,
	DSCR_BADSRC,
	DSCR_ACCESS_DENIED,
	DSCR_TIMEOUT
};

//@{ \name Chat Service messages
extern const char SENDMESSAGE_METHOD[];
extern const char SENDCOMMAND_METHOD[];
//@}

//@{ \name Chat Service messages parameters
extern const char TO_PARAM[];
extern const char FROM_PARAM[];
extern const char MESSAGE_PARAM[];
extern const char MESSAGE_CONTAINER_PARAM[];
extern const char DB_MESSAGE_ID_PARAM[];
//@}

//@{ \name Chat commands subparameters
extern const char EXTRAVIDEOFLOW_NOTIFY[];
extern const char CONTENTAVAILABLE_PARAM[];
extern const char CURRENTVIDEO_PARAM[];
extern const char MAIN_VIDEO[];
extern const char CONTENT_VIDEO[];
extern const char SHOWCAMERA_REQ[];
extern const char SHOWCONTENT_REQ[];
extern const char CONTENTFORWARD_PUSH[];
extern const char CONTENTFORWARD_PULL[];
extern const char CONTENTFORWARD_STOP[];
//@}

// recording
extern const char CCMD_RECORD_QUERY[];
extern const char CCMD_RECORD_ACCEPT[];
extern const char CCMD_RECORD_REJECT[];
extern const char CCMD_RECORD_AGAIN[];
extern const char CCMD_RECORD_BUSY[];
extern const char CCMD_RECORDING_OFF[];
extern const char CCMD_RECORDING_ON[];

// old desktop sharing control
extern const char CCMD_DSC_REQUEST[];
extern const char CCMD_DSC_ACCEPT[];
extern const char CCMD_DSC_RELEASE[];
extern const char CCMD_DSC_BREAK[];
extern const char CCMD_DSC_DECLINE[];
extern const char CCMD_DSC_CONTROL[];

// conference events
extern const char CCMD_CONFEV_DATANOTAVALIBLE[];
extern const char CCMD_CONFEV_REMOVEDBY[];
extern const char CCMD_CONFEV_CONFCLOSED[];

// manage device
extern const char CCMD_CAM_ON[];
extern const char CCMD_CAM_OFF[];
extern const char CCMD_MIC_ON[];
extern const char CCMD_MIC_OFF[];

/// Chat Message type
enum VS_ChatMsgType
{
	MSG_NORMAL=0,
	MSG_SYSTEM=1
};


/// Ping Service message
extern const char PING_METHOD[];

/// Client Service message
extern const char TRYACCEPTDIRECT_METHOD[];

//@{\name User Presence Service messages
extern const char ADDTOADDRESSBOOK_METHOD[];
extern const char REMOVEFROMADDRESSBOOK_METHOD[];
extern const char SEARCHADDRESSBOOK_METHOD[];
extern const char GETUSERSTATUS_METHOD[];
extern const char UPDATEADDRESSBOOK_METHOD[];
extern const char ONADDRESSBOOKCHANGE_METHOD[];
extern const char MANAGEGROUPS_METHOD[];
//@}

//@{\name status passing messages
extern const char GETALLUSERSTATUS_METHOD[];
extern const char GETSUBSCRIPTIONS_METHOD[];
extern const char SUBSCRIBE_METHOD[];
extern const char UNSUBSCRIBE_METHOD[];
extern const char ADDPEER_METHOD[];
extern const char REMOVEPEER_METHOD[];

extern const char EXTERNALPRESENCESTARTED_METHOD[];

/// MBpsList Service message
extern const char MBPSLIST_METHOD[];

/// parameters
extern const char HOPS_PARAM[];
extern const char USERPRESSTATUS_PARAM[];
extern const char QUERY_PARAM[];
extern const char ADDRESSBOOK_PARAM[];
extern const char PUBCONFSTATUS_PARAM[];
extern const char SEQUENCE_ID_PARAM[];
extern const char HASH_PARAM[];
extern const char STATE_PARAM[];
extern const char LIFETIME_PARAM[];
extern const char LOCATED_CALLID_PARAM[];
extern const char LAST_DELETED_CALL_PARAM[];
//@}

/// User Ext Status
extern const char EXTSTATUS_PARAM[];
extern const char DELETE_EXT_STATUS_PARAM[];

///sticky
extern const char EXTSTATUS_NAME_EXT_STATUS[];
extern const char EXTSTATUS_NAME_DESCRIPTION[];
extern const char EXTSTATUS_NAME_LAST_ONLINE_TIME[];
extern const char EXTSTATUS_NAME_DEVICE_TYPE[];
extern const char EXTSTATUS_NAME_CAMERA[];
extern const char EXTSTATUS_NAME_IN_CLOUD[];
extern const char EXTSTATUS_NAME_FWD_TYPE[];
extern const char EXTSTATUS_NAME_FWD_CALL_ID[];
extern const char EXTSTATUS_NAME_FWD_TIMEOUT[];
extern const char EXTSTATUS_NAME_FWD_TIMEOUT_CALL_ID[];

extern const char ALLOWED_BY_SERVER_MAX_BW[];
extern const char ALLOWED_BY_SERVER_MAX_FPS[];
extern const char ALLOWED_BY_SERVER_MAX_WXH[];



////

////

/// Address Book types
enum VS_AddressBook
{
	AB_NONE,
	AB_COMMON,			 // general AB
	AB_BAN_LIST,
	AB_MISSED_CALLS,
	AB_RECEIVED_CALLS,
	AB_PLACED_CALLS,
	AB_PERSONS,			// search AB
	AB_PUBCONF,			// multiconf user list
	AB_PERSON_DETAILS,	// detailed user info
	AB_STATUS,			// chat/online
	AB_SINGLE_STATUS,	// only one user status
	AB_USER_PICTURE,	// user picture
	AB_CONF_LIST,		// list of available multiconferences
	AB_SP_GROUPS_,			// list of available groups
	AB_INVERSE,			// inverse adress book
	AB_GROUPS,			// groups in address book
	AB_LSTATUS,			// LStatuses (Lerya Statuses) #9803
	AB_EXTERNAL_CONTACTS,		// external contacts of my friends from mobile or social networks
	AB_MY_EXTERNAL_CONTACTS,	// my external contacts from mobile or social networks (#fb:123, #vk:321, +79261234567, #sip:user@sipnet.ru)
	AB_PHONES,					// Users telephones and custom phones (can edit from client)
	AB_PERSON_DETAILS_EXTRA,	// extra detailed info about user(conferendo: films, books, etc)
	AB_DIRECTORY=128	// internal server use, add AB up of it
};

/// Search Result
enum VS_Search_Result{
	SEARCH_FAILED=0,
	SEARCH_DONE=1,
	SEARCH_NOT_MODIFIED=2
};


/// Manager Service message
extern const char UPDATEINFO_METHOD[];
extern const char GETASOFMYDOMAIN_METHOD[];
//------------------------ Param --------------------------//
extern const char LOCATORBS_PARAM[];
extern const char RS_PARAM[];
extern const char ASDOMAIN_PARAM[];

//@{ \name Directory Service messages
extern const char RESOLVE_METHOD[];
extern const char RESOLVEALL_METHOD[];
extern const char UPDATESTATUS_METHOD[];

extern const char REGISTERSTATUS_METHOD[];
extern const char UNREGISTERSTATUS_METHOD[];
extern const char POINTCONNECTED_METHOD[];
extern const char POINTDISCONNECTED_METHOD[];
extern const char POINTDETERMINEDIP_METHOD[];
extern const char PUSHSTATUSDIRECTLY_METHOD[];
extern const char RESENDSTATUS_METHOD[];
extern const char SETLOCATORBS_METHOD[];
//@}

/// Directory Service messages parameter
extern	const char REPLICATE_PARAM[];


//@{ \name Log Service messages
extern const char LOGCONFSTART_METHOD[];
extern const char LOGCONFEND_METHOD[];
extern const char LOGPARTINVITE_METHOD[];
extern const char LOGPARTJOIN_METHOD[];
extern const char LOGPARTREJECT_METHOD[];
extern const char LOGPARTLEAVE_METHOD[];
extern const char LOGPARTSTAT_METHOD[];
extern const char LOGRECORDSTART_METHOD[];
extern const char LOGRECORDSTOP_METHOD[];
extern const char LOGPARTICIPANTDEVICE_METHOD[];
extern const char LOGPARTICIPANTROLE_METHOD[];
//@}

//@{ \name participant Invite Log Service messages parameters
extern const char PRICE_PARAM[];
extern const char CHARGE1_PARAM[];
extern const char CHARGE2_PARAM[];
extern const char CHARGE3_PARAM[];
//@}

//@{ \name participant leave Log Service messages parameters
extern const char BYTES_SENT_PARAM[];
extern const char BYTES_RECEIVED_PARAM[];
extern const char RECON_SND_PARAM[];
extern const char RECON_RCV_PARAM[];
extern const char JOIN_TIME_PARAM[];
extern const char CONF_BASE_STAT_PARAM[];
//@}

//@{ \name Global Registration Service messages
extern const char LOGEVENT_METHOD[];
extern const char LOGSTATS_METHOD[];
extern const char REGISTERSERVER_METHOD[];
extern const char REGISTERSERVEROFFLINE_METHOD[];
extern const char REGISTERSERVEROFFLINEFROMFILE_METHOD[];
extern const char UPDATELICENSE_METHOD[];
extern const char UPDATELICENSEOFFLINE_METHOD[];
extern const char REQUEST_LICENSE_METHOD[];
extern const char SHARE_LICENSE_METHOD[];
extern const char SHARED_LICENSE_CHECK_METHOD[];
extern const char SHARED_LICENSE_CHECK_METHOD_RESP[];
extern const char RETURN_SHARED_LICENSE_METHOD[];
extern const char RETURN_LICENSE_FORCE_TAG[];
extern const char MASTER_CHANGED_EVENT[];
extern const char RETURN_SHARED_LICENSE_METHOD_RESP[];
extern const char SERVERVERIFYFAILED_METHOD[];
extern const char MANAGESERVER_METHOD[];
extern const char CERTIFICATEUPDATE_METHOD[];
extern const char UPDATE_CALLCFGCORRECTOR_METHOD[];
//@}

//@{ \name Torrent Service messages
extern const char DOWNLOAD_METHOD[];
extern const char MANUALLY_CONNECT_METHOD[];
extern const char IS_FINISHED_METHOD[];
extern const char SHARE_METHOD[];
//@}

//@{ \name Torrent Service messages parameters
extern const char MAGNET_LINK_PARAM[];
extern const char INFO_HASH_PARAM[];
extern const char FILENAME_PARAM[];
extern const char FOLDERNAME_PARAM[];
extern const char IPPORT_PARAM[];
extern const char URL_PARAM[];
extern const char LINK_PARAM[];
extern const char ABOUT_PARAM[];

extern const char TRACKERS_PROP[];
//@}

///

enum VS_ManageServer_Commands
{
	MSC_NONE = 0,
	MSC_SETSTATE,
	MSC_REFRESH_PROPS,
	MSC_LOADBALANCE
};

enum VS_Server_States {
	SSTATE_NONE = 0,
	SSTATE_OFFLINE,
	SSTATE_RUNNING,
	SSTATE_RESTART,
	SSTATE_REDIRECT
};

//@{ \name Global Registration Service messages parameters
extern const char MEDIABROKERSTATS_PARAM[];
extern const char KEY_PARAM[];
extern const char FIELD1_PARAM[];
extern const char FIELD2_PARAM[];
extern const char FIELD3_PARAM[];
extern const char CERT_REQUEST_PARAM[];
extern const char CERTIFICATE_PARAM[];
extern const char CERTIFICATE_CHAIN_PARAM[];
extern const char SM_CERTIFICATE_PARAM[];
extern const char PARENT_CERT_PARAM[];
extern const char SCRIPT_PARAM[];
extern const char SERVER_VERSION_PARAM[];

//@}

extern const char INVITATIONUPDATE_METHOD[];

/// Server Actions
enum VS_BrokerEvents
{
	BE_UNKNOWN=0,
	BE_START,
	BE_CHANGEDIP,
	BE_REGISTER,
	BE_REGISTER_FAIL,
	BE_DISCONNECT,
	BE_SHUTDOWN,
	BE_LICENSE_RELOAD,
	BE_CONNECT,
	BE_STATESET,
	BE_PROPSREFRESHED,
	BE_HWINFO,
	BE_UPDATE_DDNS,
	BE_LICENSE_LIMIT,
	BE_OFFLINE_REGISTER
};

/// License Actions
enum VS_LicenseActions
{
	LIC_NOACTION=0,
	LIC_DELETE=1,
	LIC_ADD=2
};

enum VS_ServerTypes : int
{
	ST_UNKNOWN = 0,
	ST_SM,				// Server Manager
	ST_AS,				// Application Server
	ST_BS,				// Billing Server
	ST_RS,				// Routing Server
	ST_VCS,				//VCS
	ST_REGS,			// reg server
	ST_TMP				///временное имя, для регистрации

};

enum VS_ClientType : int32_t
{
	CT_SIMPLE_CLIENT	= 0,
	CT_GATEWAY			= 1,
	CT_MOBILE			= 2,
	CT_TRANSCODER		= 3,
	CT_WEB_CLIENT		= 4,
	CT_TERMINAL			= 5,
	CT_TRANSCODER_CLIENT= 6,
	CT_SDK				= 7
};

enum VS_ManageGroupCmd
{
	MGC_CREATE_GROUP = 0,
	MGC_DELETE_GROUP,
	MGC_RENAME_GROUP,
	MGC_ADD_USER,
	MGC_DELETE_USER
};

enum VS_ReqUpdateAccountType
{
	RUAT_NONE = 0,
	RUAT_SESSION,
	RUAT_RIGHTS,
	RUAT_PASSWORD,
	RUAT_PROFILE
};

extern const char REQUEST_CALL_ID_PARAM[];
extern const char REQUEST_EMAIL_PARAM[];
extern const char REQUEST_NAME_PARAM[];

//common constants

extern const char DEFAULT_DESTINATION_CALLID[];
extern const char DEFAULT_DESTINATION_CALLID_SIP[];
extern const char DEFAULT_DESTINATION_CALLID_H323[];
extern const char GROUPCONF_PREFIX[];
extern const char SPECIAL_CONF_PREFIX[];
extern const char EMPTY_CONFERENCE_TAG[];

// SUBTYPE_PARAM for SetRegID_Methood
enum VS_RegID_Register_Type
{
	REGID_INVALID = -1,
	REGID_REGISTER,
	REGID_UNREGISTER
};

enum eRoamingMode_t
{
	RM_INVALID,
	RM_DISABLED,
	RM_WHITELIST,
	RM_BLACKLIST,
};

// FECC (FarEnd Camera Control)

enum class eFeccState
{
	NOT_SUPPORTED, // no such function (ex: laptop built-in camera)
	ALLOWED,       // everyone can manage PTZ without request/permission
	ONDEMAND,      // client should ask for permission
	DENIED         // user does not want you to manage his PTZ
};

enum class eFeccRequestType
{
	NONE        = 0x00,

	GET_STATE	= 0x01,
	MY_STATE	= 0x02,
	SET_STATE	= 0x03,

	REQUEST_ACCESS		= 0x11,
	ALLOW_ACCESS		= 0x12,
	DENY_ACCESS			= 0x13,
	DENY_BY_TIMEOUT_ACCESS	= 0x14,

	UP			= 0x21,
	DOWN		= 0x22,
	LEFT		= 0x23,
	RIGHT		= 0x24,
	ZOOM_IN		= 0x25,
	ZOOM_OUT	= 0x26,
	FOCUS_IN	= 0x27,
	FOCUS_OUT	= 0x28,
	HOME		= 0x29,

	STOP        = 0x30,

	POSITION	= 0x41,

	USE_PRESET	= 0x51,
	SAVE_PRESET = 0x52
};

// FECC (FarEnd Camera Control)
extern const char FECC_METHOD[];
extern const char FECC_STATE_PARAM[];
extern const char PRESET_NUM_PARAM[];

enum class eGroupType : int
{
	SYSTEM_GROUP = 0,	// non editable by clients; can be created at WebManager
	CUSTOM_GROUP = 1,	// custom groups at users AB_GROUPS
	SYSCG				// reserved for PostgreSQL
};

enum class LoginType : int32_t {
	UNKNOWN,
	SIMPLE_LOGIN,
	AUTHORIZED_LOGIN
};

enum OnUserChangeType : int32_t
{
	e_invalid = -1,
	e_add_user = 0,
	e_change_user = 1,
	e_delete_user = 2,
	// = 3,
	e_update_users_groups = 4,
	e_remove_user_from_all_confs = 5,
	e_logout_user = 6
};


// Slideshow
extern const char SHOW_SLIDE_COMMAND[];
extern const char END_SLIDESHOW_COMMAND[];
extern const char IMG_TYPE_PARAM[];
extern const char WIDTH_PARAM[];
extern const char HEIGHT_PARAM[];
extern const char SIZE_PARAM[];
extern const char SLIDE_N_PARAM[];
extern const char SLIDE_COUNT_PARAM[];
extern const char FROM_RTP_PARAM[];

// DB logging
extern const char OBJECT_TYPE_PARAM[];
extern const char EVENT_TYPE_PARAM[];
extern const char OBJECT_NAME_PARAM[];
extern const char PAYLOAD_PARAM[];

extern const char SERVER_OBJECT_TYPE[];
extern const char USER_OBJECT_TYPE[];
extern const char LOGIN_USER_EVENT_TYPE[];
extern const char AUTHORIZE_USER_EVENT_TYPE[];
extern const char STATUS_USER_EVENT_TYPE[];
extern const char LOGOUT_USER_EVENT_TYPE[];
extern const char START_SERVER_EVENT_TYPE[];
extern const char STOP_SERVER_EVENT_TYPE[];
extern const char CONNECT_SERVER_EVENT_TYPE[];
extern const char DISCONNECT_SERVER_EVENT_TYPE[];


// ConferenceModuleRights
extern const char CMR_FLAGS_PARAM[];

enum class CMRFlags : uint32_t
{
	NONE		=   0x0,

	CHAT_SEND	= 0x0001,
	CHAT_RECV	= 0x0002,
	SS_SEND		= 0x0004,
	SS_RECV		= 0x0008,
	WB_SEND		= 0x0010,
	WB_RECV		= 0x0020,
	FT_SEND		= 0x0040,
	FT_RECV		= 0x0080,
	DS_ALLOWED	= 0x0100,
	REC_ALLOWED	= 0x0200,
	AUDIO_SEND	= 0x0400,
	AUDIO_RCV	= 0x0800,
	VIDEO_SEND	= 0x1000,
	VIDEO_RCV	= 0x2000,

	ALL			= 0xffffffff,
};
VS_ENUM_BITOPS(CMRFlags, uint32_t)

extern const char HANGUP_FLAGS_PARAM[];
enum class HangupFlags : uint32_t
{
	NONE = 0x0,

	FOR_ALL		= 0x1,		// owner finish gconf for all parts, or just for him only
	BY_USER		= 0x2,		// make sure that user pressed Hangup button at GUI (not automatically by streams, or other error)

	ALL = 0xffffffff,
};
VS_ENUM_BITOPS(HangupFlags, uint32_t)

static const uint16_t DEFAULT_BFCP_MINPORT = 3000;
static const uint16_t DEFAULT_BFCP_MAXPORT = 4000;

static const uint16_t DEFAULT_RTP_MINPORT = 6002;
static const uint16_t DEFAULT_RTP_MAXPORT = 8001;

static const uint16_t DEFAULT_H245_MINPORT = 3000;
static const uint16_t DEFAULT_H245_MAXPORT = 4000;

static const uint16_t DEFAULT_WEBRTC_MINPORT = 1024;
static const uint16_t DEFAULT_WEBRTC_MAXPORT = 65535;

// MultiLogin
extern const char MULTI_LOGIN_CAPABILITY_PARAM[];
enum class VS_MultiLoginCapability: uint8_t {
	UNKNOWN,		// tcs440: old server, that knows nothing about multi_login protocol changes in invites, confs; may work incorrectly: invite only one way, or not accepting the call
	SINGLE_USER,	// tcs450: have some hacks in code to be able to connect tcs500 with multi_login, but server itself does not support multi_login: no intsnace param for tc users, only for sip
	MULTI_USER,		// tcs500: can be many users with same login (instance format user@server.name/random)
};

#endif // VS_PROTOCOL_H
