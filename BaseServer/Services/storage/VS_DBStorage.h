/**
 ****************************************************************************
 * (c) 2007 Visicron Systems, Inc.  http://www.visicron.net/
 *
 * Project: Visicron server services
 *
 * $Revision: 74 $
 * $History: VS_DBStorage.h $
 *
 * *****************  Version 74  *****************
 * User: Ktrushnikov  Date: 14.08.12   Time: 18:13
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #12325
 * - more simple logic at server & db: don't check server property (or app
 * property) for valid params from client
 *
 * *****************  Version 73  *****************
 * User: Ktrushnikov  Date: 8.08.12    Time: 19:16
 * Updated in $/VSNA/servers/baseserver/services
 * #12325
 * - use vs_update_person stored procedure
 *
 * *****************  Version 72  *****************
 * User: Ktrushnikov  Date: 8.08.12    Time: 13:12
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #12325
 * - ChangeProfile support added
 *
 * *****************  Version 71  *****************
 * User: Ktrushnikov  Date: 7.08.12    Time: 17:11
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #12322
 * - change password from client supported
 * - return result from ADODB as Parameter (not as Recordset)
 *
 * *****************  Version 70  *****************
 * User: Ktrushnikov  Date: 3.08.12    Time: 18:12
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #12823
 * - after AddToAddressBook call OnAddressBookChange to update AB_GROUP
 * for owner
 *
 * *****************  Version 69  *****************
 * User: Mushakov     Date: 16.07.12   Time: 16:29
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - g_dbStorage was wrapped to shared_ptr
 *
 * *****************  Version 68  *****************
 * User: Ktrushnikov  Date: 3.04.12    Time: 12:58
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - my aliases support added (for Android) AB_MY_EXTERNAL_CONTACTS=18
 * spec: http://projects.trueconf.ru/bin/view/Projects/MobilePhoneNumbers
 *
 * *****************  Version 67  *****************
 * User: Ktrushnikov  Date: 24.02.12   Time: 14:27
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - vs_bak_add_user supported (Add & Remove)
 *
 * *****************  Version 66  *****************
 * User: Ktrushnikov  Date: 26.10.11   Time: 14:18
 * Updated in $/VSNA/Servers/BaseServer/Services
 * NamedConf
 * - set MaxCast for NamedConf with value from DB (edit server property
 * named_conf_max_cast)
 *
 * *****************  Version 65  *****************
 * User: Ktrushnikov  Date: 20.10.11   Time: 13:47
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - FindUsersGroups, Rename, Delete: with no RecordSet
 * - Groups: if no hash - then hash=1 (2000 year)
 *
 * *****************  Version 64  *****************
 * User: Ktrushnikov  Date: 5.10.11    Time: 12:34
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - handle no RecordSet at FindUser() at PostreSQL
 *
 * *****************  Version 63  *****************
 * User: Ktrushnikov  Date: 26.07.11   Time: 19:52
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - Hash added for AB_GROUPS
 * - ManageGroups moved to base interface class
 *
 * *****************  Version 62  *****************
 * User: Ktrushnikov  Date: 15.07.11   Time: 15:22
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #8535, #8917
 * - user groups supported in Visicron.dll and BS server
 * (VS_AddressBookService, VS_DBStorage)
 *
 * *****************  Version 61  *****************
 * User: Ktrushnikov  Date: 11.07.11   Time: 21:52
 * Updated in $/VSNA/Servers/BaseServer/Services
 * [#8917]
 * - get AB_GROUPS from DB and send to client
 *
 * *****************  Version 60  *****************
 * User: Ktrushnikov  Date: 18.05.11   Time: 9:23
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - support conf_status in GetNamedConfInfo()
 *
 * *****************  Version 59  *****************
 * User: Ktrushnikov  Date: 28.03.11   Time: 23:06
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - email offline participant at VS_InvitesStorage
 * - new template added
 *
 * *****************  Version 58  *****************
 * User: Ktrushnikov  Date: 1.03.11    Time: 22:53
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - pass current stream_conf_id of a named_conf_id from AS to BS->DB
 *
 * *****************  Version 57  *****************
 * User: Ktrushnikov  Date: 23.11.10   Time: 16:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - restart BS on any connection error (don't do reconnects or Init())
 *
 * *****************  Version 56  *****************
 * User: Ktrushnikov  Date: 13.11.10   Time: 14:50
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - sp_get_properties_list: appName param added
 * - pass appName from client to AS->BS->DB
 *
 * *****************  Version 55  *****************
 * User: Ktrushnikov  Date: 22.09.10   Time: 16:43
 * Updated in $/VSNA/servers/baseserver/services
 * - new search from client by [name, call_id, email]
 * - sp_set_group_endpoint_properties: directX param size fixed for
 * PostgreSQL
 * - VS_WideStr::Trim() function
 *
 * *****************  Version 54  *****************
 * User: Ktrushnikov  Date: 20.09.10   Time: 15:18
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - Aliases support for PostgreSQL
 *
 * *****************  Version 53  *****************
 * User: Ktrushnikov  Date: 3.09.10    Time: 15:53
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - restart server if DB Reocnnects more than MAX_DB_RECONNECTS=20 times
 * (over 50 MB memory leaks)
 *
 * *****************  Version 52  *****************
 * User: Ktrushnikov  Date: 1.09.10    Time: 15:01
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - check DB connection at ADO COM Error only
 * - re Init DBStorage, not one DB Object (like in MS SQL)
 * - NativeError: driver error code for PostgreSQL connection problem
 * added
 *
 * *****************  Version 51  *****************
 * User: Ktrushnikov  Date: 31.08.10   Time: 17:18
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - db_ping before each procedure
 * - Reconnect if DB connection dead
 * - small cleanup at db::init()
 *
 * *****************  Version 50  *****************
 * User: Ktrushnikov  Date: 30.08.10   Time: 20:10
 * Updated in $/VSNA/Servers/BaseServer/Services
 * [#7571]
 * - New BS Event "TARIFF_CHANGED": generate UPDATEACCOUNT with TarifName,
 * TarifRestrictions, UserRights
 *
 * *****************  Version 49  *****************
 * User: Ktrushnikov  Date: 25.08.10   Time: 20:31
 * Updated in $/VSNA/Servers/BaseServer/Services
 * physical restrictions on tarif (creating confs from client)
 *
 * *****************  Version 48  *****************
 * User: Ktrushnikov  Date: 19.08.10   Time: 17:59
 * Updated in $/VSNA/Servers/BaseServer/Services
 * back to auto_key from matvery's ret_key
 *
 * *****************  Version 47  *****************
 * User: Ktrushnikov  Date: 10.08.10   Time: 22:29
 * Updated in $/VSNA/servers/baseserver/services
 * - vs_get_login_session_secret (dummy call)
 *
 * *****************  Version 46  *****************
 * User: Ktrushnikov  Date: 10.08.10   Time: 21:28
 * Updated in $/VSNA/Servers/BaseServer/Services
 * [#7561]: GenerateSessionKey() at LoginUser and at ReqUpdateAccount()
 * [#7562]: update login session key at timer timeout in Visicron.dll
 *
 * *****************  Version 45  *****************
 * User: Mushakov     Date: 26.07.10   Time: 19:27
 * Updated in $/VSNA/Servers/BaseServer/Services
 *
 * *****************  Version 44  *****************
 * User: Ktrushnikov  Date: 7.07.10    Time: 21:12
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - named confs procedures & parameters fixed
 * - init_named_conf_invitations processing added
 *
 * *****************  Version 43  *****************
 * User: Ktrushnikov  Date: 1.07.10    Time: 20:56
 * Updated in $/VSNA/Servers/BaseServer/Services
 * Arch 3.1: NamedConfs
 * - DB interfaces created (with dummy & untested imp)
 * - Processing of InvitationUpdate_Method added
 * - BS Event "NAMED_CONF" rewritten
 * - bug fix with storing changed in struct (in map)
 * - some reject messages added
 *
 * *****************  Version 42  *****************
 * User: Ktrushnikov  Date: 25.05.10   Time: 20:07
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - IsPostgreSQL flag added
 * - some dummy funcs for PostgreSQL
 *
 * *****************  Version 41  *****************
 * User: Mushakov     Date: 19.03.10   Time: 17:59
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - ClientType is received from client
 * - Endpoint function removed from code
 * - arr_key logged inRegistryServer
 *
 * *****************  Version 40  *****************
 * User: Ktrushnikov  Date: 7.03.10    Time: 17:20
 * Updated in $/VSNA/servers/baseserver/services
 * - All storages are inherited from VS_DBStorageInterface (DB, Reg, LDAP,
 * Trial)
 * - g_vcs_storage changed to g_dbStorage
 * - TConferenceStatistics added
 * - Process of LogPartStat added for VCS(file) & BS(null)
 * - fixed with d78 TransportRoute::DeleteService (dont delete deleted
 * service)
 * BS::LogSrv: suppress_missed_call_mail property added
 *
 * *****************  Version 39  *****************
 * User: Mushakov     Date: 15.09.09   Time: 21:15
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - some changes are canceled
 *
 * *****************  Version 36  *****************
 * User: Mushakov     Date: 7.08.09    Time: 15:14
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 35  *****************
 * User: Ktrushnikov  Date: 8.05.09    Time: 17:53
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - cache for OfflineChatMessages
 *
 * *****************  Version 34  *****************
 * User: Mushakov     Date: 20.02.09   Time: 19:26
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - AppId added in ParticipantDescr (invite, join log)
 * - 2 DBs supported
 *
 * *****************  Version 33  *****************
 * User: Ktrushnikov  Date: 1.12.08    Time: 13:49
 * Updated in $/VSNA/Servers/BaseServer/Services
 * [#5231] param CallId added
 *
 * *****************  Version 32  *****************
 * User: Ktrushnikov  Date: 13.11.08   Time: 18:32
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - BS Events added
 *
 * *****************  Version 31  *****************
 * User: Mushakov     Date: 26.09.08   Time: 20:03
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - New Group Conf's atributes supported;
 *
 * *****************  Version 30  *****************
 * User: Mushakov     Date: 17.09.08   Time: 16:55
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - properties encoding corrected
 *
 * *****************  Version 29  *****************
 * User: Mushakov     Date: 10.09.08   Time: 19:08
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - sending missed call mail for email of existing user (call_id ==
 * email)
 *
 * *****************  Version 28  *****************
 * User: Mushakov     Date: 9.09.08    Time: 16:19
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - bug 4603 fixed
 *
 * *****************  Version 27  *****************
 * User: Mushakov     Date: 25.07.08   Time: 14:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - logging app_ID added
 * - logging multi_conf
 * - bug 4602 fixed
 *
 * *****************  Version 26  *****************
 * User: Mushakov     Date: 11.06.08   Time: 18:45
 * Updated in $/VSNA/Servers/BaseServer/Services
 * additional logging ProcessCOMError added
 *
 * *****************  Version 25  *****************
 * User: Mushakov     Date: 26.05.08   Time: 19:37
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - params added to SetAllEpProperties method
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 19.05.08   Time: 18:32
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - removed ep_update_sc
 *
 * *****************  Version 23  *****************
 * User: Mushakov     Date: 15.05.08   Time: 15:04
 * Updated in $/VSNA/Servers/BaseServer/Services
 * missed call mail corrcted
 *
 * *****************  Version 22  *****************
 * User: Ktrushnikov  Date: 16.04.08   Time: 21:28
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - "wan_ip" param is set alone, not with SetAllEpProperties
 * (sp_set_group_edpoint_properties)
 *
 * *****************  Version 21  *****************
 * User: Stass        Date: 15.04.08   Time: 13:10
 * Updated in $/VSNA/Servers/BaseServer/Services
 * offline chat
 *
 * *****************  Version 20  *****************
 * User: Stass        Date: 2.04.08    Time: 21:44
 * Updated in $/VSNA/Servers/BaseServer/Services
 * added set server to BS presense
 *
 * *****************  Version 19  *****************
 * User: Stass        Date: 1.04.08    Time: 20:24
 * Updated in $/VSNA/Servers/BaseServer/Services
 * added ban list support
 *
 * *****************  Version 18  *****************
 * User: Ktrushnikov  Date: 16.03.08   Time: 12:24
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - Server Property functions commented (cause they are not used)
 * - set_user_status uses stored procedure in DB (not a hardcoded
 * SQL-string)
 *
 * *****************  Version 17  *****************
 * User: Ktrushnikov  Date: 15.03.08   Time: 12:07
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - std::vector now used to pass return data from VS_DBStorage
 * - get data from DB by char-constant, not by index
 *
 * *****************  Version 16  *****************
 * User: Ktrushnikov  Date: 12.03.08   Time: 20:01
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - BS: CHAT_SRV added
 * - Pass offline chat messages from AS::RESOLVE_SRV to BS::CHAT_SRV
 * - Save offline chat messages in DB via sp_friend_im_save stored
 * procedure
 *
 * *****************  Version 15  *****************
 * User: Ktrushnikov  Date: 11.03.08   Time: 15:40
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - call PostRequest for each message (not one call)
 *
 * *****************  Version 14  *****************
 * User: Ktrushnikov  Date: 9.03.08    Time: 15:00
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - added GetOfflineChatMessages() using sp_notify_login stored procedure
 * from VS_DBStorage
 *
 * *****************  Version 13  *****************
 * User: Ktrushnikov  Date: 4.03.08    Time: 16:05
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - fix: IS_AUIDO_PARAM = "@audio" but must be "@is_audio"
 *
 * *****************  Version 12  *****************
 * User: Stass        Date: 27.02.08   Time: 21:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 * new parameters in addtoAB
 *
 * *****************  Version 11  *****************
 * User: Ktrushnikov  Date: 19.02.08   Time: 15:32
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - build errors fix
 *
 * *****************  Version 10  *****************
 * User: Ktrushnikov  Date: 17.02.08   Time: 10:58
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - HardwareTest: fix output cause of funcs have other names
 * - Stored Procedure: call SetAllEpProperties() for known params
 * - split Capabilities into: is_audio, is_mic, is_camera
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 15.02.08   Time: 20:21
 * Updated in $/VSNA/Servers/BaseServer/Services
 * sending missed call mail and invate mail added
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 12.02.08   Time: 19:13
 * Updated in $/VSNA/Servers/BaseServer/Services
 * GetAppProperties method realized
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 21.01.08   Time: 19:55
 * Updated in $/VSNA/Servers/BaseServer/Services
 * BaseUserPresence added
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 7.12.07    Time: 13:21
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fixed link problems
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 6.12.07    Time: 18:58
 * Updated in $/VSNA/Servers/BaseServer/Services
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 6.12.07    Time: 18:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * base services done
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 5.12.07    Time: 11:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 * BS - new iteration
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 30.11.07   Time: 20:36
 * Updated in $/VSNA/Servers/BaseServer/Services
 ****************************************************************************/

/**
* \file VS_DBStorage.h
* \brief Server Database Storage class definition
*
*/
#ifndef VS_SERVER_DB_STORAGE_H
#define VS_SERVER_DB_STORAGE_H


#include "../../../common/std/cpplib/VS_Pool.h"
#include "../../../ServerServices/Common.h"
#include "../../../common/tools/Watchdog/VS_Testable.h"
#include "VS_DBStorageInterface.h"
#include "VS_DBNull.h"
#include "VS_DBObjects_CallLog.h"
#include "tlb_import/msado26.tlh"

#include <memory>
#include <vector>
#include <set>

////////////////////////////////////////////////////////////////////////////////
// DB fields
static const char ID[]						="id";
static const char DB_TYPE[]					="type";
static const char DB_SUBTYPE[]				="subtype";
static const char DB_STATUS[]				="status";
static const char DB_RESULT[]				="result";
static const char DB_TIME[]					="time";
static const char REG_DATE[]				="reg_date";
static const char LAST_CONNECTED[]			="last_connected";
static const char DB_START_TIME[]			="start_time";
static const char DB_END_TIME[]				="end_time";
static const char DB_RIGHTS[]				="rights";

static const char DB_CONF_STATUS[]			="conf_status";



static const char LAST_CONNECTED_PARAM[]	="@last_connected";
//static const char DB_START_DB_TIME_PARAM[]			=	"@start_time";

static const char SEARCH_TYPE_PARAM[]		="@search_type";

static const char DB_LOGIN_PARAM[]			="@login";
static const char DB_PASSWORD_PARAM[]			="@password";
static const char DB_DISPLAYNAME_PARAM[]	="@display_name";
static const char DB_TEMP_LOGIN_DATA_PARAM[] = "@bs_value";

static const char USER_STATUS_PARAM[]		="@user_status";

static const char USER_EXT_STATUS_NAME_PARAM[]= "ext_status_name";
static const char INTVAL_PARAM[] = "int_val";
static const char STRVAL_PARAM[] = "char_val";
static const char TIMESTAMPVAL_PARAM[] = "ts_val";

static const char DB_NAME_PARAM[]			="@name";
static const char USER_EMAIL_PARAM[]		="@email";
//static const char USER_FIRSTNAME_PARAM[]	="@first_name";
//static const char USER_LASTNAME_PARAM[]	="@last_name";

static const char DB_QUERY_PARAM[]			="@query";


static const char USER_ID[]				="user_id";
static const char CALL_ID[]				="call_id";

// ---- For PostgreSQL
static const char CALL_ID_OUT[]				="call_id_out";
static const char DB_DISPLAYNAME_OUT[]		="display_name_out";
// ----

static const char APP_NAME[]			="app_name";

static const char PASSWORD[]			="password";
static const char DB_LOGIN[]			="login";
static const char DB_DISPLAYNAME[]		="display_name";

static const char USER_STATUS[]			="user_status";

static const char USER_FIRSTNAME[]		="first_name";
static const char USER_LASTNAME[]		="last_name";
static const char USER_EMAIL[]			="email";


static const char EP_PROTOCOL_VERSION_PARAM[]	="@prot_version";
static const char DB_AUTO_KEY_PARAM[]			="@auto_key";
//+type

static const char EP_NET_INFO_PARAM[]			="@net_info";
static const char EP_LOCAL_NET_INFO_PARAM[]		="@local_net_info";

static const char EP_VERSION_PARAM[]			="@version";
static const char DB_APP_NAME_PARAM[]			="@app_name";


static const char EP_WAN_IP_PARAM[]				="@wan_ip";

static const char EP_SYS_CONF_PARAM[]			="@sys_conf";
static const char EP_PROCESSOR_PARAM[]			="@processor";
static const char EP_DIRECTX_PARAM[]			="@directX";
static const char EP_HARDWARE_CONFIG_PARAM[]	="@hardwareConfig";
static const char EP_AUDIO_CAPTURE_PARAM[]		="@AudioCapture";
static const char EP_VIDEO_CAPTURE_PARAM[]		="@VideoCapture";
static const char EP_AUDIO_RENDER_PARAM[]		="@AudioRender";


static const char EP_REGISTRAR_PARAM[]			="@registrar_user_id";

static const char CALL_ID_FROM[]				="@c_call_id_from";
static const char DISPLAY_NAME_FROM[]			="@c_display_name_from";
static const char CALL_ID_TO[]					="@c_call_id_to";
static const char DISPLAY_NAME_TO[]				="@c_display_name_to";
static const char TYPE_MAIL_TEMPLATE[]			="@c_type";
static const char TIME_MISSED_CALL[]			="@d_date";
static const char BODY[]						="@c_body";
static const char TIME_MESSAGE[]				= "@d_created_date";


static const char EMAIL[]						="email";
static const char EMAIL_FROM[]					="email_from";
static const char MAIL_SUBJECT_TEMPLATE[]		="mail_subject_template";
static const char MAIL_BODY_TEMPLATE[]			="mail_body_template";




//+reg date
//+last connected

static const char EP_SYS_CONF[]					="sys_conf";
static const char EP_PROCESSOR[]				="Processor";
static const char EP_DIRECTX[]					="Direct X";
static const char EP_HARDWARE_CONFIG[]			="Hardware Config";
static const char EP_AUDIO_CAPTURE[]			="Audio Capture";
static const char EP_VIDEO_CAPTURE[]			="Video Capture";
static const char EP_AUDIO_RENDER[]				="Audio Render";


static const char APP_ID[]						="app_id";
static const char EP_CURRENT_USER[]				="current_user_id";
static const char EP_LAST_KNOWN_USER[]			="last_known_user_id";
static const char EP_AUTOLOGIN[]				="autologin";
static const char EP_PROTOCOL_VERSION[]			="prot_version";
static const char DB_AUTO_KEY[]					="auto_key";
//+type

static const char EP_NET_INFO[]				="net_info";
static const char EP_LOCAL_NET_INFO[]		="local_net_info";

static const char EP_VERSION[]				="version";
static const char EP_WAN_IP[]				="wan_ip";
static const char EP_REGISTRAR[]			="registrar_user_id";
//+reg date
//+last connected

// Invitations
static const char DB_INVITE_TIME_PARAM[]		="@invite_time";
static const char DB_INVITE_DAYS_PARAM[]		="@invite_days";
static const char DB_INVITE_EMAIL_PARAM[]		="@invite_email";
static const char DB_PARTICIPANTS_PARAM[]		="@participants";


static const char BOOK_PARAM[]					= "@book";

static const char CURRENT_DB_TIME_PARAM[]		="@current_time";
static const char IS_STARTUP_PARAM[]			="@is_startup";

static const char PROP_NAME_PARAM[]				="@prop_name";
static const char PROP_VALUE_PARAM[]			="@prop_value";

static const char TRIGGER_PARAM[]				="@trigger";
static const char ERROR_CODE_PARAM[]			="error_code";

static const char DB_CLIENT_TYPE_PARAM[]		="@client_type";

class VS_DBStorage;
class VS_TransportRouterService;

/*class BSEvent
{
public:
	VS_Container*	cnt;
	VS_SimpleStr	to;
	VS_SimpleStr	broker_id;

	BSEvent(): cnt(0) {}
};*/

class VS_DBObjects : public VS_DBObjects_CallLog
{
public:
	int error_code;
	bool IsPostgreSQL;
	//db
	ADODB::_ConnectionPtr db;
	ADODB::_ConnectionPtr db1;
	ADODB::_CommandPtr ab_add_user,ab_remove_user;
	ADODB::_CommandPtr ab_get,ab_missed_calls,ab_received_calls,ab_placed_calls,ab_get_person_details, ab_get_missed_call_mail;
	ADODB::_CommandPtr ban_user,unban_user;
	ADODB::_CommandPtr ab_update_user,ab_update_details,ab_update_picture,ab_delete_picture;
	ADODB::_CommandPtr user_login, sp_login_temp_set_param, user_get_rights, user_resolve_alias, user_get_external_accounts;
	ADODB::_CommandPtr user_find;		std::string user_find_template;
	ADODB::_CommandPtr /*log_conf_start,log_conf_end,log_part_join,log_part_leave,log_part_invite,log_part_stats, */update_conf_info;
	ADODB::_CommandPtr app_prop_get_all,app_prop_get,app_prop_get_all_props, srv_prop_get,/*srv_prop_get_all,srv_prop_set,*/ep_prop_get,ep_prop_set,ep_group_prop_set,user_prop_get;

	ADODB::_CommandPtr notify_login, friend_im_save;

	ADODB::_CommandPtr user_set_status, user_get_status, user_clear_statuses,
	user_set_ext_status, user_get_ext_status, ext_statuses_get_allowed;

	ADODB::_CommandPtr cleanup;
	ADODB::_CommandPtr get_bs_events;
	ADODB::_CommandPtr get_users_with_offline_messages;
	ADODB::_CommandPtr get_named_conf_info; std::string get_named_conf_info_template;
	ADODB::_CommandPtr set_named_conf_server;
	ADODB::_CommandPtr get_named_conf_participants; std::string get_named_conf_participants_template;
	ADODB::_CommandPtr init_named_conf_invitations; std::string init_named_conf_invitations_template;

	ADODB::_CommandPtr get_login_session_secret;
	ADODB::_CommandPtr get_properties_list;		std::string get_properties_list_template;	// for PostgreSQL only

	ADODB::_CommandPtr get_aliases;		std::string get_aliases_template;		// for PostgreSQL only
	ADODB::_CommandPtr search_users;											// for PostgreSQL only

	ADODB::_CommandPtr ab_group_get_list_users;		std::string ab_group_get_list_users_template;
	ADODB::_CommandPtr ab_group_create;//	std::wstring ab_group_create_template;		// for PostgreSQL only
	ADODB::_CommandPtr ab_group_delete;		std::string ab_group_delete_template;
	ADODB::_CommandPtr ab_group_edit;		std::wstring ab_group_edit_template;

	ADODB::_CommandPtr ab_group_add_user;
	ADODB::_CommandPtr ab_group_delete_user;

	ADODB::_CommandPtr ab_external_add_user;
	ADODB::_CommandPtr ab_external_delete_user;

	ADODB::_CommandPtr sp_add_alias;
	ADODB::_CommandPtr vs_delete_alias;

	ADODB::_CommandPtr change_password;
	ADODB::_CommandPtr update_person;

	ADODB::_CommandPtr ab_phones_find;		std::string ab_phones_find_template;
	ADODB::_CommandPtr ab_phones_add;
	ADODB::_CommandPtr ab_phones_update;
	ADODB::_CommandPtr ab_phones_delete;

	ADODB::_CommandPtr sp_ntf_web_log_insert;
	ADODB::_CommandPtr sp_ntf_update_reg_id;

	ADODB::_CommandPtr create_file_transfer,
					   add_file_to_transfer,
					   add_file_transfer_stat,
					   add_file_transfer_files_stat,
					   delete_file_transfer,
					   delete_file_transfer_owner,
					   set_file_transfer_delete_files,
					   set_file_transfer_delete_files_by_days,
					   get_file_transfer,
					   set_file_transfer_owner_cnt;
	ADODB::_CommandPtr get_server_by_login, get_server_by_call_id;
	VS_DBObjects();
	bool Init(VS_DBStorage* dbs);
	void ProcessCOMError(const _com_error &e, ADODB::_ConnectionPtr cur_db,VS_DBStorage* dbs, const char *add_descr);
};
class VS_DBOFactory : public VS_Pool::Factory
{
	VS_DBStorage* m_dbs;
public:
	VS_DBOFactory(VS_DBStorage* dbs): m_dbs(dbs)
	{};
	bool New(VS_Pool::Data& data) override
	{
		VS_DBObjects* dbo=new VS_DBObjects();
		data=dbo;
		return dbo->Init(m_dbs) && dbo->error_code==0;
	}
	void Delete(VS_Pool::Data data) override
	{
		delete (VS_DBObjects*)data;
	}

};

/**
*  Class for encapsulation of db storage operations
*
*/
class VS_DBStorage : public VS_DBStorageInterface, public VS_DBNULL_OBJ
{
	friend class VS_DBObjects;
private:
	std::shared_ptr<VS_Pool>	m_dbo_pool;
	VS_Lock	m_allowed_ext_statuses_lock;
	std::set<std::string> m_allowed_ext_statuses;
	VS_Lock	m_additional_app_properties_lock;
	std::map<VS_SimpleStr, VS_SimpleStr> m_additional_app_properties;
	inline static void MakeConfID(const int conference, const VS_SimpleStr& server_id, vs_conf_id& conf_id);
	VS_Lock			m_usersWithOfflineMessages_lock;
	VS_StrIStrMap	m_usersWithOfflineMessages;
	void InitUsersWithOfflineMessages();
	int SearchUsers(VS_Container& rCnt, VS_Container& cnt, const vs_user_id& owner);
protected:
	static std::string STRING_SEPARATOR;
	enum States {
		STATE_CREATED=0,
		STATE_RUNNING,
		STATE_RECONNECT,
		STATE_FAILED
	};
	int				state;
	void FetchUser		(ADODB::FieldsPtr f,VS_UserData& user);
	VS_DBObjects* GetDBO(const VS_Pool::Item* &item);
	void ReleaseDBO(const VS_Pool::Item* item)
	{	m_dbo_pool->Release(item);	};
	const char* GetParam(const char* param, bool IsPosgresSQL = false);
	int GetPropertiesList(const char* call_id, const char* appName, VS_Container& prop_cnt);
	void SetExtStatus(VS_DBObjects *dbo,const char* call_id,const VS_ExtendedStatusStorage &extStatus);
	bool GetExtendedStatus(VS_DBObjects *dbo, const char *call_id, VS_ExtendedStatusStorage &sticky);

	void UpdateListAllowedExtStatuses();

public:
	VS_DBStorage();
	~VS_DBStorage();
	bool Init(const VS_SimpleStr& broker_name) override;
	//testable interface
	bool		Test(void) override
	{
		return (state==STATE_FAILED)?false:true;
	};
	void CleanUp() override;
	//users
	bool FindUser(const vs_user_id& id, VS_UserData& user, bool find_by_call_id_only = true) override;
	void SetUserStatus(const VS_SimpleStr& call_id, int status, const VS_ExtendedStatusStorage &extStatus, bool set_server, const VS_SimpleStr& server) override;
	bool GetExtendedStatus(const char *call_id, VS_ExtendedStatusStorage &) override;
	void ClearStatuses(const VS_SimpleStr& server) override;
	int	LoginUser(const VS_SimpleStr& login, const VS_SimpleStr& password, const VS_SimpleStr& appID, VS_SimpleStr& autoKey, const VS_SimpleStr& appServer, VS_UserData& user, VS_Container& prop_cnt, const VS_ClientType &client_type = CT_SIMPLE_CLIENT) override;
	//address books
	int	FindUsers(VS_Container& cnt, int& entries, VS_AddressBook ab, const vs_user_id& owner, const std::string& query, long client_hash, VS_Container& in_cnt) override;
	int	AddToAddressBook(VS_AddressBook ab, const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt, VS_SimpleStr& add_call_id, std::string& add_display_name, VS_TransportRouterServiceHelper* srv = 0) override;
	int	RemoveFromAddressBook(VS_AddressBook ab, const vs_user_id& user_id1, const vs_user_id& user_id2, VS_Container& cnt, long& hash, VS_Container& rCnt) override;
	int	UpdateAddressBook(VS_AddressBook ab, const vs_user_id& user_id1, const char* call_id2, VS_Container& cnt, long& hash, VS_Container& rCnt) override;
	//properties:
	//applications
	int GetAppProperties(VS_Container& prop, const VS_SimpleStr& app_name) override;
	int GetAllAppProperties(VS_AppPropertiesMap &prop_map) override;
	bool SetAppProperty(const VS_SimpleStr& prop_name, const VS_SimpleStr &value) override;
	bool GetAppProperty(const VS_SimpleStr& app_name, const VS_SimpleStr& prop_name, VS_SimpleStr& value) override;
	bool GetAppProperty(const VS_SimpleStr& app_name, const VS_SimpleStr& prop_name, VS_WideStr& value) override;
	bool GetServerProperty(const std::string& prop_name, std::string &value) override;
	bool SetServerProperty(const VS_SimpleStr& name, const VS_SimpleStr& value) override;
	bool GetServerTime(std::chrono::system_clock::time_point& ft) override;
	//endpoint
	bool SetEndpointProperty(const char* ep_id, const char* name, const char* value) override;
	bool SetEndpointProperty(const char* ep_id, const char* name, const wchar_t* value) override;
	bool SetAllEpProperties(const char* app_id, const int prot_version, const short int type, const wchar_t* version,
									const /*char*/wchar_t *app_name, const /*char*/wchar_t* sys_conf, const /*char*/wchar_t* processor, const /*char*/ wchar_t *directX,
									const /*char*/wchar_t* hardwareConfig, const /*char*/wchar_t* AudioCapture, const /*char*/wchar_t *VideoCapture, const /*char**/wchar_t* AudioRender, const char* call_id) override;
	bool GetEndpointProperty(const VS_SimpleStr& ep_id, const VS_SimpleStr& _name, VS_SimpleStr& value) override;
	//user
	bool GetUserProperty(const vs_user_id& user_id, const std::string& name, std::string& value) override;
	int  SaveTempLoginData(const char* login, const char* app_id, VS_Container &data) override;
	//logging
	bool	LogConferenceStart(const VS_ConferenceDescription& conf, bool remote = false) override;
	bool	LogConferenceEnd(const VS_ConferenceDescription& conf) override;
	bool	LogParticipantInvite	(const vs_conf_id& conf_id,const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,
		const std::chrono::system_clock::time_point time = std::chrono::system_clock::time_point(), VS_ParticipantDescription::Type type = VS_ParticipantDescription::PRIVATE_HOST) override;
	bool	LogParticipantJoin(const VS_ParticipantDescription& pd, const VS_SimpleStr& callid2 = NULL) override;
	bool	LogParticipantReject(const vs_conf_id& conf_id, const vs_user_id& user, const vs_user_id& invited_from, const VS_Reject_Cause cause) override;
	bool	LogParticipantLeave(const VS_ParticipantDescription& pd) override;
	bool	LogParticipantStatistics(const VS_ParticipantDescription& pd) override;
	bool	LogSlideShowCmd(const char *confId, const char *from, const char *url, const char *mimeType, size_t slideIndex, size_t slidesCount, const char *about,
		size_t width, size_t height, size_t size) override;
	bool	LogSlideShowEnd(const char *confId, const char *from) override;
	bool	LogGroupChat(const char *confId, const char *from, const char *text) override;
	bool	UpdateConferencePic(VS_Container &cnt) override;
	bool	GetMissedCallMailTemplate(const std::chrono::system_clock::time_point missed_call_time, const char* fromId, std::string& inOutfromDn, const char* toId, std::string& inOutToDn, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ) override;
	bool	GetInviteCallMailTemplate(const std::chrono::system_clock::time_point missed_call_time, const char *fromId, std::string& inOutfromDn, const char *toId, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ) override;
	bool	GetMissedNamedConfMailTemplate(const char* fromId, std::string& inOutfromDn, const char* toId, std::string& inOutToDn, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ) override;
	int		GetOfflineChatMessages(const char* call_id, std::vector<VS_Container> &vec) override;
	bool	SetOfflineChatMessage(const VS_SimpleStr& from_call_id, const VS_SimpleStr& to_call_id, const VS_SimpleStr& body_utf8, const std::string& from_dn, const VS_Container &cont) override;
	void	GetBSEvents(std::vector<BSEvent> &vec) override;
	void	GetUpdatedExtStatuses(std::map<std::string,VS_ExtendedStatusStorage>&) override;
	int		GetNamedConfInfo(const char* conf_id, VS_ConferenceDescription& cd, ConferenceInvitation& ci, VS_SimpleStr& as_server, long& scope) override;
	void	SetNamedConfServer(const char* named_conf_id, const char* stream_conf_id) override;
	int		GetNamedConfParticipants(const char* conf_id, ConferenceInvitation& ci) override;
	int		InitNamedConfInvitaions(std::vector<std::string> &v) override;
	VS_SimpleStr GetLoginSessionSecret() override;
	long ChangePassword(const char* call_id, const char* old_pass, const char* new_pass, const VS_SimpleStr& from_app_id) override;
	long UpdatePerson(const char* call_id, VS_Container& cnt) override;
	long SetRegID(const char* call_id, const char* reg_id, VS_RegID_Register_Type reg_type) override;
	long NotifyWeb_MissedCall(VS_Container& cnt) override;
	std::shared_ptr<VS_Pool> dbo_pool() { return m_dbo_pool; }
public:
	virtual bool IsConferendoBS() const = 0;
};

#endif //VS_SERVER_DB_STORAGE_H
