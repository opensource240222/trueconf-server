/**
 **************************************************************************
 * \file VS_Protocol.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Visicron Client-to-Server Protocol. Based on text messages
 * description
 *
 * \b Project Standart Libraries
 * \author SMirnovK
 * \author StasS
 * \date 18.06.03
 *
 * $Revision: 73 $
 *
 * $History: VS_Protocol.cpp $
 *
 * *****************  Version 73  *****************
 * User: Mushakov     Date: 17.07.12   Time: 23:09
 * Updated in $/VSNA/std/cpplib
 *  - LoginConfigurator() was removed
 * - messages from configurator are handled by SessionID
 * - fix TransportMessage::IsFromServer()
 *
 * *****************  Version 72  *****************
 * User: Ktrushnikov  Date: 17.05.12   Time: 14:58
 * Updated in $/VSNA/std/cpplib
 * VS_VCSConfigurationService::OnPointDeterminedIP_Event:
 * - processing modex to Service thread from TransportRouter thread
 * (because, g_dbStorage->FindUser() can take long time at LDAP)
 *
 * *****************  Version 71  *****************
 * User: Mushakov     Date: 4.05.12    Time: 20:33
 * Updated in $/VSNA/std/cpplib
 *  - onConnect (err) handled in transcoder
 *  - refactoring subscribtion (VS_ExternalPresenceInterface added)
 *
 * *****************  Version 70  *****************
 * User: Mushakov     Date: 22.04.12   Time: 16:13
 * Updated in $/VSNA/std/cpplib
 *  - Changing statuses by gateway start/stop was supported
 *
 * *****************  Version 69  *****************
 * User: Ktrushnikov  Date: 18.04.12   Time: 11:54
 * Updated in $/VSNA/std/cpplib
 * #11610
 * - constants to protocol
 * - USERPRESSTATUS at other type of searches
 *
 * *****************  Version 68  *****************
 * User: Mushakov     Date: 28.03.12   Time: 20:27
 * Updated in $/VSNA/std/cpplib
 *  - resolve and subscribe terminals
 *
 * *****************  Version 67  *****************
 * User: Sanufriev    Date: 16.02.12   Time: 19:06
 * Updated in $/VSNA/std/cpplib
 * - key frame request for SVC server
 *
 * *****************  Version 66  *****************
 * User: Sanufriev    Date: 10.02.12   Time: 10:13
 * Updated in $/VSNA/std/cpplib
 * - update svc server implementation
 * - add svc server capability
 *
 * *****************  Version 65  *****************
 * User: Ktrushnikov  Date: 11/10/11   Time: 6:56p
 * Updated in $/VSNA/std/cpplib
 * - GTYPE_PARAM added
 *
 * *****************  Version 64  *****************
 * User: Ktrushnikov  Date: 26.10.11   Time: 14:18
 * Updated in $/VSNA/std/cpplib
 * NamedConf
 * - set MaxCast for NamedConf with value from DB (edit server property
 * named_conf_max_cast)
 *
 * *****************  Version 63  *****************
 * User: Mushakov     Date: 10.10.11   Time: 20:06
 * Updated in $/VSNA/std/cpplib
 *
 * *****************  Version 62  *****************
 * User: Ktrushnikov  Date: 19.09.11   Time: 21:18
 * Updated in $/VSNA/std/cpplib
 * #9802,#9803
 * - lerya statuses
 *
 * *****************  Version 61  *****************
 * User: Mushakov     Date: 14.09.11   Time: 14:43
 * Updated in $/VSNA/std/cpplib
 *
 * *****************  Version 60  *****************
 * User: Mushakov     Date: 12.09.11   Time: 16:36
 * Updated in $/VSNA/std/cpplib
 * - GW
 *
 * *****************  Version 59  *****************
 * User: Ktrushnikov  Date: 10.08.11   Time: 12:49
 * Updated in $/VSNA/std/cpplib
 * - GetServerOfUser() removed
 * - cleanup
 *
 * *****************  Version 58  *****************
 * User: Ktrushnikov  Date: 15.07.11   Time: 15:23
 * Updated in $/VSNA/std/cpplib
 * #8535, #8917
 * - user groups supported in Visicron.dll and BS server
 * (VS_AddressBookService, VS_DBStorage)
 *
 * *****************  Version 57  *****************
 * User: Mushakov     Date: 6.05.11    Time: 20:43
 * Updated in $/VSNA/std/cpplib
 *  - new reg; new reg cert; cert chain supported in tc_server
 *
 * *****************  Version 56  *****************
 * User: Ktrushnikov  Date: 22.03.11   Time: 12:58
 * Updated in $/VSNA/std/cpplib
 * - defines for constant strings
 *
 * *****************  Version 55  *****************
 * User: Ktrushnikov  Date: 9.03.11    Time: 18:30
 * Updated in $/VSNA/std/cpplib
 * added new service OFFLINECHAT_SRV in VCS & BS
 *
 * *****************  Version 54  *****************
 * User: Ktrushnikov  Date: 13.11.10   Time: 14:50
 * Updated in $/VSNA/std/cpplib
 * - sp_get_properties_list: appName param added
 * - pass appName from client to AS->BS->DB
 *
 * *****************  Version 53  *****************
 * User: Mushakov     Date: 3.11.10    Time: 20:46
 * Updated in $/VSNA/std/cpplib
 * - VCS manager authorization added
 * - VCS Config dead lock fixed
 *
 * *****************  Version 52  *****************
 * User: Mushakov     Date: 1.11.10    Time: 21:00
 * Updated in $/VSNA/std/cpplib
 * - cert update added
 * - registration from server added
 * - authorization servcer added
 *
 * *****************  Version 51  *****************
 * User: Mushakov     Date: 15.10.10   Time: 16:27
 * Updated in $/VSNA/std/cpplib
 * - defaultDomain added
 *
 * *****************  Version 50  *****************
 * User: Mushakov     Date: 10.09.10   Time: 20:24
 * Updated in $/VSNA/std/cpplib
 * - Registration on SM added
 *
 * *****************  Version 49  *****************
 * User: Ktrushnikov  Date: 10.08.10   Time: 21:29
 * Updated in $/VSNA/std/cpplib
 * [#7561]: GenerateSessionKey() at LoginUser and at ReqUpdateAccount()
 * [#7562]: update login session key at timer timeout in Visicron.dll
 *
 * *****************  Version 48  *****************
 * User: Smirnov      Date: 28.07.10   Time: 16:32
 * Updated in $/VSNA/std/cpplib
 * - tarif restrictions
 *
 * *****************  Version 47  *****************
 * User: Smirnov      Date: 27.07.10   Time: 21:24
 * Updated in $/VSNA/std/cpplib
 * enh #7558
 *
 * *****************  Version 46  *****************
 * User: Ktrushnikov  Date: 13.07.10   Time: 22:02
 * Updated in $/VSNA/std/cpplib
 * Arch 3.1: GroupConfLimits
 * - tarif flags for conf & user added
 * - processing tarif restrictions at server
 * - set rights by  tarif at login
 * - support tarif for namedconfs
 *
 * *****************  Version 45  *****************
 * User: Ktrushnikov  Date: 29.06.10   Time: 12:21
 * Updated in $/VSNA/std/cpplib
 * Arch 3.1: NamedConfs invitation:
 * - RESOLVEALL_METHOD to get server & status of users
 * - start resolve in separate thread (PoolThreads)
 * - ask SM for RS in BS
 * - access to RS name from all BS services via locks
 * - reconnect to RS
 * - calc is_time fixed
 *
 * *****************  Version 44  *****************
 * User: Smirnov      Date: 28.04.10   Time: 17:49
 * Updated in $/VSNA/std/cpplib
 * - bugfix#7335
 *
 * *****************  Version 43  *****************
 * User: Ktrushnikov  Date: 23.03.10   Time: 14:08
 * Updated in $/VSNA/std/cpplib
 * - CountOf Mobile, Guest, GW fixed (called twice for one user)
 * - Pass ClientType as CLIENTTYPE_PARAM (not as TYPE_PARAM)
 * - Pass ClientType for Authorize_Method (LDAP)
 *
 * *****************  Version 42  *****************
 * User: Ktrushnikov  Date: 7.03.10    Time: 17:27
 * Updated in $/VSNA/std/cpplib
 * - All storages are inherited from VS_DBStorageInterface (DB, Reg, LDAP,
 * Trial)
 * - g_vcs_storage changed to g_dbStorage
 * - TConferenceStatistics added
 * - Process of LogPartStat added for VCS(file) & BS(null)
 * - fixed with d78 TransportRoute::DeleteService (dont delete deleted
 * service)
 * BS::LogSrv: suppress_missed_call_mail property added
 *
 * *****************  Version 41  *****************
 * User: Ktrushnikov  Date: 13.01.10   Time: 21:43
 * Updated in $/VSNA/std/cpplib
 * SS & DS support from old arch:
 * - ConnectServices added
 *
 * *****************  Version 40  *****************
 * User: Mushakov     Date: 28.10.09   Time: 17:59
 * Updated in $/VSNA/std/cpplib
 * - vs_ep_id removed
 * - registration corrected
 *
 * *****************  Version 39  *****************
 * User: Mushakov     Date: 26.10.09   Time: 20:32
 * Updated in $/VSNA/std/cpplib
 *  - sign verification added
 *
 * *****************  Version 38  *****************
 * User: Mushakov     Date: 23.10.09   Time: 15:05
 * Updated in $/VSNA/std/cpplib
 *  - VCS 3
 *
 * *****************  Version 37  *****************
 * User: Ktrushnikov  Date: 15.09.09   Time: 16:39
 * Updated in $/VSNA/std/cpplib
 * - Rating added
 * - display_name type changed to char* (from wchar_t)
 * - send BS Events to different service (AUTH,FWD)
 *
 * *****************  Version 36  *****************
 * User: Ktrushnikov  Date: 3.09.09    Time: 11:38
 * Updated in $/VSNA/std/cpplib
 * Save abooks at AS:
 *   - VS_Container: GetLongValueRef() added
 *   - TRANSPORT_SRCUSER_PARAM: pass user from Transport to Container
 *
 * *****************  Version 35  *****************
 * User: Mushakov     Date: 7.08.09    Time: 15:14
 * Updated in $/VSNA/std/cpplib
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 34  *****************
 * User: Mushakov     Date: 31.03.09   Time: 19:16
 * Updated in $/VSNA/std/cpplib
 * Reg server added
 *
 * *****************  Version 33  *****************
 * User: Smirnov      Date: 31.03.09   Time: 18:15
 * Updated in $/VSNA/std/cpplib
 * - stream symmetric crypt support
 *
 * *****************  Version 32  *****************
 * User: Smirnov      Date: 2.02.09    Time: 17:27
 * Updated in $/VSNA/std/cpplib
 * - update rights supported
 *
 * *****************  Version 31  *****************
 * User: Smirnov      Date: 21.11.08   Time: 12:58
 * Updated in $/VSNA/std/cpplib
 * - ban removed
 *
 * *****************  Version 30  *****************
 * User: Mushakov     Date: 20.11.08   Time: 17:48
 * Updated in $/VSNA/std/cpplib
 * - caching AddressBook added
 *
 * *****************  Version 29  *****************
 * User: Smirnov      Date: 3.10.08    Time: 18:09
 * Updated in $/VSNA/std/cpplib
 * - correct wrong parametrs in conference picture update
 *
 * *****************  Version 28  *****************
 * User: Mushakov     Date: 26.09.08   Time: 20:03
 * Updated in $/VSNA/std/cpplib
 * - New Group Conf's atributes supported;
 *
 * *****************  Version 27  *****************
 * User: Smirnov      Date: 25.09.08   Time: 17:29
 * Updated in $/VSNA/std/cpplib
 * - added "scope" flag for public conference
 *
 * *****************  Version 26  *****************
 * User: Mushakov     Date: 25.07.08   Time: 14:51
 * Updated in $/VSNA/std/cpplib
 * - logging app_ID added
 * - logging multi_conf
 * - bug 4602 fixed
 *
 * *****************  Version 25  *****************
 * User: Ktrushnikov  Date: 14.04.08   Time: 16:44
 * Updated in $/VSNA/std/cpplib
 * - "l_dns_states.UnLock()" called before
 * "m_dns->UpdateRecords(nfrecords, rfrecords)" to prevent lock for 15
 * seconds
 * - Update DNS in OnPointConnect/Disconnect implemented in Service thread
 * (not in Transport Thread)
 * - Processing BrokerEvents as responces from servers to ManageCommands
 * added
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 11.04.08   Time: 17:32
 * Updated in $/VSNA/std/cpplib
 * - protocol cleaning
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 26.03.08   Time: 15:32
 * Updated in $/VSNA/std/cpplib
 * - hashed login procedure
 *
 * *****************  Version 22  *****************
 * User: Mushakov     Date: 25.03.08   Time: 17:51
 * Updated in $/VSNA/std/cpplib
 *  - SSL added
 *  - fixed bug: Connect to server with another name
 *
 * *****************  Version 21  *****************
 * User: Ktrushnikov  Date: 6.03.08    Time: 17:36
 * Updated in $/VSNA/std/cpplib
 * - Use VS_Container params to pass params for LocalRequest
 * - Don't use a specific function for LocalRequest's
 * - VS_Protocol: LOCALREQUEST_METHOD added
 *
 * *****************  Version 20  *****************
 * User: Stass        Date: 18.02.08   Time: 21:43
 * Updated in $/VSNA/std/cpplib
 * added alias support (first ver)
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 5.12.07    Time: 11:09
 * Updated in $/VSNA/std/cpplib
 * BS - new iteration
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 ************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VS_Protocol.h"

/************************ Services**************************/
const char AUTH_SRV[]					= "AUTH";
const char ADDRESSBOOK_SRV[]			= "ABOOK";
const char CONFIGURATION_SRV[]			= "NET_CONFIG";
const char CONFERENCE_SRV[]				= "CONFERENCE";
const char CHAT_SRV[]					= "CHAT";
const char CHATV2_SRV[]                 = "CHATV2";
const char OFFLINECHAT_SRV[]			= "OFFLINECHAT";
const char PING_SRV[]					= "PING";
const char CLIENT_SRV[]					= "CLIENT";
const char PRESENCE_SRV[]				= "PRESENCE";
const char MANAGER_SRV[]				= "MANAGER";
const char MULTICONF_SRV[]				= "MULTI_CONFERENCE";
const char LOG_SRV[]					= "LOG";
const char RESOLVE_SRV[]				= "RESOLVE";
const char CHECKLIC_SRV[]				= "CHECKLIC";
const char VERIFY_SRV[]					= "VERIFY";
const char SERVERCONFIGURATOR_SRV[]		= "SERVERCONFIGURATOR";
const char WEBRTC_PEERCONNECTION_SRV[]	= "WEBRTC_PEERCONNECTION";

const char LOCATE_SRV[]					= "LOCATE";
const char REGISTRATION_SRV[]			= "REGISTRATION";
const char TRANSCODERSDISPATCHER_SRV[]	= "TRANSCODERSDISPATCHER";
const char GATEWAY_SRV[]				= "GATEWAY";
const char TORRENT_SRV[]				= "TORRENT";
const char SIPCALL_SRV[]				= "SIPCALL";
const char CALL_CFG_UPDATER_SRV[]       = "CALL_CFG_UPDATER";
const char DS_CONTROL_SRV[]				= "DS_CONTROL";

//------------------------ Common Param --------------------------//
const char METHOD_PARAM[]				= "Method";
const char RESULT_PARAM[]				= "Result";
const char TYPE_PARAM[]					= "Type";
const char DISCONNECT_REASON_PARAM[]	= "DisconnectReason";
const char GTYPE_PARAM[]				= "GType";
const char NAME_PARAM[]					= "Name";
const char TIME_PARAM[]					= "time";
const char FILETIME_PARAM[]				= "FileTime";
const char DATA_PARAM[]					= "Data";
const char SUBTYPE_PARAM[]				= "SubType";
const char SERVER_PARAM[]				= "Server";
const char SIGN_PARAM[]					="Sign";
const char TRANSPORT_SRCUSER_PARAM[]	= "TransportSrcUser";
const char TRANSPORT_SRCSERVER_PARAM[]	= "TransportSrcServer";
const char TRANSPORT_SRCSERVICE_PARAM[]	= "TransportSrcService";
const char TRANSPORT_DSTUSER_PARAM[]	= "TransportDstUser";
const char GID_PARAM[]					= "GID";
const char GNAME_PARAM[]				= "GName";
const char CMD_PARAM[]					= "Cmd";
const char ID_PARAM[]					= "id";
const char EDITABLE_PARAM[]				= "editable";
const char VALID_UNTIL_PARAM[]			= "validUntil";

const char MUTIPREFIX_CONST[]			= "@c_";

/**************** Endpoint Registartrion*********************/
//const char ENDPOINTIP_METHOD[]			= "EndpointIp";
//------------------------ Param --------------------------//
const char CLIENTVERSION_PARAM[]		= "ClientVersion";
const char CLIENTTYPE_PARAM[]			= "ClientType";
const char PROTOCOLVERSION_PARAM[]		= "ProtocolVersion";
const char PROPERTY_PARAM[]				= "Property";
const char APPNAME_PARAM[]				= "AppName";

/**************** User Registartrion *********************/
//------------------------ Param --------------------------//
const char LOGIN_PARAM[]				= "Login";
const char USERNAME_PARAM[]				= "UserName";
const char FIRSTNAME_PARAM[]			= "FirstName";
const char LASTNAME_PARAM[]				= "LastName";
const char EMAIL_PARAM[]				= "E-Mail";
const char PASSWORD_PARAM[]				= "Password";
const char DISPLAYNAME_PARAM[]			= "DisplayName";
const char CALLID_PARAM[]				= "CallId";
const char CALLID2_PARAM[]				= "CallID2";
const char ALIAS_PARAM[]				= "Alias";
const char REALID_PARAM[]				= "RealID";
const char USERCOMPANY_PARAM[]			= "company";
const char USERPHONE_PARAM[]			= "phone";
const char APPID_PARAM[]				= "appId";
const char PUBLIC_PARAM[]				= "isPublic";
const char TOPIC_PARAM[]				= "topic";
const char LANG_PARAM[]					= "lang";
const char TARIF_PARAM[]				= "tarif";
const char TARIFNAME_PARAM[]			= "TarifName";
const char SESSION_PARAM[]				= "Session";
const char TARIFRESTR_PARAM[]			= "TarifRestr";
const char USER_DEFAULT_DOMAIN[]		= "UserDefaultDomain";
const char EXTERNAL_ACCOUNT[]			= "ExternalAccount";
const char REGID_PARAM[]				= "RegID";
const char LOGINSTRATEGY_PARAM[]		= "LoginStrategy";
const char APPLICATION_SETTINGS_PARAM[] = "ApplicationSettings";
const char PROTOCOL_VERSION_PARAM[]		= "ProtocolVersion";
const char NEW_STATUS_PARAM[]			= "NewStatus";
const char OLD_STATUS_PARAM[]			= "OldStatus";
const char RELOAD_CONFIGURATION[]		= "ReloadConfiguration";
const char RESERVATION_TOKEN[]			= "ReservationToken";


/**************** Network Configuration *********************/
const char UPDATECONFIGURATION_METHOD[] = "UpdateConfiguration";
const char CONFIGURATIONUPDATED_METHOD[]= "ConfigurationUpdated";
const char GETAPPPROPERTIES_METHOD[]	= "GetAppProps";
const char GETALLAPPPROPERTIES_METHOD[]	= "GetAllAppProps";
const char SETPROPERTIES_METHOD[]		= "SetProps";
const char SETALLAPPPROPERTIES_METHOD[]	= "SetAllAppProps";
const char REDIRECTTO_METHOD[]			= "RedirectTo";
// for configurator
const char ONAPPPROPSCHANGE_METHOD[]	= "OnAppPropsChange";
// for Amazon DDNS
const char EXTERNAL_IP_DETERMINED_METHOD[] = "ExternalIPDetermined";
//------------------------ Param --------------------------//
const char IPCONFIG_PARAM[]				= "IPConfig";
const char IP6CONFIG_PARAM[]			= "IPv6Config";
const char EPCONFIG_PARAM[]				= "EpConfig";
const char HOST_PARAM[]					= "Host";
const char PORT_PARAM[]					= "Port";
const char MULTICAST_IP_PARAM[]			= "McastIp";
const char RESTRICTFLAGS_PARAM[]		= "RestrictFlags";
// this parameter server provides the transcoder to tell it,
// what type of ip protocol should be used:
// 1) IPv4 protocol ===> value = VS_IPPortAddress:ADDR_IPV4
// 2) IPv6 protocol ===> value = VS_IPPortAddress:ADDR_IPV6
const char IPADDRESSFAMILY_PARAM[]		= "IPAddressFamily";

/*************************** User Login *********************/
const char LOGINUSER_METHOD[]			= "LoginUser";
const char LOGINUSER2_METHOD[]			= "LoginUser2";
const char USERLOGGEDIN_METHOD[]		= "UserLoggedin";
const char LOGOUTUSER_METHOD[]			= "LogoutUser";
const char USERLOGGEDOUT_METHOD[]		= "UserLoggedout";
const char CHECKUSERLOGINSTATUS_METHOD[]= "CheckUserLoginStatus";
const char AUTHORIZE_METHOD[]           = "Authorize";
const char UPDATEACCOUNT_METHOD[]       = "UpdateAccount";
const char REQUPDATEACCOUNT_METHOD[]	= "ReqUpdateAccount";
const char SETREGID_METHOD[]			= "SetRegID";
const char UPDATE_PEERCFG_METHOD[]		= "UpdatePeerCfg_Method";
const char SETUSERENDPAPPINFO_METHOD[]	= "SetUserEndpointAppInfo";
// for configurator
const char ONUSERCHANGE_METHOD[]		= "OnUserChange";
//------------------------ Param --------------------------//
const char CAUSE_PARAM[]				= "Cause";
const char RIGHTS_PARAM[]				= "Rights";
const char RATING_PARAM[]				= "Rating";
const char SEPARATIONGROUP_PARAM[]		= "SeparationGroup";
const char SEQUENCE_PARAM[]				= "Seq";

const char AUTH_BY_ECP_METHOD[]			= "AuthByECP";
const char TICKET_ID_PARAM[]			= "ticketId";
const char TICKET_BODY_PARAM[]			= "ticketBody";
const char SIGNED_TICKET_BODY_PARAM[]	= "signedTicketBody";

/*************************** Conference *********************/
const char CREATECONFERENCE_METHOD[]	= "CreateConference";
const char CONFERENCECREATED_METHOD[]	= "ConferenceCreated";
const char DELETECONFERENCE_METHOD[]	= "DeleteConference";
const char CONFERENCEDELETED_METHOD[]	= "ConferenceDeleted";
const char USERACCESSIBLEON_METHOD[]	= "UserAccessibleOn";
const char USERREGISTRATIONINFO_METHOD[]= "UserRegistrationInfo";
const char INVITE_METHOD[]				= "Invite";
const char ACCEPT_METHOD[]				= "Accept";
const char REJECT_METHOD[]				= "Reject";
const char HANGUP_METHOD[]				= "Hangup";
const char RESTART_METHOD[]				= "Restart";
const char JOIN_METHOD[]				= "Join";
const char SENDPARTSLIST_METHOD[]		= "SendPartsList";
const char KICK_METHOD[]				= "Kick";
const char IGNORE_METHOD[]				= "Ignore";
const char NOTIFYMONEYUPDATE_METHOD[]	= "MoneyUpdate";
const char CONNECTRECEIVER_METHOD[]		= "ConnectReceiver";
const char RECEIVERCONNECTED_METHOD[]	= "ReceiverConnected";
const char CONNECTSENDER_METHOD[]		= "ConnectSender";
const char SENDERCONNECTED_METHOD[]		= "SenderConnected";
const char PLACECALL_METHOD[]			= "PlaceCall";
const char REQINVITE_METHOD[]			= "ReqInvite";
const char INVITETOMULTI_METHOD[]		= "InviteToMulti";
const char DELETEPARTICIPANT_METHOD[]	= "DeleteParticipant";
const char UPDATEPARTICIPANT_METHOD[]	= "UpdateParticipant";
const char LISTENERSFLTR_METHOD[]		= "ListenersFltr";
const char ROLEEVENT_METHOD[]			= "RoleEvent";
const char DEVICESTATUS_METHOD[]		= "DeviceStatus";
const char INVITEREPLY_METHOD[]			= "InviteReply";
const char CONNECTSERVICES_METHOD[]     = "ConnectServices";
const char INVITEUSERS_METHOD[]			= "InviteUsers";
const char SETLAYERS_METHOD[]			= "SetLayers";
const char CONNECTFAILURE_METHOD[]		= "ConnectFailure";
const char UPDATESETTINGS_METHOD[]		= "UpdateSettings";
const char MANAGELAYOUT_METHOD[]		= "ManageLayout";
const char MANAGECONEFRENCE_METHOD[]	= "ManageConference";
const char CHANGEDEVICE_METHOD[]		= "ChangeDevice";
const char SETDEVICESTATE_METHOD[]		= "SetDeviceState";
const char DEVICESLIST_METHOD[]			= "DevicesList";
const char DEVICECHANGED_METHOD[]		= "DeviceChanged";
const char DEVICESTATE_METHOD[]			= "DeviceState";
const char QUERYDEVICES_METHOD[]		= "QueryDevices";

const char SENDMAIL_METHOD[]			= "SendMail";

const char UPDATECONFERENCEPIC_METHOD[]	= "UpdateConferencePic";

const char SETMYLSTATUS_METHOD[]		= "SetMyLStatus";
const char CLEARALLLSTATUSES_METHOD[]	= "ClearAllLStatuses";
const char SENDSLIDESTOUSER_METHOD[]	= "SendSlidesToUser";

const char AUTOINVITE_METHOD[]			= "AutoInvite";

//------------------------ Param --------------------------//
const char OWNER_PARAM[]				= "Owner";
const char DURATION_PARAM[]				= "Duration";
const char MAXPARTISIPANTS_PARAM[]		= "MaxPartisipants";
const char MAXCAST_PARAM[]				= "MaxCast";
const char DESTROYCONDITION_PARAM[]		= "DestroyCondition";
const char CONFERENCE_PARAM[]			= "Conference";
const char ENDPOINT_PARAM[]				= "EndPoint";
const char BROKER_PARAM[]				= "Broker";
const char PRODUCTID_PARAM[]			= "ProductId";
const char DIRECTCONNECT_PARAM[]		= "DirectConnect";
const char NEEDCONNECTINFO_PARAM[]		= "NeedConnectInfo";
const char MEDIAFILTR_PARAM[]			= "MediaFiltr";
const char SERVICES_PARAM[]             = "Services";
const char CLIENTCAPS_PARAM[]			= "ClientCaps";
const char RESTRICTBITRATE_PARAM[]		= "RestrictBtr";
const char RESTRICTBITRATESVC_PARAM[]	= "RestrictBtrSVC";
const char ROLE_PARAM[]					= "Role";
const char PREVIEW_BODY_PARAM[]			= "PreviewBodyParam";
const char PREVIEW_TYPE_PARAM[]			= "PreviewTypeParam";
const char IS_OPERATOR_PARAM[]			= "IsOperator";
const char DEVICESTATUS_PARAM[]			= "DeviceStatus";
const char SCOPE_PARAM[]				= "Scope";
const char PICTURE_PARAM[]				= "Picture";
const char SYMKEY_PARAM[]				= "SymKey";
const char SVCMODE_PARAM[]				= "SVCMode";
const char REQESTKEYFRAME_PARAM[]		= "RequestKeyFrame";
const char SYSLOAD_PARAM[]				= "SystemLoad";
const char FRAMESIZEMBUSER_PARAM[]		= "FrameSizeMBUser";
const char LAYOUT_PARAM[]				= "Layout";

const char SERVERNAME_PARAM[]			= "ServerName";
const char SERVERID_PARAM[]				= "ServerId";
const char LSTATUS_PARAM[]				= "LStatus";
const char MUSTJOIN_PARAM[]				= "MustJoin";

const char UCALL_PARAM[]				= "UCall";
const char PVIEW_PARAM[]				= "PView";

const char FUNC_PARAM[]					= "Func";
const char PERMISSIONS_PARAM[]			= "Permissions";
const char SOFT_KICK_PARAM[]            = "Soft";

const char VOLUME_PARAM[]				= "Volume";
const char MUTE_PARAM[]					= "Mute";
const char ACTIVE_PARAM[]				= "Active";

const char START_TIME_PARAM[]			= "StartTime";
const char STOP_TIME_PARAM[]			= "StopTime";

/********************** Desktop Sharing ************************/
const char VIDEOSOURCETYPE_METHOD[]		= "VideoSourceType";
const char DSCONTROL_REQUEST_METHOD[]	= "DSControlRequest";
const char DSCONTROL_RESPONSE_METHOD[]	= "DSControlResponse";
const char DSCONTROL_FINISH_METHOD[]	= "DSControlFinish";
const char DSCONTROL_FINISHED_METHOD[]	= "DSControlFinished";
const char DSCOMMAND_METHOD[]			= "DSCommand";

const char VIDEOTYPE_PARAM[]			= "VideoType";

/*************************** Chat *****************************/
const char SENDMESSAGE_METHOD[]			= "SendMessage";
const char SENDCOMMAND_METHOD[]			= "SendCommand";
//------------------------ Param --------------------------//
const char TO_PARAM[]					= "To";
const char FROM_PARAM[]					= "From";
const char MESSAGE_PARAM[]				= "Message";
const char MESSAGE_CONTAINER_PARAM[]	= "MessageContainer";
const char DB_MESSAGE_ID_PARAM[]		= "DBMessageId";
//---------------- Commands subparameters ------------------//
const char EXTRAVIDEOFLOW_NOTIFY[]		= "ExtraVideoFlowNotify";
const char CONTENTAVAILABLE_PARAM[]		= "ContentAvailable";
const char CURRENTVIDEO_PARAM[]			= "CurrentVideo";
const char MAIN_VIDEO[]					= "MainVideo";
const char CONTENT_VIDEO[]				= "ContentVideo";
const char SHOWCAMERA_REQ[]				= "ShowCamera";
const char SHOWCONTENT_REQ[]			= "ShowContent";
const char CONTENTFORWARD_PUSH[]		= "ContentForward:Push";
const char CONTENTFORWARD_PULL[]		= "ContentForward:Pull";
const char CONTENTFORWARD_STOP[]		= "ContentForward:Stop";

// recording
const char CCMD_RECORD_QUERY[] = "plugin:Record:OnQuery";
const char CCMD_RECORD_ACCEPT[] = "plugin:Record:Accept";
const char CCMD_RECORD_REJECT[] = "plugin:Record:Reject";
const char CCMD_RECORD_AGAIN[] = "VRecord:WriteAgain";
const char CCMD_RECORD_BUSY[] = "VRecord:Busy";
const char CCMD_RECORDING_OFF[] = "VRecord:OffNotify";
const char CCMD_RECORDING_ON[] = "VRecord:OnNotify";

// old desktop sharing control
const char CCMD_DSC_REQUEST[] = "DesktopSharing:RequestControl";
const char CCMD_DSC_ACCEPT[] = "DesktopSharing:AcceptToTakeControl";
const char CCMD_DSC_RELEASE[] = "DesktopSharing:ReleaseControl";
const char CCMD_DSC_BREAK[] = "DesktopSharing:BreakControl";
const char CCMD_DSC_DECLINE[] = "DesktopSharing:DeclineControl";
const char CCMD_DSC_CONTROL[] = "descktopsharing:%S";

// conference events
const char CCMD_CONFEV_DATANOTAVALIBLE[] = "Conf:DataNotAvailable";
const char CCMD_CONFEV_REMOVEDBY[] = "Conf:RemovedByOwner";
const char CCMD_CONFEV_CONFCLOSED[] = "Conf:ClosedByOwner";

// manage device
const char CCMD_CAM_ON[] = "remote:cam_on";
const char CCMD_CAM_OFF[] = "remote:cam_off";
const char CCMD_MIC_ON[] = "remote:mic_on";
const char CCMD_MIC_OFF[] = "remote:mic_off";


/*************************** Ping *****************************/
const char PING_METHOD[]				= "Ping";

/*************************** Client *********************/
const char TRYACCEPTDIRECT_METHOD[]		= "TryAcceptDirect";

/************************ Address Book ************************/
const char ADDTOADDRESSBOOK_METHOD[]	= "AddToAddressBook";
const char REMOVEFROMADDRESSBOOK_METHOD[]= "RemoveFromAddressBook";
const char SEARCHADDRESSBOOK_METHOD[]	= "SearchAddressBook";
const char GETUSERSTATUS_METHOD[]		= "GetUserStatus";
const char UPDATEADDRESSBOOK_METHOD[]	= "UpdateAddressBook";
const char ONADDRESSBOOKCHANGE_METHOD[]	= "OnAddressBookChange";
const char MANAGEGROUPS_METHOD[]		= "ManageGroups";
/************************ MBps users list ************************/
const char MBPSLIST_METHOD[]			= "MBpsList";

//------------------------ Param --------------------------//
const char HOPS_PARAM[]					= "Hops";
const char USERPRESSTATUS_PARAM[]		= "UserPresStatus";
const char QUERY_PARAM[]				= "Query";
const char ADDRESSBOOK_PARAM[]			= "AddressBook";
const char PUBCONFSTATUS_PARAM[]		= "PubConfStatus";
const char SEQUENCE_ID_PARAM[]			= "SeqID";
const char HASH_PARAM[]			        = "Hash";
const char LIFETIME_PARAM[]				= "LifeTime";
const char LOCATED_CALLID_PARAM[]		= "LocatedCallId";
const char LAST_DELETED_CALL_PARAM[]	= "last_deleted_call";

/************************ Manager Service ************************/
const char UPDATEINFO_METHOD[]			= "UpdateInfo";
const char GETASOFMYDOMAIN_METHOD[]		= "GetASOfMyDomain";
//------------------------ Param --------------------------//
const char LOCATORBS_PARAM[]			= "LocatorBS";
const char RS_PARAM[]					= "RS";
const char ASDOMAIN_PARAM[]				= "ASDomain";
/*********************** Directory Service ***********************/
const char GETALLUSERSTATUS_METHOD[]	= "GetAllUserStatus";
const char RESOLVE_METHOD[]				= "Resolve";
const char RESOLVEALL_METHOD[]				= "ResolveAll";
const char UPDATESTATUS_METHOD[]		= "UpdateStatus";

const char REGISTERSTATUS_METHOD[]		= "RegisterStatus";
const char UNREGISTERSTATUS_METHOD[]	= "UnRegisterStatus";
const char POINTCONNECTED_METHOD[]		= "PointConnected";
const char POINTDISCONNECTED_METHOD[]	= "PointDisconnected";
const char POINTDETERMINEDIP_METHOD[]	= "PointDeterminedIP";
const char PUSHSTATUSDIRECTLY_METHOD[]	="PushStatusDirectly";
const char RESENDSTATUS_METHOD[]		="ResendStatus";
const char SETLOCATORBS_METHOD[]		= "SETLOCATORBS_METHOD";

const char GETSUBSCRIPTIONS_METHOD[]	= "GetSubscriptions";
const char SUBSCRIBE_METHOD[]			= "Subscribe";
const char UNSUBSCRIBE_METHOD[]			= "Unsubscribe";


const char ADDPEER_METHOD[]     		= "AddPeer";
const char REMOVEPEER_METHOD[]  		= "RemovePeer";

const char EXTERNALPRESENCESTARTED_METHOD[]		= "ExtPresenceStarted";


//------------------------ Param --------------------------//
const char REPLICATE_PARAM[]			= "Replicate";
const char STATE_PARAM[]				= "State";

/************************** Log Service *************************/
const char LOGCONFSTART_METHOD[]		= "LogConfCreate";
const char LOGCONFEND_METHOD[]			= "LogConfEnd";
const char LOGPARTINVITE_METHOD[]		= "LogPartInvite";
const char LOGPARTJOIN_METHOD[]			= "LogPartJoin";
const char LOGPARTREJECT_METHOD[]		= "LogPartReject";
const char LOGPARTLEAVE_METHOD[]		= "LogPartLeave";
const char LOGPARTSTAT_METHOD[]			= "LogPartStat";
const char LOGRECORDSTART_METHOD[]      = "LogRecordStart";
const char LOGRECORDSTOP_METHOD[]       = "LogRecordStop";
const char LOGPARTICIPANTDEVICE_METHOD[]= "LogParticipantDevice";
const char LOGPARTICIPANTROLE_METHOD[]	= "LogParticipantRole";
//------------------------ Param --------------------------//
		//part invite
const char PRICE_PARAM[]				= "Price";
const char CHARGE1_PARAM[]				= "Charge1";
const char CHARGE2_PARAM[]				= "Charge2";
const char CHARGE3_PARAM[]				= "Charge3";

		//part leave
const char BYTES_SENT_PARAM[]			= "BytesSent";
const char BYTES_RECEIVED_PARAM[]		= "BytesRcvd";
const char RECON_SND_PARAM[]			= "ReconSnd";
const char RECON_RCV_PARAM[]			= "ReconRcv";
const char JOIN_TIME_PARAM[]			= "JoinTime";
		//base conference statistics
const char CONF_BASE_STAT_PARAM[]		= "ConferenceBaseStat";


/******************** Global Registration Service ******************/
const char LOGEVENT_METHOD[]			= "LogEvent";
const char LOGSTATS_METHOD[]			= "LogStats";
const char REGISTERSERVER_METHOD[]		= "RegisterBroker";
const char REGISTERSERVEROFFLINE_METHOD[]= "RegisterBrokerOffline";
const char REGISTERSERVEROFFLINEFROMFILE_METHOD[] = "RegisterBrokerOfflineFromFile";
const char UPDATELICENSE_METHOD[]		= "UpdateLicense";
const char UPDATELICENSEOFFLINE_METHOD[]= "UpdateLicenseOffline";
const char REQUEST_LICENSE_METHOD[]		= "RequestLicenseMethod";
const char SHARE_LICENSE_METHOD[]		= "ShareLicenseMethod";
const char SHARED_LICENSE_CHECK_METHOD[] = "SharedLicenseCheckMethod";
const char SHARED_LICENSE_CHECK_METHOD_RESP[] = "SharedLicenseCheckMethodResponse";
const char RETURN_SHARED_LICENSE_METHOD[] = "ReturnSharedLicenseMethod";
const char RETURN_LICENSE_FORCE_TAG[]	= "ReturnLicenseByForce";
const char MASTER_CHANGED_EVENT[]		= "MasterChangedEvent";
const char RETURN_SHARED_LICENSE_METHOD_RESP[] = "ReturnSharedLicenseMethodResponse";
const char SERVERVERIFYFAILED_METHOD[]	= "ServerVerifyFailed";
const char MANAGESERVER_METHOD[]		= "ManageServer";
const char CERTIFICATEUPDATE_METHOD[]	= "CertificateUpdate";
const char UPDATE_CALLCFGCORRECTOR_METHOD[] = "UpdateCallConfigCorrector";
//------------------------ Param --------------------------//
const char MEDIABROKERSTATS_PARAM[]		= "MediaBrokerStats";
const char KEY_PARAM[]					= "Key";
const char FIELD1_PARAM[]				= "Field1";
const char FIELD2_PARAM[]				= "Field2";
const char FIELD3_PARAM[]				= "Field3";
const char CERT_REQUEST_PARAM[]			= "CertRequest";
const char CERTIFICATE_PARAM[]			= "Certificate";
const char CERTIFICATE_CHAIN_PARAM[]	= "Certificate Chain";
const char PARENT_CERT_PARAM[]			= "Parent Certificate";

const char SM_CERTIFICATE_PARAM[]		= "SM_Certificate";

const char INVITATIONUPDATE_METHOD[]	= "InvitationUpdate";

const char SCRIPT_PARAM[]               = "Script";

const char SERVER_VERSION_PARAM[]		= "ServerVersion";


////Gateway

const char TRANSCODERINIT_METHOD[]	= "TranscoderInit";
const char PREPARETRANSCODERFORCALL_METHOD[] = "PrepareTranscoderForCall";
const char SETMEDIACHANNELS_METHOD[] = "SetMediaChannels";
const char CLEARCALL_METHOD[] = "ClearCall";

const char REQUEST_CALL_ID_PARAM[]	= "RequestCallID";
const char REQUEST_EMAIL_PARAM[]	= "RequestEmail";
const char REQUEST_NAME_PARAM[]	= "RequestName";


// Torrents

const char DOWNLOAD_METHOD[] = "DownloadMethod";
const char MANUALLY_CONNECT_METHOD[] = "ManuallyConnectMethod";
const char IS_FINISHED_METHOD[] = "IsFinishedMethod";
const char SHARE_METHOD[] = "Share";
const char MAGNET_LINK_PARAM[] = "MagnetLinkParam";
const char INFO_HASH_PARAM[] = "InfoHashParam";
const char FILENAME_PARAM[] = "FileNameParam";
const char IPPORT_PARAM[] = "IpPortParam";
const char URL_PARAM[] = "URL_PARAM";
const char LINK_PARAM[] = "LinkParam";
const char ABOUT_PARAM[] = "ABOUT_PARAM";

const char TRACKERS_PROP[] = "trackers";

///Common comstants
const char DEFAULT_DESTINATION_CALLID[] = "@@default_destination@@";
const char DEFAULT_DESTINATION_CALLID_SIP[] = "@@default_destination_sip@@";
const char DEFAULT_DESTINATION_CALLID_H323[] = "@@default_destination_h323@@";
const char GROUPCONF_PREFIX[] = "\\c\\";
const char SPECIAL_CONF_PREFIX[] = "$c";
const char EMPTY_CONFERENCE_TAG[] = "empty conference";

///extended statuses

const char EXTSTATUS_PARAM[] = "ExtSt";
const char DELETE_EXT_STATUS_PARAM[] = "DelExtSt";
/// depricated
const char EXTSTATUS_NAME_EXT_STATUS[] = "ext_status";
const char EXTSTATUS_NAME_DESCRIPTION[] = "status_description";
const char EXTSTATUS_NAME_LAST_ONLINE_TIME[] = "last_online_time";
const char EXTSTATUS_NAME_DEVICE_TYPE[] = "device_type";
const char EXTSTATUS_NAME_CAMERA[] = "camera";
const char EXTSTATUS_NAME_IN_CLOUD[] = "in_cloud";
const char EXTSTATUS_NAME_FWD_TYPE[] = "fwd_type";
const char EXTSTATUS_NAME_FWD_CALL_ID[] = "fwd_call_id";
const char EXTSTATUS_NAME_FWD_TIMEOUT[] = "fwd_timeout";
const char EXTSTATUS_NAME_FWD_TIMEOUT_CALL_ID[] = "fwd_timeout_call_id";

const char ALLOWED_BY_SERVER_MAX_BW[] = "AllowedByServerMaxBW";
const char ALLOWED_BY_SERVER_MAX_FPS[] = "AllowedByServerMaxFPS";
const char ALLOWED_BY_SERVER_MAX_WXH[] = "AllowedByServerMaxWxH";

// FECC (FarEnd Camera Control)
const char FECC_METHOD[]				= "FECC";
const char FECC_STATE_PARAM[]			= "FeccState";
const char PRESET_NUM_PARAM[]			= "PresetNum";

// Slideshow
const char SHOW_SLIDE_COMMAND[] = "SHOW_SLIDE_COMMAND:";
const char END_SLIDESHOW_COMMAND[] = "END_SLIDESHOW_COMMAND :";
const char IMG_TYPE_PARAM[] = "TYPE_PARAM";
const char WIDTH_PARAM[] = "WIDTH_PARAM";
const char HEIGHT_PARAM[] = "HEIGHT_PARAM";
const char SIZE_PARAM[] = "SIZE_PARAM";
const char SLIDE_N_PARAM[] = "SLIDE_N";
const char SLIDE_COUNT_PARAM[] = "SLIDE_COUNT";
const char FROM_RTP_PARAM[] = "FROM_RTP";

// DB logging
const char OBJECT_TYPE_PARAM[] = "OBJECT_TYPE";
const char EVENT_TYPE_PARAM[] = "EVENT_TYPE";
const char OBJECT_NAME_PARAM[] = "OBJECT_NAME";
const char PAYLOAD_PARAM[] = "PAYLOAD_PARAM";

const char SERVER_OBJECT_TYPE[] = "server";
const char USER_OBJECT_TYPE[] = "user";
const char LOGIN_USER_EVENT_TYPE[] = "login";
const char AUTHORIZE_USER_EVENT_TYPE[] = "authorize";
const char STATUS_USER_EVENT_TYPE[] = "status";
const char LOGOUT_USER_EVENT_TYPE[] = "logout";
const char START_SERVER_EVENT_TYPE[] = "start";
const char STOP_SERVER_EVENT_TYPE[] = "stop";
const char CONNECT_SERVER_EVENT_TYPE[] = "connect";
const char DISCONNECT_SERVER_EVENT_TYPE[] = "disconnect";

// ConferenceModuleRights
const char CMR_FLAGS_PARAM[] = "CMRFlags";

const char HANGUP_FLAGS_PARAM[] = "HangupFlags";

const char MULTI_LOGIN_CAPABILITY_PARAM[] = "MultiLoginCapability";
