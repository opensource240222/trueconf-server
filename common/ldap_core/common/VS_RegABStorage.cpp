/**
 ****************************************************************************
 * Project: server services
 * \author stass
 * \brief Registry AB Storage
 *
 ****************************************************************************/

#include "../../std/cpplib/VS_CallIDUtils.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/md5.h"

#include "VS_RegABStorage.h"
#include "VS_LogABLimit_Server.h"
#include <boost/make_shared.hpp>

#include "std-generic/compat/map.h"
#include "ldap_core/common/VS_UserPartEscaped.h"

const char REG_AB_KEY_MASK[]  = "Users\\%s\\Address Book";
const char REG_BAN_KEY_MASK[]  = "Users\\%s\\Ban List";

const wchar_t FIRST_NAME_TAG[]	= L"First Name";

#define DEBUG_CURRENT_MODULE VS_DM_DBSTOR

bool ClientHashChanged(const VS_RegistryKey &ab_key, const long client_hash, long &OUT_new_hash) {
	OUT_new_hash = VS_MakeHash(ab_key.GetLastWriteTime());
	return !VS_CompareHash(client_hash, OUT_new_hash);
}

int VS_RegABStorage::ABFind(VS_Container& cnt, int& users, VS_AddressBook ab,const char*  owner, const std::string& query, long client_hash, VS_Container* in_cnt)
{
	if (!owner)
		return SEARCH_FAILED;

	switch(ab)
	{
	case AB_PERSONS:
		if (!CanEdit())
			return SEARCH_FAILED;
		users=m_sink->SearchUsers(cnt,query,in_cnt);
		return SEARCH_DONE;
	case AB_BAN_LIST:
		{
			char key_name[512] = {0};
            snprintf(key_name, sizeof(key_name) - 1,REG_BAN_KEY_MASK, owner);

			VS_RegistryKey ab_key(false, key_name, false);	// readOnly=false for RemoveDups()
			if(!ab_key.IsValid())
			{
				int32_t server_hash = (++client_hash)? client_hash: ++client_hash;
				cnt.AddValueI32(HASH_PARAM, server_hash);
				return SEARCH_DONE;
			}

			RemoveDups(ab_key);

            long hash(0);
            if (!ClientHashChanged(ab_key, client_hash, hash)) {
                users = -1;
                return SEARCH_NOT_MODIFIED;
            }

			cnt.AddValueI32(HASH_PARAM, hash);

			ab_key.ResetValues();

			std::string display_name;
			std::string call_id;

			vs::map<std::string, std::string> m;

			while (ab_key.NextString(display_name, call_id))
			{
				VS_RealUserLogin r(call_id);
				const char* to_add = r;
				if (VS_IsNotTrueConfCallID(call_id))
					to_add = call_id.c_str();
				m.emplace(std::move(to_add), std::move(display_name));
			}

			for(auto it=m.begin(); it!=m.end(); it++)
			{
				cnt.AddValue(USERNAME_PARAM, it->first);
				cnt.AddValue(CALLID_PARAM,   it->first);
				cnt.AddValue(DISPLAYNAME_PARAM, it->second);
				users++;
			}
		}
		return SEARCH_DONE;
	case AB_COMMON:
		{
			if (!m_sink->IsCacheReady())
			{
				users=-1;
				return SEARCH_NOT_MODIFIED;
			}

			VS_AbCommonMap m;
			long server_hash(0);
			VS_LogABLimit_Server limits;
			GetABForUser(owner,m,server_hash,&limits);
			if (VS_CompareHash((int32_t)client_hash, (int32_t)server_hash))
			{
				users=-1;
				return SEARCH_NOT_MODIFIED;
			}
			cnt.AddValueI32(HASH_PARAM, server_hash);

			for(VS_AbCommonMap::iterator it=m.begin(); it!=m.end(); it++)
			{
				cnt.AddValue(USERNAME_PARAM, it->first);
				cnt.AddValue(CALLID_PARAM,   it->first);
				cnt.AddValue(DISPLAYNAME_PARAM, it->second.displayName);
				if (it->second.IsCustomContactOfUser == false)
					cnt.AddValue(EDITABLE_PARAM, false);
#ifdef _SVKS_M_BUILD_
				cnt.AddValue("mvdPosition", it->second.mvdPosition);
				cnt.AddValue("ou", it->second.ou);
#endif
				users++;
			}

			return SEARCH_DONE;
		}
	case AB_GROUPS:
		{
			VS_RegistryKey key(false, GROUPS_KEY);
			if(!key.IsValid())
				return SEARCH_FAILED;

            long hash(0);
            if (!ClientHashChanged(key, client_hash, hash)) {
                users = -1;
                return SEARCH_NOT_MODIFIED;
            }
			cnt.AddValueI32(HASH_PARAM, hash);

			VS_RegistryKey group_key;
			key.ResetKey();
			while (key.NextKey(group_key))
			{
				if (!group_key.IsValid())
					continue;
				if (!strcasecmp(group_key.GetName(),"@no_group"))
					continue;
				std::unique_ptr<char, free_deleter> group_ldap_dn;
				if (group_key.GetValue(group_ldap_dn, VS_REG_STRING_VT, "LDAP DN") && group_ldap_dn && *group_ldap_dn)
				{
					if (!m_sink->IsLDAP_Sink())
						continue;
				}
				std::string dn;
				group_key.GetString(dn, "GroupName");

				cnt.AddValue(GID_PARAM, group_key.GetName());
				cnt.AddValue(GNAME_PARAM, dn);
				cnt.AddValueI32(GTYPE_PARAM, eGroupType::SYSTEM_GROUP);

				std::map<std::string, std::string> m;
				char key_name[512];key_name[sizeof(key_name)-1]=0;
				auto fetch_users = [&m, &key_name]() {
					VS_RegistryKey users_key(false, key_name);
					users_key.ResetValues();

					std::string display_name;
					std::string call_id;

					while (users_key.NextString(display_name, call_id))
					{
						if (call_id.empty())
							continue;

						auto utf8_call_id_lower = vs::UTF8ToLower(call_id);
						VS_RealUserLogin r(utf8_call_id_lower);
						const char* to_add = r;
						if (VS_IsNotTrueConfCallID(utf8_call_id_lower))
							to_add = utf8_call_id_lower.c_str();
						m.insert(std::pair<std::string, std::string>(to_add, display_name));
					}
				};

				snprintf(key_name, sizeof(key_name) - 1, "Groups\\%s\\Users", group_key.GetName());
				fetch_users();
				snprintf(key_name, sizeof(key_name) - 1, "Groups\\%s\\Address Book\\Contacts", group_key.GetName());		// CustomContacts of GroupAB
				fetch_users();

				for(auto it=m.begin(); it!=m.end(); it++)
				{
					cnt.AddValue(CALLID_PARAM,   it->first);
				}

				users++;		// count of groups
			}

			// -----------------------
			// TODO: пользовательские группы
			// -----------------------

			return SEARCH_DONE;
		}
	default:
		return SEARCH_FAILED;
	};
}

int VS_RegABStorage::ABAdd(VS_AddressBook ab,const vs_user_id& user_id1, const char* call_id2, const char* display_name, long& hash, bool IsFromServer)
{
	VS_SimpleStr tmp1;
    std::string tmp2;
	return ABAdd(ab, user_id1, call_id2, display_name, hash, tmp1, tmp2, true, IsFromServer);
}

int VS_RegABStorage::ABAdd(VS_AddressBook ab,const vs_user_id& user_id1, const char* call_id2, const char* dn, long& hash, VS_SimpleStr& add_call_id, std::string& add_display_name, bool use_full_name, bool IsFromServer)
{
	if (!IsFromServer && !CanEdit())
		return VSS_USER_ACCESS_DENIED;

	const char* book_mask = 0;
	if (ab==AB_BAN_LIST)
		book_mask = REG_BAN_KEY_MASK;
	else if (ab==AB_COMMON) {
		book_mask = REG_AB_KEY_MASK;
		VS_RealUserLogin r(SimpleStrToStringView(user_id1));
		VS_StorageUserData ude;
		if (!m_sink->FindUser_Sink((const char*)r, ude) || !(ude.m_rights&VS_UserData::UR_COMM_EDITAB))
			return VSS_USER_ACCESS_DENIED;
	}

	if(book_mask)
	{
		bool IsNotTC = VS_IsNotTrueConfCallID(nullptr == call_id2 ? "" : call_id2);
		VS_RealUserLogin r2(nullptr == call_id2 ? "" : call_id2);
		if (!r2 || r2.GetUser().empty())
			return VSS_USER_ACCESS_DENIED;

		std::string display_name;
		if (dn)
			display_name = dn;
		else if (!m_sink->GetDisplayName((const char*)r2, display_name)) {
			if (IsNotTC && call_id2) {
				display_name = call_id2;
			} else {
				auto user = r2.GetUser();
				if (!user.empty()) display_name = user;
			}
		}


		char key_name[512] = {0};
		VS_RealUserLogin r(SimpleStrToStringView(user_id1));
		VS_SimpleStr _user_id1 = (use_full_name)? user_id1.m_str: r.GetUser().c_str();
        snprintf(key_name,sizeof(key_name)-1, book_mask, VS_GetUserPartEscaped(_user_id1.m_str).c_str());

		VS_RegistryKey ab_key(false, key_name, false, true);
		if(!ab_key.IsValid())
			return VSS_REGISTRY_ERROR;

		RemoveDups(ab_key);

		if (ab==AB_COMMON)
		{
			VS_AbCommonMap m;
			GetABForUserFromGroups(r.GetUser().c_str(), m);
			if (m.find((const char*)r2)!=m.end() || m.find(r2.GetUser())!=m.end())		// user can delete only users from his own AB
				return VSS_USER_EXISTS;
		}

		bool IsFull = ab_key.HasValue(r2.GetID());
		bool IsShort = ab_key.HasValue(r2.GetUser());
		bool IsExist = (r2.IsOurSID())? IsFull||IsShort: IsFull;
		if (IsExist)
			return VSS_USER_EXISTS;

		if(!ab_key.SetString(display_name.c_str(), r2.IsOurSID() ? r2.GetUser().c_str() : static_cast<const char*>(r2)))
			return VSS_USER_ACCESS_DENIED;

		hash=0;
		if (ab==AB_COMMON)
		{
			VS_AbCommonMap tmp;
			GetABForUser(_user_id1, tmp, hash);
		} else if (ab==AB_BAN_LIST) {
			hash = VS_MakeHash(ab_key.GetLastWriteTime());
		}

		add_call_id = (IsNotTC)? call_id2: static_cast<const char*>(r2);
		add_display_name = std::move(display_name);

		return 0;
	}
	else
		return -1;
}


int VS_RegABStorage::ABRemove(VS_AddressBook ab,const vs_user_id& user_id1,const vs_user_id& user_id2, long& hash, bool IsFromServer)
{
	if (!IsFromServer && !CanEdit())
		return VSS_USER_ACCESS_DENIED;

	const char* book_mask = 0;
	if (ab==AB_BAN_LIST)
		book_mask = REG_BAN_KEY_MASK;
	else if (ab==AB_COMMON) {
		book_mask = REG_AB_KEY_MASK;
		VS_RealUserLogin r(SimpleStrToStringView(user_id1));
		VS_StorageUserData ude;
		if (!m_sink->FindUser_Sink((const char*)r, ude) || !(ude.m_rights&VS_UserData::UR_COMM_EDITAB))
			return VSS_USER_ACCESS_DENIED;
	}

	if(book_mask)
	{
		if(!user_id2)
			return VSS_USER_NOT_VALID;

		VS_RealUserLogin r1(SimpleStrToStringView(user_id1));
		char key_name[512] = {0};
        snprintf(key_name,sizeof(key_name)-1, book_mask, VS_GetUserPartEscaped(r1.GetUser().c_str()).c_str());

		VS_RegistryKey ab_key(false, key_name, false);
		if(!ab_key.IsValid())
			return VSS_REGISTRY_ERROR;

		RemoveDups(ab_key);

		VS_RealUserLogin r2(SimpleStrToStringView(user_id2));

		if (ab==AB_COMMON)
		{
			VS_AbCommonMap m;
			GetABForUserFromGroups(r1.GetUser().c_str(), m);
			if (m.find((const char*)r2)!=m.end() || m.find(r2.GetUser())!=m.end())		// user can delete only users from his own AB
				return VSS_USER_ACCESS_DENIED;
		}
		if (!ab_key.RemoveValue(r2.GetID()) && !ab_key.RemoveValue(r2.GetUser()))
			return VSS_USER_ACCESS_DENIED;

		hash=0;
		if (ab==AB_COMMON)
		{
			VS_AbCommonMap tmp;
			GetABForUser(r1.GetUser().c_str(), tmp, hash);
		} else if (ab==AB_BAN_LIST) {
			hash = VS_MakeHash(ab_key.GetLastWriteTime());
		}

		return 0;
	}
	else
		return -1;
}


int VS_RegABStorage::ABUpdate(VS_AddressBook ab, const vs_user_id& user_id1, const char* call_id2, VS_Container& cnt, long& hash)
{
  if(ab==AB_COMMON)
  {
	  if (!CanEdit())
		  return VSS_USER_ACCESS_DENIED;

	  VS_RealUserLogin r(SimpleStrToStringView(user_id1));
	  VS_StorageUserData ude;
	  if (!m_sink->FindUser_Sink((const char*)r, ude) || !(ude.m_rights&VS_UserData::UR_COMM_EDITAB))
		  return VSS_USER_ACCESS_DENIED;

    if(!call_id2 || !*call_id2)
      return VSS_USER_NOT_VALID;

    const char* u_dn=cnt.GetStrValueRef(DISPLAYNAME_PARAM);
    if(!u_dn)
      return VSS_USER_NOT_VALID;

	char key_name[512] = {0};
    snprintf(key_name,sizeof(key_name)-1, REG_AB_KEY_MASK, VS_GetUserPartEscaped(r.GetUser().c_str()).c_str());

    VS_RegistryKey ab_key(false, key_name, false, true);
    if(!ab_key.IsValid())
      return VSS_REGISTRY_ERROR;

	RemoveDups(ab_key);

	VS_RealUserLogin r2(call_id2);
	bool IsFull = ab_key.HasValue(r2.GetID());
	bool IsShort = ab_key.HasValue(r2.GetUser());
    if(!IsFull && !IsShort)
      return VSS_USER_ACCESS_DENIED;

	// check that user2 is not from group AB
	VS_AbCommonMap m;
	GetABForUserFromGroups(r.GetUser().c_str(), m);
	if (m.find((const char*)r2)!=m.end() || m.find(r2.GetUser())!=m.end())		// user can delete only users from his own AB
		return VSS_USER_ACCESS_DENIED;

	VS_SimpleStr to_update = 0;
	if (VS_IsNotTrueConfCallID(call_id2))
		to_update = call_id2;
	else
		to_update = (IsShort)? (const char*)(r2.GetUser().c_str()): (const char*)r2;

	if(!ab_key.SetString(u_dn, to_update))
      return VSS_USER_ACCESS_DENIED;

	hash=0;
	if (ab==AB_COMMON)
	{
		VS_AbCommonMap tmp;
		GetABForUser(r.GetUser().c_str(), tmp, hash);
	} else if (ab==AB_BAN_LIST) {
		hash = VS_MakeHash(ab_key.GetLastWriteTime());
	}

    return 0;
  }
  else
    return -1;
}

void VS_RegABStorage::RemoveDups(VS_RegistryKey& key)
{
	std::string display_name, call_id;

	key.ResetValues();
	while (key.NextString(display_name,call_id))
	{
		if (call_id.empty())
			continue;

		VS_RealUserLogin r(call_id);

		if (r== call_id.c_str() && r.IsOurSID())
		{
			char tmp[256] = {0};
			if (key.GetValue(tmp, sizeof(tmp), VS_REG_STRING_VT, r.GetUser().c_str())<=0)
			{
				key.SetString(display_name.c_str(), r.GetUser().c_str());
			}

			if (key.RemoveValue(r.GetID()))		// ensure we deleted this key, so we woun't have an infinite loop after ResetValues()
				key.ResetValues();		// restart from begin
		}
	}
}

bool VS_RegABStorage::CanEdit()
{
	bool res(true);
	VS_RegistryKey cfg_key(false, CONFIGURATION_KEY);
	if (!cfg_key.IsValid())
		return res;
	unsigned long val(0);
	if (cfg_key.GetValue(&val,sizeof(unsigned long),VS_REG_INTEGER_VT,"EditAB Disabled")>0 && val>0)
		res = false;
	return res;
}

bool VS_RegABStorage::GetABForUserFromGroups(const char* owner, VS_AbCommonMap& m, VS_LogABLimit_Interface* limits)
{
	std::map<std::string, VS_RegGroupInfo> reg_groups;
	m_sink->GetRegGroups(reg_groups);

	VS_RealUserLogin r(nullptr == owner ? "" : owner);
	VS_StorageUserData ude;
	if (!m_sink->FindUser_Sink((const char*)r, ude))
		return SEARCH_FAILED;

	std::vector<std::string> process_groups;

	if (!ude.m_groups.size()) {
		process_groups.emplace_back("@no_group");
	} else {
		for(std::vector<std::string>::iterator i=ude.m_groups.begin(); i!=ude.m_groups.end(); ++i)
			process_groups.push_back((*i));
	}

	bool HaveAllUsers(false);
	std::shared_ptr<VS_AbCommonMap> all_users;

	for (const auto& process_group : process_groups)
	{
		auto it = reg_groups.find(process_group);
		if (it==reg_groups.end())
			continue;
		const VS_RegGroupInfo& g = it->second;

		// add special group contacts from registry
		if (m.size() + g.contacts.size() <= AB_LIMIT(AB_MAXSIZE_BY_GROUPS)) {
			for (const auto& c : g.contacts)
			{
				auto& ref = m[c.first];
				ref.displayName = c.second;
				ref.IsCustomContactOfGroup = true;
			}
		} else {
			if (limits)
				limits->LimitByGroupCustomContacts(owner, m.size(), g.contacts.size(), AB_LIMIT(AB_MAXSIZE_BY_GROUPS), it->first);
		}

		// check scope
		if (g.scope == e_ab_scope_all_users) {
			if (!HaveAllUsers)
			{
				m_sink->GetAllUsers(all_users);
				HaveAllUsers = true;
			}
			if (all_users)
			{
				if (m.size() + all_users->size() <= AB_LIMIT(AB_MAXSIZE_BY_GROUPS))
					m.insert(all_users->begin(), all_users->end());
				else {
					if (limits)
						limits->LimitByGroupAllUsers(owner, m.size(), all_users->size(), AB_LIMIT(AB_MAXSIZE_BY_GROUPS), it->first);
				}
			}
		}else if (g.scope == e_ab_scope_groups) {
			for (const auto& gr : g.groups)
			{
				std::shared_ptr<VS_AbCommonMap> tmp;
				m_sink2->GetRegGroupUsers(gr, tmp);
				if (tmp)
				{
					if (m.size() + tmp->size() <= AB_LIMIT(AB_MAXSIZE_BY_GROUPS))
						m.insert(tmp->begin(), tmp->end());
					else {
						if (limits)
							limits->LimitByGroup(owner, m.size(), tmp->size(), AB_LIMIT(AB_MAXSIZE_BY_GROUPS), it->first, gr);
					}
				}
			}
		}else if (g.scope == e_ab_scope_nobody) {
			// do nothing
		}
	}
	if (limits && !limits->IsErrorShown() && m.size() > AB_LIMIT(WARNING_AB_MAXSIZE_BY_GROUPS))
		limits->WarnByGroup(owner, m.size(), AB_LIMIT(WARNING_AB_MAXSIZE_BY_GROUPS));
	return false;
}

bool VS_RegABStorage::GetABForUserFromRegistry(const vs_user_id& owner, VS_AbCommonMap& m, VS_LogABLimit_Interface* limits)
{
	char key_name[512] = {0};
    snprintf(key_name,sizeof(key_name)-1, REG_AB_KEY_MASK, VS_GetUserPartEscaped(owner.m_str).c_str());

	VS_RegistryKey ab_key(false, key_name, false);	// readOnly=false for RemoveDups()
	if(!ab_key.IsValid())
		return false;

	RemoveDups(ab_key);

	ab_key.ResetValues();

	unsigned long total_count(0);

	std::string display_name;
	std::string call_id;

	while (ab_key.NextString(display_name,call_id),!call_id.empty())
	{
		VS_RealUserLogin r(call_id);
		const char* to_add = r;
		if (VS_IsNotTrueConfCallID(call_id))
			to_add = call_id.c_str();
		if (total_count < AB_LIMIT(AB_MAXSIZE_BY_CUSTOM_AB)) {
			auto& ref = m[to_add];
			ref.displayName = display_name;
			ref.IsCustomContactOfUser = true;
		}
		else
		{
			if (limits)
				limits->LimitByUser(owner.m_str, AB_LIMIT(AB_MAXSIZE_BY_CUSTOM_AB));
			break;
		}
		call_id.clear();
		display_name.clear();
		++total_count;
	}
	if (limits && !limits->IsErrorShown() && total_count > AB_LIMIT(WARNING_AB_MAXSIZE_BY_CUSTOM_AB))
		limits->WarnByUser(owner.m_str, AB_LIMIT(WARNING_AB_MAXSIZE_BY_CUSTOM_AB));
	return false;
}

bool VS_RegABStorage::GetABForUser(const char* owner, VS_AbCommonMap& m, long& server_hash, VS_LogABLimit_Interface* limits)
{
	if (!owner)
		return false;

	if (!m_sink->IsCacheReady())
		return false;

	if (m_sink->IsLDAP_Sink()) {
		if (!m_sink->GetABForUserImp(owner, m))
		{
			GetABForUserFromGroups(owner,m,limits);
			GetABForUserFromRegistry(owner,m,limits);
		}
	} else {
		GetABForUserFromGroups(owner,m,limits);
		GetABForUserFromRegistry(owner,m,limits);
	}

	m.erase((const char*)owner);
	m.erase((const char*)(VS_RealUserLogin)owner);

	// calc hash of a map
	MD5 md5;
	for(VS_AbCommonMap::iterator it=m.begin(); it!=m.end(); it++)
	{
		md5.Update(it->first);
		md5.Update(it->second.displayName);
#ifdef _SVKS_M_BUILD_
		md5.Update(it->second.mvdPosition.c_str(), it->second.mvdPosition.length()*sizeof(wchar_t));
		md5.Update(it->second.ou.c_str(), it->second.ou.length()*sizeof(wchar_t));
#endif
	}
	md5.Final();
	unsigned char digest[16];
	md5.GetBytes(digest);
	const auto p = reinterpret_cast<const uint32_t*>(digest);
	server_hash = p[0] ^ p[1] ^ p[2] ^ p[3];
	return true;
}