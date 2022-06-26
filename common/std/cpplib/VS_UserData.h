/*************************************************
 * $Revision: 46 $
 * $History: VS_UserData.h $
 *
 * *****************  Version 46  *****************
 * User: Ktrushnikov  Date: 13.08.12   Time: 13:21
 * Updated in $/VSNA/Servers/ServerServices/types
 * - flag value fixed (UR_COMM_CHANGEPASSWORD=0x00200000)
 * - return type fixed
 * - dprint fixed
 *
 * *****************  Version 45  *****************
 * User: Ktrushnikov  Date: 6.08.12    Time: 19:29
 * Updated in $/VSNA/Servers/ServerServices/types
 * - added: UR_COMM_CHANGEPASSWORD	= 0x40000000
 * - added to enum: VS_ReqUpdateAccountType
 *
 * *****************  Version 44  *****************
 * User: Mushakov     Date: 2.11.11    Time: 20:38
 * Updated in $/VSNA/servers/serverservices/types
 *  - ClientTransport. Send Mess without gate ability
 *
 * *****************  Version 43  *****************
 * User: Ktrushnikov  Date: 8/25/11    Time: 9:33p
 * Updated in $/VSNA/Servers/ServerServices/types
 * bitrix
 * - IsOurSID() for guests return true;
 *
 * *****************  Version 42  *****************
 * User: Ktrushnikov  Date: 31.07.11   Time: 12:36
 * Updated in $/VSNA/Servers/ServerServices/types
 * bitrix: display_name of guests in Trial mode
 *
 * *****************  Version 41  *****************
 * User: Smirnov      Date: 27.04.11   Time: 14:18
 * Updated in $/VSNA/Servers/ServerServices/types
 * - HD video right
 *
 * *****************  Version 40  *****************
 * User: Ktrushnikov  Date: 24.03.11   Time: 15:23
 * Updated in $/VSNA/Servers/ServerServices/types
 * LDAP: server crash when login with user, that is in conf now on other
 * PC [Beagle]
 * - check for null in LDAPStorage::FindUser()
 *
 * *****************  Version 39  *****************
 * User: Ktrushnikov  Date: 17.03.11   Time: 19:35
 * Updated in $/VSNA/Servers/ServerServices/types
 * new style of system groups
 * - implementation (alfa)
 * - FindUserExtended() renamed to overloaded FindUser()
 * - OnUserChange(): type==4 supported
 *
 * *****************  Version 38  *****************
 * User: Ktrushnikov  Date: 27.01.11   Time: 13:40
 * Updated in $/VSNA/Servers/ServerServices/types
 * VCS 3.2
 * - offline chat messages in roaming
 * - on of SubscriptionHub::Subscribe() fixed
 *
 * *****************  Version 37  *****************
 * User: Ktrushnikov  Date: 8.11.10    Time: 12:40
 * Updated in $/VSNA/Servers/ServerServices/types
 * [#8067]
 * - don't show & RemoveDups() users in AB with @server_name
 * - update AB for short name
 * - Sleep() added again
 *
 * *****************  Version 36  *****************
 * User: Ktrushnikov  Date: 2.11.10    Time: 10:42
 * Updated in $/VSNA/Servers/ServerServices/types
 * #3569:
 * - vs_bc.dll: send to right place; send call_id with server_name
 * - VS_RegistryStorage: send to client UPDATEACCOUNT_METHOD when
 * OnUserChange() with type=1
 * - VS_RegistryStorage: FetchRights at Read()
 * #7454:
 * - Sleep() removed
 *
 * *****************  Version 35  *****************
 * User: Mushakov     Date: 15.10.10   Time: 16:27
 * Updated in $/VSNA/Servers/ServerServices/types
 * - defaultDomain added
 *
 * *****************  Version 34  *****************
 * User: Ktrushnikov  Date: 11.10.10   Time: 14:27
 * Updated in $/VSNA/Servers/ServerServices/types
 * #7924: VIDEORECORDING bit in Rights by License
 * #7923: TarifRestrictions in VCS
 * - restrick in VCSConfRestrick
 * - send TARIFRESTR_PARAM to client
 *
 * *****************  Version 33  *****************
 * User: Ktrushnikov  Date: 25.08.10   Time: 22:29
 * Updated in $/VSNA/Servers/ServerServices/types
 * pass properties from BS to user via AS at login using "cnt in cnt"
 *
 * *****************  Version 32  *****************
 * User: Ktrushnikov  Date: 10.08.10   Time: 21:29
 * Updated in $/VSNA/Servers/ServerServices/types
 * [#7561]: GenerateSessionKey() at LoginUser and at ReqUpdateAccount()
 * [#7562]: update login session key at timer timeout in Visicron.dll
 *
 * *****************  Version 31  *****************
 * User: Ktrushnikov  Date: 13.07.10   Time: 22:02
 * Updated in $/VSNA/Servers/ServerServices/types
 * Arch 3.1: GroupConfLimits
 * - tarif flags for conf & user added
 * - processing tarif restrictions at server
 * - set rights by  tarif at login
 * - support tarif for namedconfs
 *
 * *****************  Version 30  *****************
 * User: Smirnov      Date: 29.04.10   Time: 14:34
 * Updated in $/VSNA/Servers/ServerServices/types
 * - added right for desktop sharing
 *
 * *****************  Version 29  *****************
 * User: Ktrushnikov  Date: 16.04.10   Time: 16:57
 * Updated in $/VSNA/Servers/ServerServices/types
 * - email added as alias
 * - ABstorage: Resolve added
 *
 * *****************  Version 28  *****************
 * User: Ktrushnikov  Date: 16.04.10   Time: 10:39
 * Updated in $/VSNA/Servers/ServerServices/types
 * - unnecessary cycle removed
 *
 * *****************  Version 27  *****************
 * User: Ktrushnikov  Date: 16.04.10   Time: 10:33
 * Updated in $/VSNA/Servers/ServerServices/types
 * don't add m_serverName if input in format: user@server.name
 *
 * *****************  Version 26  *****************
 * User: Ktrushnikov  Date: 5.04.10    Time: 21:50
 * Updated in $/VSNA/Servers/ServerServices/types
 * - check for null [#7197]
 *
 * *****************  Version 25  *****************
 * User: Ktrushnikov  Date: 5.04.10    Time: 20:44
 * Updated in $/VSNA/Servers/ServerServices/types
 * - Trial Storage AB, statises fixed
 *
 * *****************  Version 24  *****************
 * User: Ktrushnikov  Date: 1.04.10    Time: 13:40
 * Updated in $/VSNA/Servers/ServerServices/types
 * [bug #7179]
 * - support for user@server.name
 * - VS_RealUserLogin::GetUser() added
 * - OnUserChange() type == 3 deleted
 *
 * *****************  Version 23  *****************
 * User: Ktrushnikov  Date: 29.03.10   Time: 21:35
 * Updated in $/VSNA/Servers/ServerServices/types
 * - deadlock fixed at VS_RealUserLogin::operator=()
 *
 * *****************  Version 22  *****************
 * User: Ktrushnikov  Date: 22.03.10   Time: 19:03
 * Updated in $/VSNA/Servers/ServerServices/types
 * - save users online_status
 * - remove key at startup
 * - type == 3 for OnUpdateUser
 *
 * *****************  Version 21  *****************
 * User: Mushakov     Date: 19.03.10   Time: 17:59
 * Updated in $/VSNA/Servers/ServerServices/types
 *  - ClientType is received from client
 * - Endpoint function removed from code
 * - arr_key logged inRegistryServer
 *
 * *****************  Version 20  *****************
 * User: Mushakov     Date: 26.02.10   Time: 15:48
 * Updated in $/VSNA/Servers/ServerServices/types
 *  - mobile client supported (AuthService)
 * - new cert
 *
 * *****************  Version 19  *****************
 * User: Mushakov     Date: 8.12.09    Time: 20:23
 * Updated in $/VSNA/Servers/ServerServices/types
 *  - ConfRestriction Interface added
 *
 * *****************  Version 18  *****************
 * User: Ktrushnikov  Date: 2.12.09    Time: 18:50
 * Updated in $/VSNA/servers/serverservices/types
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
 * *****************  Version 17  *****************
 * User: Mushakov     Date: 2.11.09    Time: 17:07
 * Updated in $/VSNA/Servers/ServerServices/types
 *
 * *****************  Version 16  *****************
 * User: Mushakov     Date: 2.11.09    Time: 15:03
 * Updated in $/VSNA/Servers/ServerServices/types
 * - store users by login only
 *
 * *****************  Version 15  *****************
 * User: Ktrushnikov  Date: 15.09.09   Time: 16:39
 * Updated in $/VSNA/Servers/ServerServices/types
 * - Rating added
 * - display_name type changed to char* (from wchar_t)
 * - send BS Events to different service (AUTH,FWD)
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 3.09.09    Time: 18:01
 * Updated in $/VSNA/servers/serverservices/types
 * - PRO restrictions
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 7.08.09    Time: 15:14
 * Updated in $/VSNA/Servers/ServerServices/types
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 4.05.09    Time: 19:07
 * Updated in $/VSNA/Servers/ServerServices/types
 * - cache for login
 *
 * *****************  Version 11  *****************
 * User: Ktrushnikov  Date: 31.03.09   Time: 20:34
 * Updated in $/VSNA/Servers/ServerServices/types
 * - m_group added
 *
 * *****************  Version 10  *****************
 * User: Ktrushnikov  Date: 26.03.09   Time: 19:25
 * Updated in $/VSNA/Servers/ServerServices/types
 * - Initial checkin SBSv3
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 3.02.09    Time: 12:49
 * Updated in $/VSNA/Servers/ServerServices/types
 * - comma added;
 * - Msg log AV fixed
 * - some logs removed (RS)
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 2.02.09    Time: 17:27
 * Updated in $/VSNA/Servers/ServerServices/types
 * - update rights supported
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 30.10.08   Time: 20:33
 * Updated in $/VSNA/Servers/ServerServices/types
 *  - Alias status sending removed (AS ->RS )
 * - Logging of subType added
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 18.02.08   Time: 21:43
 * Updated in $/VSNA/Servers/ServerServices/types
 * added alias support (first ver)
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 12.02.08   Time: 20:31
 * Updated in $/VSNA/Servers/ServerServices/types
 * passing aliases to as
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 5.02.08    Time: 16:42
 * Updated in $/VSNA/Servers/ServerServices/types
 * - direct conf half-repaired
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 22.11.07   Time: 20:34
 * Updated in $/VSNA/Servers/ServerServices/types
 * new iteration
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 15.11.07   Time: 15:28
 * Updated in $/VSNA/Servers/ServerServices/types
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 13.11.07   Time: 17:32
 * Created in $/VSNA/Servers/ServerServices/types
 * new services
 *
 *************************************************/

#ifndef VS_USERDATA_H
#define VS_USERDATA_H

#include "VS_ExternalAccount.h"
#include "VS_SimpleStr.h"
#include "VS_WideStr.h"
#include "VS_UserID.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std-generic/clib/strcasecmp.h"
#include "VS_MapTpl.h"
#include "VS_Protocol.h"
#include "VS_Utils.h"
#include <vector>
#include <typeinfo>
#include <chrono>
#include <map>

extern std::string g_tr_endpoint_name;

class VS_UserData
{
public:
	vs_user_id		m_name;
	std::string		m_callId;
	std::string		m_displayName;

	long			m_type;

	VS_ClientType	m_client_type;

	long			m_rights;
	VS_SimpleStr	m_homeServer;
	VS_StrI_IntMap	m_aliases;

	long			m_rating;
	long			m_SeparationGroup;

	// application data
	unsigned long	m_protocolVersion;
	VS_SimpleStr	m_appName;
	VS_SimpleStr	m_appID;
	VS_BinBuff		m_IPconfig;

	struct SettingsValue {
		int32_t integer;	// "Value"
		bool IsLocked;		// "IsLocked"
	};
	std::map<std::string, SettingsValue> m_appSettings;

	enum UserStatus
	{
		US_NONE		= 0x00000000,
		US_LOGIN	= 0x00000001
	};

	enum UserRights
	{
		UR_APP_MASK				=	0xf,
		UR_APP_COMMUNICATOR		= 0x1,
		UR_APP_ADVISOR			= 0x2,
		UR_NONE					= 0x00000000,
		UR_LOGIN				= 0x00000010,
		UR_COMM_MULTI			= 0x00000020,
		UR_COMM_PASSWORDMULTI	= 0x00000040,
		UR_COMM_CREATEMULTI		= 0x00000080,
		UR_COMM_SEARCHEXISTS	= 0x00000100,
		UR_COMM_EDITAB			= 0x00000200,
		UR_COMM_BROADCAST		= 0x00000400,
		UR_COMM_FILETRANSFER	= 0x00000800,
		UR_COMM_WHITEBOARD		= 0x00001000,
		UR_COMM_SLIDESHOW		= 0x00002000,
		UR_COMM_UPDATEAB		= 0x00004000,
		UR_COMM_CALL    		= 0x00008000,
		UR_COMM_APPCALLLOG		= 0x00010000,
		UR_COMM_DSHARING        = 0x00020000,
		UR_COMM_RECORDING       = 0x00040000,
		UR_COMM_EDITGROUP       = 0x00080000,
		UR_COMM_HDVIDEO         = 0x00100000,
		UR_COMM_CHANGEPASSWORD	= 0x00200000,
		UR_COMM_DIALER			= 0x00400000,
		UR_COMM_EDITDIAL		= 0x00800000,
		UR_COMM_CHAT			= 0x01000000,
		UR_COMM_SHARE_CONTROL	= 0x02000000,

		UR_COMM_MOBILEPRO		= 0x10000000,
		UR_COMM_PROACCOUNT		= 0x20000000
	};

	enum UserTypes
	{
		UT_PERSON         = 0 ,
		UT_GUEST		  = 3,
		UT_ALIAS		  = 100
	};

	VS_SimpleStr	m_tarif_name;
	long			m_tarif_restrictions;

	std::vector< VS_ExternalAccount > m_external_accounts;
/*
	bool IsFreeTarif(){ return m_tarif_name == "FREE"; }
	bool IsProTarif(){ return m_tarif_name == "PRO"; }
	bool IsCorpTarif(){ return m_tarif_name == "CORP"; }
*/
	virtual ~VS_UserData(void) {};

	VS_UserData(const char* ID=0,const char *DisplayName=0,long type=-1, long rights=UR_NONE,const char* homeServer=0)
		: m_name(ID), m_type(type), m_client_type(CT_SIMPLE_CLIENT), m_rights(rights), m_homeServer(homeServer), m_rating(0),  m_SeparationGroup(0), m_protocolVersion(0),
		 m_tarif_restrictions(0) {
		if (DisplayName) m_displayName = DisplayName;
	}

	virtual bool IsValid( void ) const {return !!m_name && !m_displayName.empty();}
	bool IsPro() const  {return (m_rights&UR_COMM_PROACCOUNT)!=0;}
};

class VS_TempUserData : public VS_UserData
{
public:

	VS_SimpleStr m_login;
	VS_SimpleStr m_password;
	VS_SimpleStr m_HA1_password;
	VS_SimpleStr m_h323_password;
	VS_SimpleStr m_key;
	VS_SimpleStr m_seq;
	VS_SimpleStr m_defaultDomain;

	unsigned long m_lTime;

	std::vector<std::string> m_groups;

	VS_TempUserData() : m_lTime(0) {}
	virtual ~VS_TempUserData()	{}

	VS_SimpleStr	m_login_session_key;

	VS_BinBuff		m_props;
};

/**
	todo(mu): review this class. server_name has sufix #server_type
	but in logins and user names there is no #suffix. Here it is considered not every where
	It works ok only for our users (suffix #vcs is removed),
	but with foreign servers either login is @serverName#fdfd or server name without #
*/
class VS_RealUserLogin
{
private:
	std::string		m_realLogin;
	bool			m_IsGuest;
public:
	VS_RealUserLogin(string_view login): m_IsGuest(false)
	{
		if (login.empty())
			return;

		m_IsGuest = IsGuest(login);

		if (login.find_last_of('@') != string_view::npos) {
			m_realLogin = std::string(login);
		}
		else
		{
			m_realLogin  = std::string(login);
			m_realLogin +="@";
			m_realLogin += VS_RemoveServerType(g_tr_endpoint_name);
		}
	}
	VS_RealUserLogin(): m_IsGuest(false)
	{
	}

	std::string GetUser() const
	{
		auto str(m_realLogin);
		auto pos = str.find_last_of('@');
		if (pos != std::string::npos)
			str.erase(pos);
		return str;
	}

	string_view GetDomain() const
	{
		if (m_realLogin.empty())
			return {};
		string_view sv = m_realLogin;
		auto pos = sv.find_last_of('@');
		if (pos != string_view::npos) {
			sv.remove_prefix(pos + 1);
			return sv;
		}
		return {};
	}

	const char *GetID() const { return m_realLogin.c_str(); }

	bool IsGuest() const
	{
		return m_IsGuest;
	}

	bool IsOurSID() const
	{
		const auto domain = GetDomain();
		const auto server_domain = VS_RemoveServerType(g_tr_endpoint_name);
		return domain.length() == server_domain.length() && strncasecmp(domain.data(), server_domain.data(), domain.length()) == 0;
	}

	operator const char*() const { return m_realLogin.c_str(); }
	bool operator==(const VS_RealUserLogin& r) const {
		return m_realLogin == r.m_realLogin;
	}
	bool operator==(const char* str) const {
		return m_realLogin == str;
	}
	bool operator<(const VS_RealUserLogin& r) const
	{
		return m_realLogin < r.m_realLogin;
	}

	static bool IsGuest(string_view login) {
		return login.length() > 6 && !strncasecmp(login.data(), "#guest", 6);
	}
};

enum VS_UserPhoneType {
	USERPHONETYPE_INVALID = 0,
	USERPHONETYPE_MOBILE,
	USERPHONETYPE_WORK,
	USERPHONETYPE_HOME,
	USERPHONETYPE_OTHER
};

struct VS_UserPhoneItem
{
	VS_SimpleStr		id;
	vs_user_id			call_id;		// to whom from owner's AB is this entry
	VS_SimpleStr		phone;
	VS_UserPhoneType	type;			// mobile, work, home, other
	bool				editable;		// can edit from client

	VS_UserPhoneItem(): type(USERPHONETYPE_INVALID), editable(false)
	{}
};

class VS_StorageUserData : public VS_TempUserData
{
public:
	VS_RealUserLogin	m_realLogin;

	std::string			m_FirstName;
	std::string			m_LastName;
	int					m_status;
	std::string				m_Company;

	VS_StrIStrMap		m_auto_login_data;

	int					m_online_status;
	VS_SimpleStr		m_email;

	std::vector<VS_UserPhoneItem>		m_phones;

	VS_StorageUserData():m_status(US_NONE), m_online_status(0){}
};

#endif // VS_USERDATA_H
