/**
 ****************************************************************************
 * (c) 2004 Visicron Inc.  http://www.visicron.net/
 *
 * Project: Visicron server services
 *
 * $Revision: 50 $
 * $History: VS_ConfMemStorage.h $
 *
 * *****************  Version 50  *****************
 * User: Ktrushnikov  Date: 3.08.12    Time: 18:12
 * Updated in $/VSNA/Servers/VCS/Services
 * #12823
 * - after AddToAddressBook call OnAddressBookChange to update AB_GROUP
 * for owner
 *
 * *****************  Version 49  *****************
 * User: Ktrushnikov  Date: 3.08.12    Time: 15:22
 * Updated in $/VSNA/Servers/VCS/Services
 * #11593: registry & ldap searching better match spec
 * (http://projects.trueconf.ru/bin/view/Projects/ClientSearch#VCS)
 *
 * *****************  Version 48  *****************
 * User: Mushakov     Date: 17.07.12   Time: 23:09
 * Updated in $/VSNA/Servers/VCS/Services
 *  - LoginConfigurator() was removed
 * - messages from configurator are handled by SessionID
 * - fix TransportMessage::IsFromServer()
 *
 * *****************  Version 47  *****************
 * User: Ktrushnikov  Date: 25.06.12   Time: 19:21
 * Updated in $/VSNA/servers/vcs/services
 * LDAP
 * - No login group supported
 * - UpdateRegistry when server starts
 * - Set "LDAP Status"=1 when user login
 * - delete of user at AD when server offline supported
 *
 * *****************  Version 46  *****************
 * User: Ktrushnikov  Date: 15.06.12   Time: 12:09
 * Updated in $/VSNA/Servers/VCS/Services
 * new ldap:
 * - VS_LDAPCore split for tc_server.exe & tc_conf.dll
 *
 * *****************  Version 45  *****************
 * User: Ktrushnikov  Date: 22.05.12   Time: 11:05
 * Updated in $/VSNA/Servers/VCS/Services
 * #12014
 * - appProps at Trial storage (one code for all storages)
 *
 * *****************  Version 44  *****************
 * User: Ktrushnikov  Date: 16.05.12   Time: 18:02
 * Updated in $/VSNA/Servers/VCS/Services
 * #11435: UsersEndpoints fixed
 * - "Registered" of endpoint
 * - endpoint info at LDAP mode
 * - don't save endpoint info of CT_TRANSCODER
 *
 * *****************  Version 43  *****************
 * User: Ktrushnikov  Date: 14.05.12   Time: 16:43
 * Updated in $/VSNA/Servers/VCS/Services
 * #11810
 * - autologin by autoKey at LDAP
 *
 * *****************  Version 42  *****************
 * User: Ktrushnikov  Date: 27.04.12   Time: 14:34
 * Updated in $/VSNA/Servers/VCS/Services
 * #11729: SetUserStatus at LDAP also
 *
 * *****************  Version 41  *****************
 * User: Ktrushnikov  Date: 17.04.12   Time: 17:30
 * Updated in $/VSNA/Servers/VCS/Services
 * - support for more than one alias per user
 *
 * *****************  Version 40  *****************
 * User: Ktrushnikov  Date: 16.04.12   Time: 17:14
 * Updated in $/VSNA/Servers/VCS/Services
 * #11593: Registry
 * - search by phone
 * - m_aliasList fixed
 * - by TrueConfID, by e-mail user find not stricmp
 *
 * *****************  Version 39  *****************
 * User: Ktrushnikov  Date: 11/03/11   Time: 5:10p
 * Updated in $/VSNA/Servers/VCS/Services
 * - LDAP & Registry: UpdateUsersGroups fix
 * - Login for MultiGatewayTranscoder
 *
 * *****************  Version 38  *****************
 * User: Ktrushnikov  Date: 19.09.11   Time: 21:24
 * Updated in $/VSNA/Servers/VCS/Services
 * #9802,#9803
 * - lerya statuses
 *
 * *****************  Version 37  *****************
 * User: Ktrushnikov  Date: 31.07.11   Time: 12:37
 * Updated in $/VSNA/Servers/VCS/Services
 * bitrix: display_name of guests in Trial mode
 *
 * *****************  Version 36  *****************
 * User: Ktrushnikov  Date: 26.07.11   Time: 19:55
 * Updated in $/VSNA/Servers/VCS/Services
 * - Hash added for AB_GROUPS
 * - ManageGroups moved to base interface class
 *
 * *****************  Version 35  *****************
 * User: Ktrushnikov  Date: 15.07.11   Time: 21:50
 * Updated in $/VSNA/Servers/VCS/Services
 * - VCS: interface for manage client groups (not implemented yet)
 *
 * *****************  Version 34  *****************
 * User: Ktrushnikov  Date: 11.04.11   Time: 22:37
 * Updated in $/VSNA/Servers/VCS/Services
 * - LDAP Status: skip users with value==1 (for Zobov)
 *
 * *****************  Version 33  *****************
 * User: Ktrushnikov  Date: 28.03.11   Time: 23:06
 * Updated in $/VSNA/Servers/VCS/Services
 * - email offline participant at VS_InvitesStorage
 * - new template added
 *
 * *****************  Version 32  *****************
 * User: Ktrushnikov  Date: 24.03.11   Time: 17:23
 * Updated in $/VSNA/Servers/VCS/Services
 * - timeout delete, because it is useless
 *
 * *****************  Version 31  *****************
 * User: Ktrushnikov  Date: 24.03.11   Time: 0:37
 * Updated in $/VSNA/Servers/VCS/Services
 * LDAP:
 * - added more locks for m_reg_groups, m_rights_cache
 * - rights: CREATEMULTI fix with tarif_opt
 * - improve locks of m_UsersGroupsCnt
 * - PoolThreadTask in VCSAuthService for OnUserChange(type=4)
 *
 * *****************  Version 30  *****************
 * User: Ktrushnikov  Date: 21.03.11   Time: 22:08
 * Updated in $/VSNA/Servers/VCS/Services
 * New Groups style
 * - supported in LDAP mode
 * - one container for all users
 * - get list of online users from g_storage
 * - fix: VS_SimpleStorage::UpdateUsersGroups() send GID instead of ID
 *
 * *****************  Version 29  *****************
 * User: Ktrushnikov  Date: 17.03.11   Time: 19:36
 * Updated in $/VSNA/Servers/VCS/Services
 * new style of system groups
 * - implementation (alfa)
 * - FindUserExtended() renamed to overloaded FindUser()
 * - OnUserChange(): type==4 supported
 *
 * *****************  Version 28  *****************
 * User: Ktrushnikov  Date: 17.02.11   Time: 20:18
 * Updated in $/VSNA/Servers/VCS/Services
 * #8429
 * - file_name according to year and month
 * - get history from current and previous month
 *
 * *****************  Version 27  *****************
 * User: Ktrushnikov  Date: 17.02.11   Time: 18:18
 * Updated in $/VSNA/Servers/VCS/Services
 * VCS 3.2
 * - #8429: store Call History in SQLite file
 *
 * *****************  Version 26  *****************
 * User: Ktrushnikov  Date: 27.01.11   Time: 13:40
 * Updated in $/VSNA/Servers/VCS/Services
 * VCS 3.2
 * - offline chat messages in roaming
 * - on of SubscriptionHub::Subscribe() fixed
 *
 * *****************  Version 25  *****************
 * User: Ktrushnikov  Date: 15.12.10   Time: 16:01
 * Updated in $/VSNA/Servers/VCS/Services
 * - conf stat to file in AS (was onlyin VCS)
 * - "Save Conf Stat" dword reg key added
 *
 * *****************  Version 24  *****************
 * User: Ktrushnikov  Date: 24.11.10   Time: 20:32
 * Updated in $/VSNA/Servers/VCS/Services
 * - clear "Logged User" at UsersEndpoints at Init()
 *
 * *****************  Version 23  *****************
 * User: Ktrushnikov  Date: 13.11.10   Time: 12:47
 * Updated in $/VSNA/Servers/VCS/Services
 * - 3 LDAP filters defaults changed (e-mail, filter login, filter callid)
 * - Send Mail Templates fix for LDAP
 *
 * *****************  Version 22  *****************
 * User: Ktrushnikov  Date: 13.10.10   Time: 13:40
 * Updated in $/VSNA/Servers/VCS/Services
 * #7944: do OfflineChatCleanup() once a day
 *
 * *****************  Version 21  *****************
 * User: Ktrushnikov  Date: 13.10.10   Time: 10:59
 * Updated in $/VSNA/Servers/VCS/Services
 * [#7944]
 * - expire time support added (=30 days by default)
 * - Timer() for LDAP renamed
 *
 * *****************  Version 20  *****************
 * User: Ktrushnikov  Date: 13.10.10   Time: 9:11
 * Updated in $/VSNA/Servers/VCS/Services
 * [#7944] offline chat messages support in VCS
 * - sqlite added to ^std
 * - confRestrict interface (VCS with AS)
 * - get/set offline chat messages in VS_SimpleStorage
 * - get offline chat messages at login in VCSAuthService
 *
 * *****************  Version 19  *****************
 * User: Ktrushnikov  Date: 11.10.10   Time: 14:27
 * Updated in $/VSNA/Servers/VCS/Services
 * #7924: VIDEORECORDING bit in Rights by License
 * #7923: TarifRestrictions in VCS
 * - restrick in VCSConfRestrick
 * - send TARIFRESTR_PARAM to client
 *
 * *****************  Version 18  *****************
 * User: Mushakov     Date: 7.10.10    Time: 19:55
 * Updated in $/VSNA/Servers/VCS/Services
 * - organization name from certificate in DUI added
 *
 * *****************  Version 17  *****************
 * User: Ktrushnikov  Date: 27.09.10   Time: 11:29
 * Updated in $/VSNA/Servers/VCS/Services
 * VCS::LDAP crash when client start
 * - duplicate error_code removed from VS_SimpleStorage (is in
 * VS_DBStorageInterface)
 * - check return code of Init() at ctor of VS_LDAPStorage &
 * VS_RegistryStorage
 *
 * *****************  Version 16  *****************
 * User: Ktrushnikov  Date: 2.09.10    Time: 13:45
 * Updated in $/VSNA/Servers/VCS/Services
 * - LoginUser() interface fix with prop_cnt
 *
 * *****************  Version 15  *****************
 * User: Ktrushnikov  Date: 17.08.10   Time: 20:49
 * Updated in $/VSNA/Servers/VCS/Services
 * [#7454]
 * - update clients when ManageAddressBook
 * - Configurator bug bypass with small Sleep (temporary)
 *
 * *****************  Version 14  *****************
 * User: Ktrushnikov  Date: 7.07.10    Time: 17:04
 * Updated in $/VSNA/Servers/VCS/Services
 * [VCS 3.0.7]
 * - Endpoint properites didn't come: Local Ip, IP, Logged User
 * - save m_app_id at login at auto_login too (not just by login/pass)
 * - UPDATECONFIGURATION_METHOD: don't send ourservice, cause there will
 * be no SrcUser. call directly.
 * - PointConditions added to CONFIGURATION_SRV
 * - g_dbStorage->SetEndpointProperty support empty string
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 14.05.10   Time: 15:53
 * Updated in $/VSNA/Servers/VCS/Services
 * - ab_directory commented
 *
 * *****************  Version 12  *****************
 * User: Ktrushnikov  Date: 7.04.10    Time: 13:39
 * Updated in $/VSNA/Servers/VCS/Services
 * [#7140]
 * - properties fix
 *
 * *****************  Version 11  *****************
 * User: Ktrushnikov  Date: 29.03.10   Time: 20:50
 * Updated in $/VSNA/Servers/VCS/Services
 * - return autoKey in stack at Login
 * - FindUserExtended refactoring
 * - CleanUp
 * - VS_UserDescrption renamed to VS_UserData
 *
 * *****************  Version 10  *****************
 * User: Ktrushnikov  Date: 24.03.10   Time: 20:59
 * Updated in $/VSNA/Servers/VCS/Services
 * - mem alloc
 *
 * *****************  Version 9  *****************
 * User: Ktrushnikov  Date: 23.03.10   Time: 12:07
 * Updated in $/VSNA/Servers/VCS/Services
 * - Guest login support for LDAP mode
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 19.03.10   Time: 17:59
 * Updated in $/VSNA/Servers/VCS/Services
 *  - ClientType is received from client
 * - Endpoint function removed from code
 * - arr_key logged inRegistryServer
 *
 * *****************  Version 7  *****************
 * User: Ktrushnikov  Date: 7.03.10    Time: 17:58
 * Updated in $/VSNA/Servers/VCS/Services
 * headers fix
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 7.03.10    Time: 17:20
 * Updated in $/VSNA/Servers/VCS/Services
 * - All storages are inherited from VS_DBStorageInterface (DB, Reg, LDAP,
 * Trial)
 * - g_vcs_storage changed to g_dbStorage
 * - TConferenceStatistics added
 * - Process of LogPartStat added for VCS(file) & BS(null)
 * - fixed with d78 TransportRoute::DeleteService (dont delete deleted
 * service)
 * BS::LogSrv: suppress_missed_call_mail property added
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 24.02.10   Time: 13:50
 * Updated in $/VSNA/Servers/VCS/Services
 * - Read Aliases from Rigistry
 * - Add aliases per user
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 23.02.10   Time: 14:20
 * Updated in $/VSNA/Servers/VCS/Services
 * - removed empty classes
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 12.02.10   Time: 13:13
 * Updated in $/VSNA/Servers/VCS/Services
 * - named group conf alfa
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 27.01.10   Time: 13:59
 * Updated in $/VSNA/Servers/VCS/Services
 * VCS:
 * - Fix user status in Registry
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 3.01.10    Time: 15:16
 * Created in $/VSNA/Servers/VCS/Services
 * - VCS refactoried
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 3.01.10    Time: 13:51
 * Updated in $/VSNA/Servers/VCS/Storage
 * - LicenseEvents added
 * - restricted build in restrict interface included
 * - RESTRICTED_BUILD removed
 * - Reading "group allowed" flag from licenses added
 * - guest autorization in LDAP mode added
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 2.12.09    Time: 18:50
 * Updated in $/VSNA/Servers/VCS/Storage
 * Configuration service added
 * Guest: don't add to m_users map
 * VS_StorageUserData
 * - save AppID-Key pairs per User
 * VSAuthService
 * - process second login of user (logout first user)
 * VCSStorage
 * - added FindUserExtended: get Key param by login
 * - interface cleanup (confs)
 * VS_ConfMemStorage
 * - removed from inheritance
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 4.11.09    Time: 21:13
 * Created in $/VSNA/Servers/VCS/Storage
 *  - new names
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 4.11.09    Time: 18:49
 * Updated in $/VSNA/Servers/SBSv3_m/Storage
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 2.11.09    Time: 15:03
 * Updated in $/VSNA/Servers/SBSv3_m/Storage
 * - store users by login only
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 28.10.09   Time: 17:58
 * Updated in $/VSNA/Servers/SBSv3_m/Storage
 * - vs_ep_id removed
 * - registration corrected
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 7.08.09    Time: 14:48
 * Created in $/VSNA/Servers/SBSv3_m/Storage
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 24.08.07   Time: 13:50
 * Updated in $/VS2005/Servers/MediaBroker/BrokerServices
 * - fix: not implemented GetEndpointNetConf for SBS
 * - fix: return only Online Endpoints in FindEnpoints method for SBS
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 25.05.07   Time: 18:56
 * Updated in $/VS2005/Servers/MediaBroker/BrokerServices
 * - lock endpoint methods in simple storage
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 15  *****************
 * User: Stass        Date: 7.12.06    Time: 17:26
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * moved endpoint keys save from storage to ep reg srv
 *
 * *****************  Version 14  *****************
 * User: Stass        Date: 8.11.06    Time: 15:51
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 13  *****************
 * User: Stass        Date: 30.10.06   Time: 18:46
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added gateway counter
 *
 * *****************  Version 12  *****************
 * User: Stass        Date: 20.09.06   Time: 13:34
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * changed endpoint keys processing
 * moved reg endpoints to SimpleStorage
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 18.09.06   Time: 14:47
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * - async all storage working services
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 14.09.06   Time: 17:13
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * resolve alias from registry
 *
 * *****************  Version 9  *****************
 * User: Stass        Date: 23.05.06   Time: 19:20
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * moved check participant stubs to SimpleStorage
 * added user rights check to LDAP conf start with rights cache
 * added LDAP offline user check
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 23.05.06   Time: 15:51
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * fixed non-passing of registry
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 18.05.06   Time: 20:05
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added LDAP group right management
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 31.03.06   Time: 15:37
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added ab and search to LDAP storage
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 30.03.06   Time: 17:12
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * added switchable AB to reg storage
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 9.07.05    Time: 19:29
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * sbs+ mc storage
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 6.07.05    Time: 14:29
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * moved MC storage to separate classs
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 21.09.04   Time: 17:31
 * Updated in $/VS/Servers/MediaBroker/BrokerServices
 * registry storage conferences moved to inherited ConfMem class
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 1.09.04    Time: 18:25
 * Created in $/VS/Servers/MediaBroker/BrokerServices
 * added additional base class
 *
 ****************************************************************************/

/**
* \file VS_ConfMemStorage.h
* Server Database Storage class definition
*
*/
#ifndef VS_SERVER_CONF_MEM_STORAGE_H
#define VS_SERVER_CONF_MEM_STORAGE_H


#include "../../common/std/cpplib/VS_WideStr.h"
#include "VS_EndpointDescription.h"
#include "../../BaseServer/Services/storage/VS_DBStorageInterface.h"

#include "../../common/std/statistics/TConferenceStatistics.h"
#include "SecureLib/VS_Certificate.h"

#include "../../ServerServices/VS_FileConfStat.h"
#include "../../ServerServices/VS_ReadLicense.h"

#include "../../AppServer/Services/ChatDBInterface.h"
#include "ldap_core/common/VS_UserPartEscaped.h"
#include "ldap_core/common/VS_ABStorage.h"
#include "ldap_core/VS_GroupManager.h"
#include "std-generic/cpplib/synchronized.h"

#include "std-generic/compat/map.h"
#include <set>
#include <boost/signals2.hpp>

class CppSQLite3DB;
class VS_RegistryKey;
class VS_TranscoderLogin;

/**
*  \brief Abstract class for encapsulation of conference storage operations
*
*/



//////////////////////////////////////////////////////////////////////////////////////////////////
/// SimpleStorage class - base for Registy and LDAP storages

extern const char RIGHTS_TAG[];

extern const char ALIASES_KEY[];

extern const char TYPE_TAG[];
extern const char STATUS_TAG[];
extern const char ONLINE_STATUS_TAG[];
extern const char LDAP_STATUS_TAG[];

extern const char MANAGE_AB_TAG[];
extern const bool MANAGE_AB_INIT;

extern const char ENDPOINTS_KEY[];
extern const char PROPERTIES_KEY[];

extern const char OFFLINE_MESS_EXPIRE_DAYS_TAG[];
extern const int OFFLINE_MESS_EXPIRE_DAYS_INIT;


extern const char WAN_IP_TAG[];

extern const char LOGIN_NAME_TAG[];
extern const char DISPLAY_NAME_TAG[];
extern const char HA1_PASSWORD_TAG[];
extern const char PASSWORD_TAG[];
extern const char H323_PASSWORD_TAG_OLD[];
extern const char H323_PASSWORD_TAG_NEW[];

extern const char FIRST_NAME_TAG[];
extern const char LAST_NAME_TAG[];
extern const char COMPANY_TAG[];

namespace callLog {
	class Postgres;
}
namespace regstorage_test {
	struct RegistryStorageTest;
}

static constexpr auto MAX_AVATAR_SIZE(500000);

class VS_SimpleStorage : public VS_DBStorageInterface, public VS_IABSink_GetRegGroupUsers
{
	friend struct regstorage_test::RegistryStorageTest;
	std::unique_ptr<callLog::Postgres>	m_dbCallLog;
	static const char SQLITE_FILE[];
	static const char SQLITE_TABLE[];

	static const char SQLITE_CONF_CONTENT_FILE[];
	static const char SQLITE_CHAT_TABLE[];
	static const char SQLITE_SLIDES_TABLE[];

	static const char SQLITE_TABLE_CALLS[];

	static const char GUEST_PREFIX[];
	static const char GUEST_PREFIX_FIXED_CALLID[];
	static const char GUEST_DISPLAYNAME[];
	static const char SHARED_KEY_2[];
	static const char SHARED_KEY_3[];
	// max chunk
	static const size_t OFFLINE_MESSAGES_CHUNK_SIZE;
	// max offline roaming messages
	static const size_t MAX_OFFLINE_CHAT_MESSAGES;
public:
	static const char LOGGEDUSER_TAG[];
	static const char LASTUSER_TAG[];
	static const char CLIENTVERSION_TAG[];
	static const char APPLICATION_TAG[];
	static const char PROTOCOLVERSION_TAG[];
	static const char LASTCONNECTED_TAG[];

	static const char HASH_PHONES_TAG[];
	static const char PHONE_BOOK_TAG[];
	static const char USER_PHONES_TAG[];
	static const std::string USER_PROPERTIES_TAG;
	static std::string STRING_SEPARATOR;
	typedef VST_StrIMap<VS_StorageUserData> UserMap;

	VS_SimpleStorage(const std::weak_ptr<VS_TranscoderLogin> &transLogin);
	~VS_SimpleStorage();
	size_t GetOfflineMessagesChunkSize(void) const;
	void   SetOfflineMessagesChunkSize(const size_t chunk_size);

	static const char AVATARS_DIRECTORY[];

protected:
	int FindUserPictureImpl(const char* path, int& entries, const std::string& query, long client_hash, VS_Container& cnt);
	static const char EMAIL_TAG[];

	static const char CUSTOM_GROUPS_KEY[];
	static const char GROUP_NAME_TAG[];

	///virtual
	virtual bool GetParticipantLimit(const vs_user_id& user_id, VS_ParticipantDescription::Type type, int& rights, double& limit, double& decLimit) = 0;
	virtual int LoginAsUser(const VS_SimpleStr& login, const VS_SimpleStr& password, const VS_SimpleStr& appID, VS_SimpleStr& autoKey, VS_StorageUserData& user, VS_Container& prop_cnt, const VS_ClientType &client_type = CT_SIMPLE_CLIENT) = 0;
	virtual bool FetchRights(const VS_StorageUserData& user, VS_UserData::UserRights& rights) = 0;
	virtual std::string GetDefauldEditableFields();
	///overrided for derived
	bool Init(const VS_SimpleStr& broker_id) override;
	void Timer(unsigned long ticks, VS_TransportRouterServiceHelper* caller) override;
	bool LogParticipantLeave(const VS_ParticipantDescription& pd) override;
	void SetUserStatus(const VS_SimpleStr& call_id, int status, const VS_ExtendedStatusStorage &extStatus, bool set_server, const VS_SimpleStr& server) override;
	int FindUsers(VS_Container& cnt, int& entries, VS_AddressBook ab, const vs_user_id& owner, const std::string& query, long client_hash, VS_Container& in_cnt) override;
	bool OnPropertiesChange(const char* pass) override;
	void CleanUp() override;
	int  GetAppProperties(VS_Container& prop, const VS_SimpleStr& app_name) override;
	bool GetAppProperty(const VS_SimpleStr& app_name, const VS_SimpleStr& prop_name, VS_SimpleStr& value) override;
	bool GetAppProperty(const VS_SimpleStr& /*app_name*/, const VS_SimpleStr& /*prop_name*/, VS_WideStr& /*value*/) override
	{
		return false;
	};
	/// internal for derived
	void SaveAutoLoginKey(const std::string& user_id, const char* appID, const char* autoKey);
	void SaveAutoLoginKey(const char* user_id, const char* appID, const char* autoKey);
	void OnUserLoggedInAtEndpoint(const VS_UserData* ud);
	bool FindGuest(const vs_user_id& id, VS_UserData& user);
	void SearchUsersByAlias(const char* email, const char* name, std::map<VS_SimpleStr, VS_StorageUserData> &result);
	void AddServerIPAsAlias(VS_UserData& ud) const;
	void FindUsersPhones(const vs_userpart_escaped& owner, std::vector<VS_UserPhoneItem> &v);
	std::string GetUserPhoneRegPath(const vs_userpart_escaped& call_id);
	int32_t GetPhoneBookHash(const vs_userpart_escaped& call_id);
	void UpdateUsersPhonesHash_Global();
	void GetSystemGroups(const char* owner, VS_Container& rCnt, int& entries);
	long CalcABGroupsHash(const std::string & owner);
	void GetABGroupsOfUser(const std::string & owner, VS_Container & cnt, int & entries);
	void GetCustomGroups(const char* owner, VS_Container& cnt, int& entries);
	void GetMissedCallMailTemplateBase(std::string &subj_templ, std::string &body_templ);
	void GetInviteCallMailTemplateBase(std::string &subj_templ, std::string &body_templ);
	void GetMultiInviteMailTemplateBase(std::string &subj_templ, std::string &body_templ);
	void GetMissedNamedConfMailTemplateBase(std::string &subj_templ, std::string &body_templ);
	bool CheckGuestUserPassword(const char* password, const char* user_id);
	bool TryTerminalLogin(const VS_TempUserData &user, const char *user_id, const VS_SimpleStr &password);
	void FetchTarifOpt(VS_UserData& user);
	bool ParseQuery(const std::string& query, VS_StorageUserData& params);
	virtual int FindUserPicture(VS_Container& cnt, int& entries, VS_AddressBook ab, const std::string& query, long client_hash);
	virtual int SetUserPicture(VS_Container &cnt, VS_AddressBook ab, const char *callId, long &hash);
	virtual int DeleteUserPicture(VS_AddressBook ab, const char *callId, long &hash);
	void CheckLic_TerminalPro(bool &IsLoginAllowed, bool &IsTerminalPro);
	void ResetLicenseChecker(LicenseCheckFunctionT && f);
	void GetServerIPAsAliasPostfix(std::set<std::string>& v) const;
	void ReadApplicationSettings(VS_StorageUserData &user);

	VS_Lock			m_appProps_lock;
	VS_Container	m_appProperties;
	VS_ABStorage*	m_ab_storage;
	vs::Synchronized<UserMap, std::recursive_mutex>			m_users;
	VS_SimpleStr	m_broker_id;
	VS_SimpleStr	m_secret2;		// for guest password with prefix $2
	VS_Certificate	m_srvCert;
	VS_SimpleStr	m_session_id;
	std::shared_ptr<VS_GroupManager> m_group_manager;
	std::vector<std::pair<std::string, std::string>> m_picture_mime_types;	// [ext,mime]
	LicenseCheckFunctionT m_license_checker;
private:
	/// overrided
	// Get HA1 from Registry and check input values for correctness
	bool CheckDigestByRegistry(const VS_SimpleStr& login, const VS_SimpleStr& password) override;
	void ProcessConfStat(VS_Container& cnt) override;
	void GetRoamingOfflineMessages(const char* our_sid, VS_ChatMsgs& v) override;
	int GetOfflineChatMessages(const char* call_id, std::vector<VS_Container> &vec) override;
	bool SetOfflineChatMessage(const VS_SimpleStr& from_call_id, const VS_SimpleStr& to_call_id, const VS_SimpleStr& body_utf8, const std::string& from_dn, const VS_Container &cont) override;
	void DeleteOfflineChatMessage(VS_Container& cnt) override;
	int GetNamedConfInfo(const char* conf_id, VS_ConferenceDescription& cd, ConferenceInvitation& ci, VS_SimpleStr& as_server, long& scope) override;
	int UpdateNamedConfInfo_RTSPAnnounce(const VS_ConferenceDescription& cd, string_view announce_id) override;
	bool ManageGroups_CreateGroup(const vs_user_id& owner, const VS_WideStr& gname, long& gid, long& hash) override;
	bool ManageGroups_DeleteGroup(const vs_user_id& owner, const long gid, long& hash) override;
	bool ManageGroups_RenameGroup(const vs_user_id& owner, const long gid, const VS_WideStr& gname, long& hash) override;
	bool ManageGroups_AddUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash) override;
	bool ManageGroups_DeleteUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash) override;
	bool CheckSessionID(const char *password) override;
	const char* GetSessionID() const override;
	int AddToAddressBook(VS_AddressBook ab, const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt, VS_SimpleStr& add_call_id, std::string& add_display_name, VS_TransportRouterServiceHelper* srv) override;
	int RemoveFromAddressBook(VS_AddressBook ab, const vs_user_id& user_id1, const vs_user_id& user_id2, VS_Container &cnt, long& hash, VS_Container &rCnt) override;
	int UpdateAddressBook(VS_AddressBook ab, const vs_user_id& user_id1, const char* call_id2, VS_Container& cnt, long& hash, VS_Container& rCnt) override;
	int GetAllAppProperties(VS_AppPropertiesMap &prop_map) override;
	bool SetAllEpProperties(const char* app_id, const int prot_version, const short int type, const wchar_t* version, const /*char*/wchar_t *app_name, const /*char*/wchar_t* sys_conf, const /*char*/wchar_t* processor, const /*char*/ wchar_t *directX, const /*char*/wchar_t* hardwareConfig, const /*char*/wchar_t* AudioCapture, const /*char*/wchar_t *VideoCapture, const /*char**/wchar_t* AudioRender, const char* call_id) override;
	//logging
	bool	LogConferenceStart(const VS_ConferenceDescription& conf, bool remote = false) override;
	bool	LogConferenceEnd(const VS_ConferenceDescription& conf) override;
	bool	LogParticipantInvite(const vs_conf_id& conf_id, const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,
		const std::chrono::system_clock::time_point time = std::chrono::system_clock::time_point(), VS_ParticipantDescription::Type type = VS_ParticipantDescription::PRIVATE_HOST) override;
	bool	LogParticipantJoin(const VS_ParticipantDescription& pd, const VS_SimpleStr& callid2 = NULL) override;
	bool	LogParticipantReject(const vs_conf_id& conf_id, const vs_user_id& user, const vs_user_id& invited_from, const VS_Reject_Cause cause) override;
	bool	LogParticipantStatistics(const VS_ParticipantDescription& pd) override;
	bool	LogConferenceRecordStart(const vs_conf_id& conf_id, const char* filename, std::chrono::system_clock::time_point started_at) override;
	bool	LogConferenceRecordStop(const vs_conf_id& conf_id, std::chrono::system_clock::time_point stopped_at, uint64_t file_size) override;
	bool	LogSystemParams(std::vector<int> &params) override;
	bool	LogParticipantDevices(VS_ParticipantDeviceParams& params) override;
	bool	LogParticipantRole(VS_ParticipantDescription& params) override;
	long	SetRegID(const char* call_id, const char* reg_id, VS_RegID_Register_Type type) override;
	bool LogSlideShowCmd(const char *confId, const char *from, const char *url, const char *mimeType, size_t slideIndex, size_t slidesCount, const char *about,
		size_t width, size_t height, size_t size) override;
	bool LogSlideShowEnd(const char *confId, const char *from) override;
	bool LogGroupChat(const char *confId, const char *from, const char *text) override;
	bool	UpdateConferencePic(VS_Container &cnt) override;
	void GetBSEvents(std::vector<BSEvent> &vec) override;
	int LoginUser(const VS_SimpleStr& login, const VS_SimpleStr& password, const VS_SimpleStr& appID, VS_SimpleStr& autoKey, const VS_SimpleStr& appServer, VS_UserData& user, VS_Container& prop_cnt, const VS_ClientType& client_type = CT_SIMPLE_CLIENT) override;
	bool DeleteUser(const vs_user_id& id) override;
	bool IsOperator(const vs_user_id& user) override;
	bool GetServerTime(std::chrono::system_clock::time_point& ft) override;
	bool SetEndpointProperty(const char* ep_id, const char* name, const char* value) override;
	bool SetEndpointProperty(const char* ep_id, const char* name, const wchar_t* value) override;
	bool GetEndpointProperty(const VS_SimpleStr& ep_id, const VS_SimpleStr& _name, VS_SimpleStr& value) override;
	bool GetExtendedStatus(const char* call_id, VS_ExtendedStatusStorage & extStatus) override;
	void GetUpdatedExtStatuses(std::map<std::string, VS_ExtendedStatusStorage>&) override;
	boost::signals2::connection Connect_AliasesChanged(const AliasesChangedSlot &slot) override;
	vs_conf_id NewConfID() override;
	///properties
	///application
	bool SetAppProperty(const VS_SimpleStr& prop_name, const VS_SimpleStr &value) override;
	///server
	bool SetServerProperty(const VS_SimpleStr& name, const VS_SimpleStr& value) override;
	bool GetServerProperty(const std::string& name, std::string& value) override;
	bool GetWebManagerProperty(const VS_SimpleStr& name, std::string& value) override;
	bool GetWebManagerProperty(const VS_SimpleStr& name, unsigned long& value) override;
	//user
	bool GetUserProperty(const vs_user_id& /*user_id*/, const std::string& name, std::string& value) override {
		return GetServerProperty(name, value);
	};
	//common ResolveAlias function
	void UpdateAliasList() override;
	void UpdateUsersGroups(const std::function<bool(void)>& is_stopping) override;
	/// internal methods
	bool ConfContent_InitSQLiteDB(CppSQLite3DB& db, const char *conf_id);
	bool TryAddColumn(CppSQLite3DB &db, const char *table, const char* column, const char *column_type);
	//bool GetServerProperty(const VS_SimpleStr& name, VS_WideStr& value);
public:
	static bool GetGuestParams(const char* guest_login, VS_SimpleStr& fixed_login, VS_SimpleStr& displayName_utf8);
	static std::string EscapeCallId(string_view callId);
private:
	void UpdatePhoneBookHash(const vs_userpart_escaped& call_id);
	long GetUsersPhonesHash_Global();
	long GetUsersPhonesHash_User(const vs_userpart_escaped& call_id);
	void SetUsersPhonesHash_User(const vs_userpart_escaped& call_id, long global_hash);
	void GetPropsFromRegByName(VS_RegistryKey& key, boost::shared_ptr<VS_Container> cnt);
	bool FindUser_Sink(const vs_user_id& user_id, VS_StorageUserData& ude, bool);
	bool IsLDAP_Sink() const;
	int AddToAddressBook_Phones(const vs_userpart_escaped& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt);
	int RemoveFromAddressBook_Phones(const vs_userpart_escaped& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt);
	int UpdateAddressBook_Phones(const vs_userpart_escaped& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt);
	bool ResolveCallIDByAlias(const VS_SimpleStr& alias, vs_user_id& call_id);
	bool ResolveAliasByCallID(const vs_user_id& call_id, std::vector<std::string>& v_alias);
	int LoginAsGuest(const vs_user_id& login_requested, const VS_SimpleStr& display_name, const VS_SimpleStr& password, const VS_SimpleStr& appID, VS_UserData& user, const VS_ClientType &client_type = CT_SIMPLE_CLIENT);
	int LoginAsTranscoder(const VS_SimpleStr& login, const VS_SimpleStr& password, const VS_SimpleStr& appID, VS_UserData& user, const VS_ClientType &client_type = CT_SIMPLE_CLIENT);
	// Digest authentication for SIP.
	// password format: $4*nonce*ha2*response
	bool TryDigestLogin(const VS_TempUserData &user, const VS_SimpleStr &password);
	// For H225 RAS RRQ message from external h323 terminal.
	// Format: $5*[alias1_size*alias1*timestamp1*md5_token1][alias1_size*alias1*timestamp1*md5_token1]...
	bool TryH323Login(const VS_TempUserData &user, const VS_SimpleStr &password);
	///2 participant function wrappers
	bool CheckTempPassword(const char* password, const char* prefix, const char* login, const char*fixed_id, const char* secret);
	bool CheckGuestPassword(const char* fixed_id, const VS_SimpleStr& password, const VS_ClientType &type);
	bool CheckTranscoderPassword(const VS_SimpleStr& call_id, const VS_SimpleStr &pass);
	void Timer_OfflineChatCleanup();
	void CleanExtStatuses(const char *user_name);
	bool      Read(VS_RegistryKey &reg_ep, VS_EndpointDescription& ep);
	std::string GetUserPropertyPath(const char * call_id);
	bool GetUserProperty(const vs_user_id & user_id, const VS_SimpleStr & name, int64_t & value);

	//types
	typedef VST_StrMap<VS_SimpleStr> UserEpMap;
	///members
	bool m_ab_manage;
	std::map<std::string, boost::shared_ptr<VS_Container>>	m_appPropertiesByName;
	VS_SimpleStr			m_lastEndpointName;
	std::string				m_last_write;
	vs::Synchronized<VS_StrIStrMap,vs::fast_mutex>    m_aliasList;
	int				m_money_warn_time;		//const
	int				m_money_warn_period;	//const
	int				m_money_warn_send_time; //var
	int				m_tick;					//var
	VS_SimpleStr	m_secret3;	// for guest password with prefix $3
	size_t m_skip;
	size_t m_chunk_size;
	std::map<VS_SimpleStr, VS_SimpleStr> m_additional_app_properties;
	AliasesChangedSignal m_fireAliasesChanged;
	unsigned long	m_Timer_OfflineChatMessages;
	int				m_offline_mess_expire_days;
	std::unique_ptr<ChatDBInterface> m_chatDB;
	std::weak_ptr<VS_TranscoderLogin> m_transLogin;

	mutable vs::Synchronized<vs::map<vs_conf_id, std::chrono::steady_clock::time_point>> m_conf_started;
	unsigned long GetConfStartTimeDiffOrDefault(const char* conf_id) const;
	void ResetDBCallLog(std::unique_ptr<callLog::Postgres> &&new_calllog);
	callLog::Postgres* GetCallLog() const;
};

#endif //VS_SERVER_CONF_MEM_STORAGE_H
