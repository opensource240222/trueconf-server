/**
****************************************************************************
* (c) 2002-2014 TrueConf
*
* Project: VCS services
*
****************************************************************************/
/**
* \file VS_RegistryStorage.cpp
* Server Registry Storage class implementation functions
*
*/

////////////////////////////////////////////////////////////////////////////////
// Consts
////////////////////////////////////////////////////////////////////////////////


#include "VS_RegistryStorage.h"
#include "VS_ReadOnlyABStorage.h"

#include "ServerServices/VS_ReadLicense.h"
#include "ServersConfigLib/VS_ServersConfigLib.h"
#include "std-generic/cpplib/utf8.h"
#include "std/cpplib/md5.h"
#include "LicenseLib/VS_LicensesWrap.h"
#include "ProtectionLib/Protection.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/VS_Utils.h"
#include "std/clib/vs_defines.h"
#include "TrueGateway/CallConfig/VS_Indentifier.h"
#include "std/VS_RegistryPasswordEncryption.h"

#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>
#include <ctime>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "AppServer/Services/VS_Storage.h"
#include <mutex>

#define DEBUG_CURRENT_MODULE VS_DM_RSTOR

////////////////////////////////////////////////////////////////////////////////
// Consts
////////////////////////////////////////////////////////////////////////////////


// User fields
const char FIRST_NAME_TAG[]	= "First Name";
const char LAST_NAME_TAG[]		= "Last Name";

const char COMPANY_TAG[]			= "Company";
const char USER_GROUP_TAG[]	= "Group";

const char USER_RIGHTS_TAG[]		="User Rights";

const char PASSWORD_TAG[]			= "Password";
const char H323_PASSWORD_TAG_OLD[]		= "H323 Password";
const char H323_PASSWORD_TAG_NEW[] = "H323 Password2";

/// Conference  registry key
const char CONFERENCES_KEY[]		= "Conferences";


VS_RegistryStorage::VS_RegistryStorage(VS_ABStorage* ab_storage, bool useGroups,const VS_SimpleStr& broker_id, const std::weak_ptr<VS_TranscoderLogin> &transLogin)
: VS_SimpleStorage(transLogin), m_hash(0), m_useGroups(useGroups)
{
	m_group_manager = std::make_shared<VS_GroupManager>(false);
	m_ab_storage = ab_storage;
	m_ab_storage->SetSink(this, this);
	if (!Init(broker_id))
		VS_DBStorageInterface::error_code = !error_code? VSS_REGISTRY_ERROR: error_code;
}


VS_RegistryStorage::~VS_RegistryStorage()
{
  if(m_ab_storage)
		delete m_ab_storage;
}
bool VS_RegistryStorage::Init(const VS_SimpleStr& broker_id)
{
	// call base
	VS_SimpleStorage::Init(broker_id);

	std::string buff;
	if (m_srvCert.GetSubjectEntry("organizationName", buff))
		m_Company = buff;
	else
		m_Company.clear();

	if(m_ab_storage->InMemory())
	{
		static_cast<VS_ReadOnlyABStorage*>(m_ab_storage)->SetUsers(&m_users,&m_hash);
	}

	if (!m_ab_storage->IsValid())
	{
		error_code=VSS_AB_STORAGE_ERROR;
		return false;
	}


	// get groups and thier users
	std::map<std::string, std::vector<VS_RealUserLogin>> groups_users;
	GetGroupsUsers(groups_users);


	////////////////////////////////////////////////////////////
	// users
	//	Verify Registry Structure
	VS_RegistryKey	u_root(false, USERS_KEY, false, false);
	VS_RegistryKey	user;
	if (u_root.IsValid())
	{
		std::vector<std::string> to_delete;
		u_root.ResetKey();
		// Enumerate and verify all Users
		while (u_root.NextKey(user))
		{
			auto pName = user.GetName();
			if (!pName) continue;

			VS_StorageUserData	ud;
			VS_RealUserLogin r(vs::UTF8ToLower(pName));
			ud.m_realLogin = r;
			ud.m_login = ud.m_name = r;
			if (!Read(user,ud,groups_users))
				continue;
			// Store user in the cache
			bool success = false;
			if (ud.IsValid())
			{
				UserMap::ConstIterator   ui;
				auto pLockedUsers = m_users.lock();
				ui = pLockedUsers->Find(ud.m_realLogin);
				if (!ui)
				{
					if(ud.m_displayName.empty())
					{
						ud.m_displayName =ud.m_LastName;
						ud.m_displayName+=", ";
						ud.m_displayName+=ud.m_FirstName;
					}
					(*pLockedUsers)[ud.m_realLogin]=ud;
					error_code = 0;
					success = true;
				}
			}
			if (!success) {
				// Remove duplicated data from the Registry
				to_delete.emplace_back(user.GetName());
			}
		}
		for (const auto& x: to_delete)
			u_root.RemoveKey(x);
	}

	m_hash=VS_MakeHash(std::chrono::system_clock::now());

	////////////////////////////////////////////////////////////
	//conferences
	//	Verify Registry Structure
	VS_RegistryKey	c_root(false, CONFERENCES_KEY, false, true);
	if (!c_root.IsValid()) {
		error_code=VSS_REGISTRY_ERROR;
		return false;
	}

  //////////////////////////////////////
  // load default rights

	VS_RegistryKey    cfg(false, CONFIGURATION_KEY, false, true);
	if (!cfg.IsValid()) {
		error_code=VSS_REGISTRY_ERROR;
		return false;
	}

	GetPhysRights();
	if (m_group_manager)
		m_group_manager->UpdateGroupList();
	error_code=0;
	return true;
}

VS_UserData::UserRights VS_RegistryStorage::GetPhysRights()
{
	/**
		Calculate rules,
	*/
	VS_StorageUserData	ud;
	long r = ud.UR_NONE;
//NANOBEGIN;
	VS_License lic_sum = p_licWrap->GetLicSum();
	r = ud.UR_APP_COMMUNICATOR
		|ud.UR_COMM_CALL|ud.UR_COMM_BROADCAST/*|ud.UR_COMM_APPCALLLOG*/|ud.UR_COMM_MOBILEPRO
		|ud.UR_COMM_CHANGEPASSWORD
		|ud.UR_COMM_EDITGROUP
		|ud.UR_COMM_CHAT;

	if(lic_sum.m_conferences>0||lic_sum.m_conferences==lic_sum.TC_INFINITY)
          r |=ud.UR_COMM_MULTI; // Check multiconfs amount in license, set if > 0
	if(m_license_checker(LE_FILETRANSFER))
		r |= ud.UR_COMM_FILETRANSFER;
	if(m_license_checker(LE_WHITEBOARD))
		r |= ud.UR_COMM_WHITEBOARD;
	if(m_license_checker(LE_SLIDESHOW))
		r |= ud.UR_COMM_SLIDESHOW;
	if(m_license_checker(LE_DSHARING))
		r |= (ud.UR_COMM_DSHARING | ud.UR_COMM_SHARE_CONTROL);
	if(m_license_checker(LE_VIDEORECORDING))
		r |= ud.UR_COMM_RECORDING;
	if(m_license_checker(LE_HDVIDEO))
		r |= ud.UR_COMM_HDVIDEO;
	if(m_license_checker(LE_MULTIGATEWAY))
	{
		VS_RegistryKey key(false, CONFIGURATION_KEY);
		char buff[512] = { 0 };
		if (key.GetValue(&buff, 512, VS_REG_STRING_VT, VOIP_GATEWAY_TAG) && buff[0])
			r |= ud.UR_COMM_DIALER;//|ud.UR_COMM_EDITDIAL; removed by SMK: in 3.4.0 disabled
	}


	r|=ud.UR_COMM_PASSWORDMULTI;
	r|=ud.UR_COMM_CREATEMULTI;

	if( m_ab_storage->CanEdit() )
		r|=ud.UR_COMM_EDITAB|ud.UR_COMM_UPDATEAB|ud.UR_COMM_SEARCHEXISTS;

//NANOEND;
	return  m_group_manager->m_physRights = (VS_UserData::UserRights)r;
}

bool VS_RegistryStorage::FindUser(const vs_user_id& id,VS_UserData& user, bool)
{
	if(!id)
	{
		error_code=VSS_USER_NOT_VALID;
		return false;
	}
	VS_RealUserLogin realLogin(SimpleStrToStringView(id));

	auto pLockedUsers = m_users.lock();
	UserMap::ConstIterator i= pLockedUsers->Find(realLogin);
	if(!i)
	{
		error_code=VSS_USER_NOT_FOUND;
		return false;
	}
	user=*(VS_UserData*)(i->data);
	return true;
}

bool VS_RegistryStorage::FindUserByAlias(const std::string& alias, VS_StorageUserData& user)
{
	if (alias.empty())
		return false;
	auto lock = m_users.lock();
	for (auto it = lock->Begin(); it != lock->End(); ++it)
	{
		VS_StorageUserData& ude = *it->data;
		for (auto const& p : ude.m_phones)
		{
			if (alias == p.phone.m_str)
			{
				user = ude;
				return true;
			}
		}
	}

	return false;
}

int  VS_RegistryStorage::LoginAsUser(const VS_SimpleStr& login,const VS_SimpleStr& password, const VS_SimpleStr& appID, VS_SimpleStr& autoKey, VS_StorageUserData& ude, VS_Container& /*prop_cnt*/, const VS_ClientType& client_type)
{
	assert(!!login);

	int result=LICENSE_USER_LIMIT;
	if (!m_license_checker(LE_LOGIN))
		return LICENSE_USER_LIMIT;

	bool IsTerminalLoginAllowed = false;
	bool IsTerminalPro = false;
	if (client_type == CT_TERMINAL)
	{
		CheckLic_TerminalPro(IsTerminalLoginAllowed, IsTerminalPro);
		if (!IsTerminalLoginAllowed)
			return INVALID_CLIENT_TYPE;
	}

	VS_RealUserLogin realLogin(vs::UTF8ToLower(login.m_str));

	try
	{
		if(!FindUser((const char*)realLogin,ude))
			throw (VS_UserLoggedin_Result)VSS_USER_NOT_FOUND;

		ude.m_client_type = client_type;
		ude.m_appID = appID;

		ude.m_aliases.Assign(ude.m_realLogin.GetUser().c_str(),0);

		if (!!ude.m_email)
			ude.m_aliases.Assign(ude.m_email,0);

		AddServerIPAsAlias(ude);

		if (!(ude.m_status&ude.US_LOGIN))
			throw USER_DISABLED;

		if (password) {
			if (!TryTerminalLogin(ude, realLogin.GetUser().c_str(), password)) {
				if (!ude.m_password)
					throw ACCESS_DENIED;
				else if (VS_ClientType::CT_WEB_CLIENT == client_type) {
					if (!m_license_checker(LE_WEBINARS))
						throw ACCESS_DENIED;
					else {
						char md5_hash[256] = { 0 };
						VS_ConvertToMD5(SimpleStrToStringView(password), md5_hash);
						if (strcasecmp(md5_hash, ude.m_password) != 0)
							throw ACCESS_DENIED;
					}
				}
				else {
					if (strcasecmp(password, ude.m_password) != 0)
						throw ACCESS_DENIED;
				}
			}

			assert(!!login);
			if (!!login) {
				MD5 md5;
				md5.Update(SimpleStrToStringView(login));
				md5.Update(std::to_string(rand()));
				md5.Update(std::to_string(time(0)));
				if (m_secret2)
					md5.Update(SimpleStrToStringView(m_secret2)); // used for guest password i.e. can be empty
				md5.Final();

				char md5_hash[33];
				md5.GetString(md5_hash);
				autoKey = ude.m_key = md5_hash;
				ude.m_auto_login_data.Assign(appID, md5_hash);
				SaveAutoLoginKey(ude.m_realLogin.GetUser(), appID, ude.m_key);
			}
		} else {	// AutoLogin
			VS_SimpleStr _key = ude.m_auto_login_data[appID]->data;
			if (!_key || (_key != autoKey))
				throw ACCESS_DENIED;
		}

		FetchRights(ude, (VS_UserData::UserRights&)ude.m_rights);
		if (client_type == CT_TERMINAL && IsTerminalPro)
			ude.m_rights |= VS_UserData::UR_COMM_PROACCOUNT;
		FetchTarifOpt(ude);

		UpdateUserData(ude);
		result=0;
	}
	catch(VS_UserLoggedin_Result error)
	{
		result=error;
	}

	//char buf[512],buf2[128];
	//tu::TimeToLFStr(std::chrono::system_clock::now(), buf2, 128);
	auto stime = tu::TimeToString(std::chrono::system_clock::now(), "%d/%m/%Y:%H:%M:%S %z", true);
#undef DEBUG_CURRENT_MODULE
#define DEBUG_CURRENT_MODULE VS_DM_LOGINS

	dprint0("%s %s %s %s:%d",
		stime.c_str(),
		login.m_str, ude.m_name.m_str, //(const char*)endpoint,
		result == 0 ? "OK" : "ERR", result);

#undef DEBUG_CURRENT_MODULE
#define DEBUG_CURRENT_MODULE VS_DM_RSTOR

	if(result==VSS_USER_NOT_FOUND
      || result==VSS_USER_NOT_VALID)
	  result=ACCESS_DENIED;

	if (result == USER_LOGGEDIN_OK) {
		OnUserLoggedInAtEndpoint(((const VS_UserData*)&ude));
		ReadApplicationSettings(ude);
	}

	return result;
}
int VS_RegistryStorage::FindUsers(VS_Container& cnt, int& entries, VS_AddressBook ab, const vs_user_id& owner, const std::string& query, long client_hash, VS_Container& in_cnt)
{
	entries=0;

	std::lock_guard<decltype(m_common_lock)> lock(m_common_lock);
	if (ab==AB_PERSON_DETAILS && !query.empty()) {
		VS_StorageUserData ud;
		bool found = FindUser(StringViewToSimpleStr(query), ud) && CT_TRANSCODER != ud.m_client_type;
		if (!found) {
			VS_UserData ud_tmp;
			found = g_storage->FindUser(query, ud_tmp);
			(*((VS_UserData*)&ud)) = ud_tmp;
		}
		if (found)
		{
			if (ud.m_displayName.empty() && !ud.m_LastName.empty() && !ud.m_FirstName.empty())
			{
				ud.m_displayName = ud.m_LastName;
				ud.m_displayName+= ", ";
				ud.m_displayName+= ud.m_FirstName;
			}
			cnt.AddValue(USERNAME_PARAM, ud.m_name);
			cnt.AddValue(CALLID_PARAM,   ud.m_name);
			if (!ud.m_displayName.empty())
				cnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
			cnt.AddValue(FIRSTNAME_PARAM, ud.m_FirstName);
			cnt.AddValue(LASTNAME_PARAM, ud.m_LastName);
			cnt.AddValue(USERCOMPANY_PARAM,ud.m_Company);
			auto client_hash = in_cnt.GetLongValueRef(HASH_PARAM);
			auto server_hash = (client_hash) ? *client_hash + 1 : VS_GenKeyByMD5();
			if (server_hash == 0)
				++server_hash;
			cnt.AddValueI32(HASH_PARAM, server_hash);
			entries=1;
		}
		return SEARCH_DONE;
	}

	VS_RealUserLogin r(SimpleStrToStringView(owner));
	if (r.IsGuest()) {
		// only AB_PERSON_DETAILS alowed
		return SEARCH_FAILED;
	}

	if (ab==AB_PHONES) {
		return FindUsersPhones(cnt, entries, owner, client_hash);
	}
	else {
		return VS_SimpleStorage::FindUsers(cnt, entries, ab, owner, query, client_hash, in_cnt);
	}
}


#include "ProtectionLib/OptimizeDisable.h"

bool VS_RegistryStorage::GetParticipantLimit (const vs_user_id& user_id, VS_ParticipantDescription::Type type, int& rights, double& limit, double& decLimit)
{
	bool allow=false;
NANOBEGIN2;
	do {
		VS_StorageUserData ud;

		limit=0.;
		decLimit=0.;
		if(!FindUser(user_id.m_str,ud))
		{
			if( type != VS_ParticipantDescription::PRIVATE_MEMBER
				&&type != VS_ParticipantDescription::MULTIS_MEMBER
				&&type != VS_ParticipantDescription::INTERCOM_MEMBER
				&&type != VS_ParticipantDescription::PUBLIC_MEMBER)
			{
				error_code=VSS_CONF_ACCESS_DENIED;
				break;
			}
		}
		VS_UserData::UserRights urights=ud.UR_NONE;
		FetchRights(ud, urights);

		// set rights
		rights = VS_ParticipantDescription::RIGHTS_NORMAL;

		switch(type)
		{
		case VS_ParticipantDescription::PRIVATE_HOST:
		case VS_ParticipantDescription::HPRIVATE_HOST:
			if(urights&ud.UR_COMM_CALL)
			{
				rights|= VS_ParticipantDescription::RIGHTS_RCV_CHAT;
				rights|= VS_ParticipantDescription::RIGHTS_RCV_LIST;
				limit=1.;
				allow=true;
			}
			else
				error_code=VSS_CONF_ACCESS_DENIED;

			break;

		case VS_ParticipantDescription::PUBLIC_HOST:
			if(urights&VS_UserData::UR_COMM_BROADCAST)
			{
				rights|= VS_ParticipantDescription::RIGHTS_RCV_LIST;
				rights|= VS_ParticipantDescription::RIGHTS_RCV_CHAT;
				limit=1.;
				allow=true;
			}
			else
				error_code=VSS_CONF_ACCESS_DENIED;

			break;

		case VS_ParticipantDescription::PRIVATE_MEMBER:
		case VS_ParticipantDescription::PUBLIC_MEMBER:
		case VS_ParticipantDescription::HPRIVATE_MEMBER:
		case VS_ParticipantDescription::MULTIS_MEMBER:
		case VS_ParticipantDescription::INTERCOM_MEMBER:
			rights|= VS_ParticipantDescription::RIGHTS_RCV_CHAT;
			rights|= VS_ParticipantDescription::RIGHTS_RCV_LIST;
			limit=1.;
			allow=true;
			break;

		case VS_ParticipantDescription::HPRIVATE_GUEST:
			limit=1.;
			allow=true;
			break;

		default:
			limit=1.;
			allow=true;
			break;
		}
	} while (false);
NANOEND2;
	return allow;
}
#include "ProtectionLib/OptimizeEnable.h"

bool VS_RegistryStorage::Read( VS_RegistryKey &reg_user, VS_StorageUserData& user )
{
	std::map<std::string, std::vector<VS_RealUserLogin>> groups_users;
	GetGroupsUsers(groups_users);
	return Read(reg_user,user,groups_users);
}

bool VS_RegistryStorage::Read( VS_RegistryKey &reg_user, VS_StorageUserData& user, std::map<std::string, std::vector<VS_RealUserLogin>>& groups_users )
{
	// Reset
	user.m_displayName.clear();
	user.m_FirstName.clear();
	user.m_LastName.clear();
	user.m_Company.clear();
	user.m_groups.clear();
	user.m_type = 0; user.m_status = user.US_NONE;
	user.m_rights = 0;
	//user.SetPassword(0, 0);
	user.m_password.Empty();
	user.m_HA1_password.Empty();
	user.m_auto_login_data.Clear();

	user.m_phones.clear();


	char buff[1024] = { 0 };
	uint32_t buffSize(0);

    reg_user.GetString(user.m_FirstName, FIRST_NAME_TAG);
    reg_user.GetString(user.m_LastName, LAST_NAME_TAG);
    reg_user.GetString(user.m_displayName, DISPLAY_NAME_TAG);

	if (user.m_displayName.empty())
	{
		user.m_displayName	= user.m_LastName;
		user.m_displayName	+= ", ";
		user.m_displayName	+= user.m_FirstName;
	}

	if (reg_user.GetValue(buff, sizeof(buff), VS_REG_INTEGER_VT, STATUS_TAG) > 0)
		user.m_status = !!*(int*)buff; // convert to UD_LOGIN

	if (reg_user.GetValue(buff, sizeof(buff), VS_REG_INTEGER_VT, LDAP_STATUS_TAG)>0 && (*(int*)buff==1))
		return false;

	memset(buff,0,sizeof(buff));
	if ((buffSize = reg_user.GetValue(buff, sizeof(buff)-1, VS_REG_STRING_VT, PASSWORD_TAG)) > 0)
	{
		user.m_password = (const char*)buff;
		//user.SetPassword((const unsigned char*)buff, buffSize);
	}

	memset(buff,0,sizeof(buff));
	if ((buffSize = reg_user.GetValue(buff, sizeof(buff)-1, VS_REG_STRING_VT, HA1_PASSWORD_TAG)) > 0)
	{
		user.m_HA1_password = (const char*)buff;
	}

	// Password for H323.
	memset(buff,0,sizeof(buff));
	if ((buffSize = reg_user.GetValue(buff, sizeof(buff)-1, VS_REG_STRING_VT, H323_PASSWORD_TAG_NEW)) > 0)
	{
		user.m_h323_password = sec::DecryptRegistryPassword(reg_user.GetName(), buff).c_str();
	}
	else if ((buffSize = reg_user.GetValue(buff, sizeof(buff) - 1, VS_REG_STRING_VT, H323_PASSWORD_TAG_OLD)) > 0)
	{
		user.m_h323_password = (const char*)buff;
	}

	reg_user.GetString(user.m_Company, COMPANY_TAG);

	if (reg_user.GetValue(buff, sizeof(buff), VS_REG_INTEGER_VT, TYPE_TAG) > 0)
		user.m_type= *(int*)buff;

	const char auto_login_tag[] = "\\AutoLogins";
	const char separator = '\\';
	std::string root = USERS_KEY; root +=separator;

	auto login = user.m_realLogin.GetUser();
	root += login;
	root += auto_login_tag;

	VS_RegistryKey logins(false, root);
	logins.ResetValues();
	std::unique_ptr<char, free_deleter> data;
	std::string valueName;

	while (logins.NextValue(data, VS_REG_STRING_VT, valueName)>0) {
		if (data && !valueName.empty()) {
			user.m_auto_login_data.Assign(valueName.c_str(), data.get());
		}
	}

	if ((buffSize = reg_user.GetValue(buff, sizeof(buff)-1, VS_REG_STRING_VT, EMAIL_TAG)) > 0)
		user.m_email = (const char*)buff;

	if (user.m_Company.empty())
		user.m_Company = m_Company;

	for(auto it=groups_users.begin(); it!=groups_users.end(); ++it)
	{
		if (std::find(it->second.begin(),it->second.end(),user.m_realLogin) != it->second.end())
		{
			user.m_groups.push_back(it->first);
		}
	}

	root = USERS_KEY; root+=separator;
	root += login; root += USER_PHONES_TAG;
	VS_RegistryKey phones_root(false, root);
	phones_root.ResetKey();
	VS_RegistryKey  phone_item;
	while (phones_root.NextKey(phone_item))
	{
		VS_UserPhoneItem item;
		if (phone_item.IsValid())
		{
			const unsigned int str_len = 1024;
			char phone_buff[str_len];
			long type = 0;

			if ((phone_item.GetValue(&type, sizeof(type), VS_REG_INTEGER_VT, TYPE_PARAM) <= 0) ||
				(phone_item.GetValue(phone_buff, str_len, VS_REG_STRING_VT, USERPHONE_PARAM) <= 0))
				continue;

			item.id = root.c_str();			// "Users\a\UserPhones"
			item.id += "\\";
			item.id += phone_item.GetName();

			item.call_id = user.m_realLogin;
			item.phone = phone_buff;

			item.type = (VS_UserPhoneType)type;
			item.editable = false;

			user.m_phones.push_back(item);
		}
	}

	FetchRights(user, (VS_UserData::UserRights&) user.m_rights);
	//// TODO: temp hack. We need to properly handle rights on client side (if not - no video), then remove
	FetchTarifOpt(user);

	return true;
}

void VS_RegistryStorage::SetUserStatus(const VS_SimpleStr& call_id,int status, const VS_ExtendedStatusStorage &extStatus, bool set_server,const VS_SimpleStr& server)
{
	VS_SimpleStorage::SetUserStatus(call_id, status, extStatus, set_server, server);

	VS_StorageUserData ude;
	if (!FindUser(call_id.m_str, ude))
		return ;

	ude.m_online_status = status;
	UpdateUserData(ude);
}


//----------------------------------------------------------------------------------
bool VS_RegistryStorage::OnUserChange(const char* user, long type, const char* pass)
{
	if (m_session_id!= pass)
		return false;
	if(!user || !(*user))
		return false;

	VS_RealUserLogin r(user);

	UserMap::Iterator i;
	LoginMap::Iterator     Li;
	std::string key_name = USERS_KEY; key_name += "\\"; key_name += r.GetUser();
	VS_RegistryKey key(false, key_name, false);
	VS_StorageUserData ud;
	{auto pLockedUsers = m_users.lock();
	if(type == 0)
	{
		if (!key.IsValid())
			return false;
		ud.m_realLogin = r;
		ud.m_login = ud.m_name = r;
		Read(key, ud);
		if (!ud.IsValid())
			return false;
		i = pLockedUsers->Find(ud.m_realLogin);
		if(!!i)
			return false;
		pLockedUsers->Insert(ud.m_realLogin, &ud);
	}
	else if(type == 1)
	{
		if (!key.IsValid())
			return false;
		ud.m_realLogin = r;
		ud.m_login = ud.m_name = r;
		Read(key, ud);
		if (!ud.IsValid()) return false;
		i = pLockedUsers->Find(ud.m_realLogin);
		if (!i) return false;
		VS_StorageUserData *pud = i->data;
		*pud = ud;
	}
	else if(type == 2){
		pLockedUsers->Erase(r);
	}
	}

	if(m_ab_storage->InMemory())
	{
		m_hash=VS_MakeHash(std::chrono::system_clock::now());
	}
	return true;
}

bool VS_RegistryStorage::LogParticipantLeave (const VS_ParticipantDescription& pd)
{
	VS_SimpleStorage::LogParticipantLeave(pd);
#if 0
	VS_SimpleStr tmp(1024);
	VS_SimpleStr tmp2(1024);
	VS_EndpointDescription ed;
	VS_FileTime leaveTime; leaveTime.Now();
	int duration = (int)((leaveTime - pd.m_joinTime)/600000000I64);
#endif

	return true;
}



int VS_RegistryStorage::GetUserRights(const vs_user_id& id)
{
	VS_UserData::UserRights rights=VS_UserData::UR_NONE;
	VS_StorageUserData ud;

	if (FindUser(id.m_str, ud))
	{
		FetchRights(ud,rights);
	}
	dprint3("R$GetUserRights for %s returned %04x\n",(const char*)id,rights);
	return rights;
}


/// ab sink
bool VS_RegistryStorage::GetDisplayName(const vs_user_id& user_id, std::string& display_name)
{
	if (!user_id)
		return false;

	VS_RealUserLogin realLogin(SimpleStrToStringView(user_id));
	{auto pLockedUsers = m_users.lock();
	UserMap::Iterator i= pLockedUsers->Find(realLogin);

	if(!i)
		return false;

	display_name=i->data->m_displayName;
	}
	return true;
}

int  VS_RegistryStorage::SearchUsers(VS_Container& cnt, const std::string& query, VS_Container* in_cnt)
{
	VS_StorageUserData param;
	bool do_search=ParseQuery(query,param);

	VS_StorageUserData* ud;

	auto pLockedUsers = m_users.lock();
	if (pLockedUsers->Size()<=0)
		return 0;

	std::map<VS_SimpleStr, VS_StorageUserData> result;

	const char* call_id = (in_cnt)? in_cnt->GetStrValueRef(CALLID_PARAM): 0;
	const char* email = (in_cnt)? in_cnt->GetStrValueRef(EMAIL_PARAM): 0;
	const char* name = (in_cnt)? in_cnt->GetStrValueRef(NAME_PARAM): 0;

	if ((call_id && *call_id) || (email && *email) || (name && *name))
		do_search = true;

	UserMap::Iterator i;
	int j=0;

	VS_SimpleStorage::SearchUsersByAlias(email, name, result);

	VS_SimpleStr by_call_id;
	if (call_id&&*call_id)
		by_call_id = call_id;
	else if (name&&*name)
		by_call_id = name;

	VS_SimpleStr by_email;
	if (email&&*email)
		by_email = email;
	else if (name&&*name)
		by_email = name;

	vs::UnicodeConverter<char, wchar_t> converter;
	for (i = pLockedUsers->Begin(); i != pLockedUsers->End(); ++i)
	{
		ud = i->data;
		if (!ud)
			continue;
		if (ud->m_client_type==CT_TRANSCODER)
			continue;
		bool add=!do_search;
		if(add) goto parse_end;
		if(!param.m_LastName.empty() && (!ud->m_LastName.empty() || !ud->m_displayName.empty()))
		{
            auto w_test = converter.Convert(ud->m_LastName.empty() ? ud->m_displayName : ud->m_LastName);
            auto w_LastName = converter.Convert(param.m_LastName);
            add |= boost::icontains(w_test, w_LastName);
		};
		if(add) goto parse_end;
		if(!param.m_FirstName.empty() && !ud->m_FirstName.empty())
		{
            auto w_test = converter.Convert(ud->m_FirstName);
            auto w_FirstName = converter.Convert(param.m_FirstName);
            add |= boost::icontains(w_test, w_FirstName);
		};
		if(add) goto parse_end;
		if(!!param.m_name && !!ud->m_name)
		{
			string_view ud_name_view(ud->m_name);
			string_view param_name_view(ud->m_name);
			add |= boost::icontains(ud_name_view, param_name_view);
		};

		if (by_call_id&&*by_call_id)
		{
			if (!ud->m_name.Length())
				continue;
			std::string str1;
			if (strchr(by_call_id, '@')!=0)			// search in full_id or short (withour @server.name)
				str1 = ud->m_name;
			else
				str1 = VS_RealUserLogin(SimpleStrToStringView(ud->m_name)).GetUser();
            if (boost::icontains(str1, by_call_id.m_str)) add = true;
		}

		if (by_email&&*by_email)
		{
			if (!ud->m_email.Length())
				continue;
			std::string str1;
			if (strchr(by_email, '@')!=0)			// search in full_id or short (withour @server.name)
				str1 = ud->m_email;
			else
				str1 = VS_RealUserLogin(SimpleStrToStringView(ud->m_email)).GetUser();
            if (boost::icontains(str1, by_email.m_str)) add = true;
		}

		if (name && *name)
		{
			if (ud->m_displayName.empty() && ud->m_FirstName.empty() && ud->m_LastName.empty())
				continue;

            auto wstr = converter.Convert(name);
			boost::trim(wstr);
			std::wstringstream wss;
			wss << wstr;
			while(!wss.eof())
			{
				std::wstring wterm;
				wss >> wterm;

				bool res(false);
                if (!res && !ud->m_FirstName.empty())   res |= boost::icontains(converter.Convert(ud->m_FirstName), wterm);
				if (!res && !ud->m_LastName.empty())    res |= boost::icontains(converter.Convert(ud->m_LastName), wterm);
				if (!res && !ud->m_displayName.empty()) res |= boost::icontains(converter.Convert(ud->m_displayName), wterm);

				// ktrushnikov: #9754: will be at IntellegentSearch at TrueConf Server 3.3.1
				//if (!res && !!ud->m_name)
				//{
				//	VS_WideStr x;	x.AssignStr(((VS_RealUserLogin)ud->m_name).GetUser());
				//	wtmp = x.m_str;	std::transform(wtmp.begin(), wtmp.end(), wtmp.begin(), std::bind1st(std::mem_fun(&ctype<wchar_t>::tolower), &ct));
				//	res |= (wtmp.find(wterm)!=std::string::npos);
				//}
				if (!res)
					break;
				if (wss.eof())
					add = true;
			}
		}

parse_end:
		if(add)
		{
			result[ud->m_name] = *ud;
		};
	}
	j=result.size();
	for(std::map<VS_SimpleStr, VS_StorageUserData>::iterator it=result.begin(); it!=result.end(); ++it)
	{
		cnt.AddValue(USERNAME_PARAM, it->second.m_name);
		cnt.AddValue(CALLID_PARAM,   it->second.m_name);
		cnt.AddValue(DISPLAYNAME_PARAM, it->second.m_displayName);
		cnt.AddValueI32(USERPRESSTATUS_PARAM, it->second.m_online_status);
	}

	cnt.AddValue(REQUEST_CALL_ID_PARAM, call_id);
	cnt.AddValue(REQUEST_EMAIL_PARAM, email);
	cnt.AddValue(REQUEST_NAME_PARAM, name);
	return j;
}

bool  VS_RegistryStorage::FetchRights(const VS_StorageUserData& ud, VS_UserData::UserRights& rights)
{
	int grpRights = ud.UR_NONE;
	int srvRights = GetPhysRights();
	rights=ud.UR_NONE;

	dprint3("FetchRights: %s\n",ud.m_name.m_str);

	if (ud.m_type == ud.UT_GUEST) {
		grpRights = (m_group_manager) ? m_group_manager->GuestRights() : VS_UserData::UR_NONE;
	}
	else if(m_useGroups && ud.m_groups.size()) {
		std::map<std::string, VS_RegGroupInfo> reg_groups;
		GetRegGroups(reg_groups);
		int num_rights = 0;
		for(std::vector<std::string>::const_iterator it=ud.m_groups.begin(); it!=ud.m_groups.end(); it++)
		{
			auto i = reg_groups.find(*it);
			if(i != reg_groups.end())
			{
				grpRights |= i->second.rights;
				num_rights++;
			}
		}
		if (!num_rights)
		{
			dprint3("not found\n");
			grpRights = (m_group_manager)? m_group_manager->m_defRights: VS_UserData::UR_NONE;
		}
	}
	else {
		grpRights = (m_group_manager)? m_group_manager->m_defRights: VS_UserData::UR_NONE;
	}

  /// set user rights
  /// rights, based on status

	if(ud.m_status&ud.US_LOGIN)
		rights=(VS_UserData::UserRights)(rights|ud.UR_LOGIN);

	rights= (VS_UserData::UserRights)
		(rights | (srvRights & grpRights) );

	VS_UserData ud_current;
	if (g_storage && g_storage->FindUser(SimpleStrToStringView(ud.m_name), ud_current))
		if (ud_current.m_rights & VS_UserData::UR_COMM_PROACCOUNT)
			rights = (VS_UserData::UserRights)(rights|ud.UR_COMM_PROACCOUNT);

	dprint3(" def:%x grp:%x srv:%x res:%x\n", (m_group_manager)? m_group_manager->m_defRights: VS_UserData::UR_NONE, grpRights, srvRights, rights);

	return true;
}

void VS_RegistryStorage::UpdateUserData(const VS_StorageUserData& ud)
{
	UserMap::Iterator i;
	auto pLockedUsers = m_users.lock();
	i = pLockedUsers->Find(ud.m_realLogin);
	if (!i) return ;
	VS_StorageUserData *pud = i->data;
	*pud = ud;
}

bool VS_RegistryStorage::DeleteUser(const vs_user_id& id)
{
	VS_RealUserLogin realLogin(SimpleStrToStringView(id));
	return m_users->Erase(realLogin) > 0;
}


bool VS_RegistryStorage::GetMissedCallMailTemplate(const std::chrono::system_clock::time_point /*missed_call_time*/, const char* fromId, std::string& /*inOutFromDn*/, const char * toId, std::string& /*inOutToDn*/, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ)
{
	GetMissedCallMailTemplateBase(subj_templ, body_templ);

	char tmp[512] = {0};
	if (fromId)
	{
		VS_RealUserLogin fromR(fromId);
		if (fromR.IsOurSID())
		{
			std::string key_name = USERS_KEY;	key_name += "\\";  key_name += fromR.GetUser();
			VS_RegistryKey key(false, key_name);
			if (key.IsValid() && key.GetValue(tmp, sizeof(tmp), VS_REG_STRING_VT, EMAIL_TAG) > 0)
				from_email = tmp;
		}
	}

	memset(tmp, 0, sizeof(tmp));
	if (toId)
	{
		VS_RealUserLogin toR(toId);
		if (toR.IsOurSID())
		{
			std::string key_name = USERS_KEY;	key_name += "\\"; key_name += toR.GetUser();
			VS_RegistryKey key2(false, key_name);
			if (key2.IsValid() && key2.GetValue(tmp, sizeof(tmp), VS_REG_STRING_VT, EMAIL_TAG) > 0)
				to_email = tmp;
		}
	}

	return true;
}

bool VS_RegistryStorage::GetInviteCallMailTemplate(const std::chrono::system_clock::time_point /*missed_call_time*/, const char *fromId, std::string& /*inOutFromDn*/, const char * toId, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ)
{
	GetInviteCallMailTemplateBase(subj_templ, body_templ);

	char tmp[512] = {0};
	if (fromId)
	{
		std::string key_name = USERS_KEY;	key_name += "\\"; key_name += VS_RealUserLogin(fromId).GetUser();
		VS_RegistryKey key(false, key_name);
		if (key.IsValid() && key.GetValue(tmp, sizeof(tmp), VS_REG_STRING_VT, EMAIL_TAG) > 0)
			from_email = tmp;
	}

	// Don't change, because no such user
	to_email = toId;

	return true;
}

bool VS_RegistryStorage::GetMultiInviteMailTemplate(const char* fromId, std::string& /*inOutFromDn*/, const char * toId, std::string& /*inOutToDn*/, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ)
{
	GetMultiInviteMailTemplateBase(subj_templ, body_templ);

	char tmp[512] = {0};
	if (fromId)
	{
		std::string key_name = USERS_KEY;	key_name += "\\"; key_name += VS_RealUserLogin(fromId).GetUser();
		VS_RegistryKey key(false, key_name);
		if (key.IsValid() && key.GetValue(tmp, sizeof(tmp), VS_REG_STRING_VT, EMAIL_TAG) > 0)
			from_email = tmp;
	}

	memset(tmp, 0, sizeof(tmp));
	if (toId)
	{
		std::string key_name = USERS_KEY;	key_name += "\\"; key_name += VS_RealUserLogin(toId).GetUser();
		VS_RegistryKey key2(false, key_name);
		if (key2.IsValid() && key2.GetValue(tmp, sizeof(tmp), VS_REG_STRING_VT, EMAIL_TAG) > 0)
			to_email = tmp;
	}

	return true;
}

bool VS_RegistryStorage::GetMissedNamedConfMailTemplate(const char* fromId, std::string& inOutFromDn, const char * toId, std::string& /*inOutToDn*/, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ)
{
	GetMissedNamedConfMailTemplateBase(subj_templ, body_templ);

	char tmp[512] = {0};
	VS_StorageUserData ude;
	if (FindUser(fromId, ude)) {
		from_email = ude.m_email;
        if(inOutFromDn.empty()) inOutFromDn = ude.m_displayName;  // if not got from client container, get fromDn here
	} else
		from_email = fromId;

	memset(tmp, 0, sizeof(tmp));
	if (toId)
	{
		std::string key_name2 = USERS_KEY;	key_name2 += "\\"; key_name2 += VS_RealUserLogin(toId).GetUser();
		VS_RegistryKey key2(false, key_name2);
		if (key2.IsValid() && key2.GetValue(tmp, sizeof(tmp), VS_REG_STRING_VT, EMAIL_TAG) > 0)
			to_email = tmp;
	}

	return true;
}

bool VS_RegistryStorage::IsDisabledUser(const char* user)
{
	if (!user || !*user)
		return false;

	VS_StorageUserData ude;
	if(!FindUser(user,ude))
		return false;

	return !(ude.m_status&ude.US_LOGIN);
}

bool VS_RegistryStorage::FindUser(const vs_user_id& id, VS_StorageUserData& user)
{
	if(!id)
	{
		error_code=VSS_USER_NOT_VALID;
		return false;
	};
	VS_RealUserLogin	realLogin(SimpleStrToStringView(id));

	{auto pLockedUsers = m_users.lock();
	UserMap::ConstIterator i= pLockedUsers->Find(realLogin);
	if(!i)
	{
		error_code=VSS_USER_NOT_FOUND;
		return false;
	}
	user = *i->data;
	}
	return true;
}

int VS_RegistryStorage::FindUsersPhones(VS_Container& cnt, int& entries, const vs_user_id& owner, long client_hash)
{
	VS_RealUserLogin r(SimpleStrToStringView(owner));
	long hash = VS_SimpleStorage::GetPhoneBookHash(r.GetUser());
	if (VS_CompareHash(hash, client_hash))
		return SEARCH_NOT_MODIFIED;

// non-Editable phones
	{auto pLockedUsers = m_users.lock();
	for (UserMap::Iterator it = pLockedUsers->Begin(); it != pLockedUsers->End(); ++it)
	{
		VS_StorageUserData* ude = it->data;
		for (std::vector<VS_UserPhoneItem>::iterator it2 = ude->m_phones.begin(); it2 != ude->m_phones.end(); ++it2)
		{
			if (!it2->phone)
				continue;
			entries++;
			cnt.AddValue(ID_PARAM, it2->id);
			cnt.AddValue(CALLID_PARAM, it2->call_id);
			cnt.AddValue(USERPHONE_PARAM, it2->phone);
			cnt.AddValueI32(TYPE_PARAM, it2->type);
			cnt.AddValue(EDITABLE_PARAM, it2->editable);
		}
	}}

// Editable phones
	std::vector<VS_UserPhoneItem> v;
	VS_SimpleStorage::FindUsersPhones(r.GetUser(), v);
	for(std::vector<VS_UserPhoneItem>::iterator it2=v.begin(); it2!=v.end(); ++it2)
	{
		if (!it2->phone)
			continue;
		entries++;
		cnt.AddValue(ID_PARAM, it2->id);
		cnt.AddValue(CALLID_PARAM, it2->call_id);
		cnt.AddValue(USERPHONE_PARAM, it2->phone);
		cnt.AddValueI32(TYPE_PARAM, it2->type);
		cnt.AddValue(EDITABLE_PARAM, it2->editable);
	}

	cnt.AddValueI32(HASH_PARAM, VS_SimpleStorage::GetPhoneBookHash(r.GetUser()));
	return SEARCH_DONE;
}

bool VS_RegistryStorage::GetRegGroupUsers(const std::string& gid, std::shared_ptr<VS_AbCommonMap>& users)
{
	char key_name[512];key_name[sizeof(key_name)-1]=0;
	snprintf(key_name,sizeof(key_name)-1,"Groups\\%s\\Users",gid.c_str());

	VS_RegistryKey ab(false, key_name);
	if(!ab.IsValid())
		return false;

	if (!users)
		users = std::make_shared<VS_AbCommonMap>();
	ab.ResetValues();

	std::string display_name;
	std::string call_id;

	while (ab.NextString(display_name,call_id),!call_id.empty())
	{
		VS_RealUserLogin r(call_id);
		const char* to_add = r;
		if (VS_IsNotTrueConfCallID(call_id))
			to_add = call_id.c_str();
		const char* dn = &display_name[0];
		VS_StorageUserData ude;
		if (FindUser(call_id.c_str(), ude) && !ude.m_displayName.empty())
			dn = ude.m_displayName.c_str();
		users->emplace(to_add, VS_AbCommonMap_Item(dn));
		call_id.clear();
		display_name.clear();
	}
	return true;
}

bool VS_RegistryStorage::GetAllUsers(std::shared_ptr<VS_AbCommonMap>& users)
{
	{auto pLockedUsers = m_users.lock();
	if (!users && pLockedUsers->Size())
		users = std::make_shared<VS_AbCommonMap>();
	for(UserMap::ConstIterator it= pLockedUsers->Begin(); it!= pLockedUsers->End(); ++it)
	{
		if (  it->data->m_type == VS_UserData::UT_GUEST
			||it->data->m_client_type == CT_TRANSCODER )
			continue;
		const char *displayname = it->data->m_displayName.c_str();
		users->emplace(it->key, VS_AbCommonMap_Item(displayname));
	}
	}
	return true;
}

void VS_RegistryStorage::GetGroupsUsers(std::map<std::string, std::vector<VS_RealUserLogin>>& groups_users)
{
	VS_RegistryKey group_key;
	VS_RegistryKey groups_key (false, GROUPS_KEY);		groups_key.ResetKey();
	while (groups_key.NextKey(group_key))
	{
		if (!group_key.IsValid())
			continue;
		if (!strcasecmp(group_key.GetName(),"@no_group"))
			continue;

		std::unique_ptr<char, free_deleter> group_ldap_dn;
		if (group_key.GetValue(group_ldap_dn, VS_REG_STRING_VT, "LDAP DN") && group_ldap_dn && *group_ldap_dn)
			continue;

		auto group = group_key.GetName();
		if (!group) continue;

		char key_name[512];key_name[sizeof(key_name)-1]=0;
		snprintf(key_name,sizeof(key_name)-1,"Groups\\%s\\Users", group);

		VS_RegistryKey users_key(false, key_name);
		users_key.ResetValues();

		std::string display_name;
		std::string call_id;

		while (users_key.NextString(display_name, call_id))
		{
			VS_RealUserLogin r(call_id);
			groups_users[group].push_back(r);
		}
	}
}

long VS_RegistryStorage::ChangePassword(const char* call_id, const char* old_pass, const char* new_pass, const VS_SimpleStr& from_app_id)
{
	if (!old_pass || !new_pass || !*new_pass) return -1;

	VS_Container cnt;
	cnt.AddValue("NewPassword", new_pass);
	cnt.AddValue("OldPassword", old_pass);
	cnt.AddValue("FromAppId", from_app_id.m_str);

	return UpdatePerson(call_id, cnt);
}

long VS_RegistryStorage::UpdatePerson( const char* call_id, VS_Container& cnt )
{
	return UpdatePerson(call_id, cnt, NULL);
}

bool SaveField(const VS_Container &cnt, const char *registry_name, VS_RegistryKey &reg,std::string &OUTcopy) {
    if (registry_name == nullptr) return false;

    const char * val_ref = cnt.GetStrValueRef();
    if (val_ref != nullptr) OUTcopy = val_ref;
    return reg.SetString(OUTcopy.c_str(), registry_name);
}

long VS_RegistryStorage::UpdatePerson(const char* call_id, VS_Container& cnt, const char *fields)
{
	if (!call_id) return -1;

	if (VS_IsNotTrueConfCallID(call_id))
		return 0;

	auto pLockedUsers = m_users.lock();
	UserMap::ConstIterator i= pLockedUsers->Find(call_id);

	if(!i) {
		error_code=VSS_USER_NOT_FOUND;
		return -1;
	}

	VS_StorageUserData &user = *i->data;

	// Update Registry
	std::string root(USERS_KEY);	root += "\\"; root += user.m_realLogin.GetUser();
	VS_RegistryKey reg_user_test(false, root, false);
	if (!reg_user_test.IsValid()) {
		error_code=VSS_REGISTRY_ERROR;
		return -1;
	}

	VS_RegistryKey reg_user(false, root, false, true);

	VS_SimpleStr prop;
	if (fields == NULL || *fields == 0)
	{
		GetAppProperty(user.m_appName, "editable_profile_fields", prop);
		if (prop.m_str == NULL)
		{
			prop = "";
		}
	}
	else prop = fields;

	prop.ToLower();
	std::set<std::string> editable;
	boost::split(editable, prop.m_str, boost::is_any_of(", "), boost::token_compress_on );

	cnt.Reset();
	bool res = false;

	while( cnt.Next() )
	{
		const char *FieldName = cnt.GetName();

		if ( strcasecmp(FieldName, DISPLAYNAME_PARAM) == 0 && editable.find("displayname") != editable.end()){
            res = SaveField(cnt, DISPLAY_NAME_TAG, reg_user, user.m_displayName);
		}
		else

		if ( strcasecmp(FieldName, FIRSTNAME_PARAM) == 0 && editable.find("firstname") != editable.end()){
            res = SaveField(cnt, FIRST_NAME_TAG, reg_user, user.m_FirstName);
		}
		else

		if ( strcasecmp(FieldName, LASTNAME_PARAM) == 0 && editable.find("lastname") != editable.end()){
            res = SaveField(cnt, LAST_NAME_TAG, reg_user, user.m_LastName);
		}
		else

		if ( strcasecmp(FieldName, USERCOMPANY_PARAM) == 0 &&  editable.find("company") != editable.end()){
            res = SaveField(cnt, COMPANY_TAG, reg_user, user.m_Company);
		}
		else

		if ( strcasecmp(FieldName, "NewPassword") == 0 &&  editable.find("password") != editable.end())
		{
			const char *old_pass = cnt.GetStrValueRef("OldPassword");
			const char *new_pass = cnt.GetStrValueRef();

			if (!old_pass || !new_pass)  return -1;
			if ( strcasecmp(user.m_password, old_pass) != 0 ) return 3; // passwords doesn't match

			user.m_password = new_pass;
			reg_user.SetString(user.m_password.m_str, PASSWORD_TAG);
			res = true;
		}
		else
		if ( strcasecmp(FieldName, "NewPassword") == 0 && editable.find("password_plain") != editable.end())
		{
			const char *old_pass = cnt.GetStrValueRef("OldPassword");
			const char *new_pass = cnt.GetStrValueRef();

			if (!old_pass || !new_pass)  return -1;

			char buff[33] = {0};
			VS_ConvertToMD5(old_pass, buff);
			if ( strcasecmp(user.m_password, buff) != 0 ) return 3; // passwords doesn't match

			// remove autologins
			char autoKey[1024];
			{
				std::string root= USERS_KEY;	root += "\\";	root += call_id;	root += "\\AutoLogins";
				VS_RegistryKey logins(false, root, false, true);

				const char *appID = cnt.GetStrValueRef("FromAppId");
				if (appID)
					logins.GetValue(autoKey, sizeof(autoKey), VS_REG_STRING_VT, appID);
				else
					logins.GetValue(autoKey, sizeof(autoKey), VS_REG_STRING_VT, user.m_appID);
			}

			reg_user.RemoveKey("AutoLogins");
			SaveAutoLoginKey(call_id, user.m_appID, autoKey);

			VS_ConvertToMD5(new_pass, buff);
			user.m_password = buff;
			reg_user.SetString(user.m_password.m_str, PASSWORD_TAG);

			MD5 md5;
			md5.Update(VS_RealUserLogin(SimpleStrToStringView(user.m_name)).GetUser());
			md5.Update(":trueconf:");
			md5.Update(new_pass);
			md5.Final();
			md5.GetString(buff);
			user.m_HA1_password = buff;
			reg_user.SetString(user.m_HA1_password.m_str, HA1_PASSWORD_TAG);

			user.m_h323_password = new_pass;
			reg_user.SetString(sec::EncryptRegistryPassword(reg_user.GetName(), new_pass).c_str(), H323_PASSWORD_TAG_NEW);

			res = true;
		}
	}

	return res ? 0 : -1;
}

std::string VS_RegistryStorage::GetDefauldEditableFields()
{
	return std::string("displayName,FirstName,LastName,company,password_plain");
}

bool VS_RegistryStorage::IsCacheReady() const
{
	return true;
}

bool VS_RegistryStorage::GetRegGroups(std::map<std::string, VS_RegGroupInfo>& reg_groups)
{
	return (m_group_manager) ? m_group_manager->GetRegGroups(reg_groups) : false;
}

bool VS_RegistryStorage::IsLDAP_Sink() const
{
	return (m_group_manager) ? m_group_manager->IsLDAP_Sink() : false;
}