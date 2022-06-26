#include "VS_GroupManager.h"

#include "../std/cpplib/VS_CallIDUtils.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "../std/cpplib/VS_RegistryConst.h"
#include "../std/debuglog/VS_Debug.h"
#include "../std/clib/vs_defines.h"
#include "std-generic/clib/strcasecmp.h"

#define DEBUG_CURRENT_MODULE VS_DM_RSTOR

const char RIGHTS_TAG[] = "Rights";
const char IS_OPERATORS_TAG[] = "IsOperators";
const char URCHATALLOWED_TAG[] = "URChatAllowed";

void VS_GroupManager::UpdateGroupList()
{
	std::lock_guard<std::mutex> lock(m_reg_groups_lock);

	//  Verify Registry Structure
	VS_RegistryKey  group_root(false, GROUPS_KEY, false, true);

	long ChatAllowedTotal = 1;
	{
		VS_RegistryKey  cfg(false, CONFIGURATION_KEY);
		cfg.GetValue(&ChatAllowedTotal, sizeof(ChatAllowedTotal), VS_REG_INTEGER_VT, URCHATALLOWED_TAG);
	}


	if (!group_root.IsValid())
	{
		//		error_code = VSS_REGISTRY_ERROR;
		return;
	}

	dprint3("RS: UpdateGroupList\n");
	auto last_write = group_root.GetLastWriteTime();
	if (!m_last_write_time.empty() && m_last_write_time == last_write)
	{
		dprint3(" no update\n");
		return;
	};
	m_last_write_time = last_write;

	m_reg_groups.clear();

	group_root.ResetKey();


	VS_RegistryKey  group;
	long grpUnparsed(0);

	// Enumerate and verify all Endpoints
	//m_reg_groups[]=
	while (group_root.NextKey(group))
	{
		std::string gid;
		const char *pName(nullptr);
		if((pName = group.GetName()) != nullptr) gid = pName;
		dstream3 << " [" << gid << "]\n";
		if (!group.IsValid())
			continue;
		VS_RegGroupInfo group_info;
		if (strcasecmp(gid.c_str(), "@no_group") != 0)		// process @no_group at both Registry and LDAP modes
		{
			group.GetString(group_info.ldap_dn, "LDAP DN");
			if (IsLDAP_Sink()) {
				if (group_info.ldap_dn.empty())
					continue;
			}
			else {	// registry mode
				if (!group_info.ldap_dn.empty())
					continue;
			}
		}
		group.GetString(group_info.group_name, "GroupName");
		if (group.GetValue(&grpUnparsed, sizeof(grpUnparsed), VS_REG_INTEGER_VT, RIGHTS_TAG) <= 0)
			grpUnparsed = 0;
		unsigned long val(0);
		if (group.GetValue(&val, sizeof(val), VS_REG_INTEGER_VT, IS_OPERATORS_TAG) > 0 && val > 0)
			group_info.IsOperators = true;
		if (ChatAllowedTotal != 0) { // add GR_CHAT to rights
			val = 1;
			group.GetValue(&val, sizeof(val), VS_REG_INTEGER_VT, URCHATALLOWED_TAG);
			if (val != 0)
				grpUnparsed |= GR_CHAT;
		}

		dprint3("(%ld)\n", grpUnparsed);
		group_info.rights = GroupToUserRights((GroupRights)grpUnparsed);

		std::string key_name = GROUPS_KEY; key_name += "\\";	key_name += group.GetName();		key_name += "\\Address Book";
		VS_RegistryKey key(false, key_name);
		if (key.IsValid())
		{
			key.GetValue(&group_info.scope, sizeof(int), VS_REG_INTEGER_VT, "Scope");

			std::string display_name;
			std::string call_id;

			std::string key2_name = key_name; key2_name += "\\";	key2_name += "Contacts";
			VS_RegistryKey key2(false, key2_name);
			if (key2.IsValid())
			{
				// read contacts
				key2.ResetValues();
				while (key2.NextString(display_name, call_id), !call_id.empty())
				{
					VS_RealUserLogin r(call_id);
					const char* to_add = r;
					if (VS_IsNotTrueConfCallID(call_id))
						to_add = call_id.c_str();
					group_info.contacts.insert(std::pair<std::string, std::string>(to_add, display_name));
					call_id.clear();
					display_name.clear();
				}
			}

			std::string key3_name = key_name;
			key3_name += '\\';
			key3_name += GROUPS_KEY;
			VS_RegistryKey key3(false, key3_name);
			if (key3.IsValid())
			{
				// read AB groups
				key3.ResetValues();
				while (key3.NextString(display_name, call_id), !call_id.empty())
				{
					group_info.groups.push_back(call_id);
					call_id.clear();
					display_name.clear();
				}
			}
		}

		// read ApplicationSettings
		std::string app_settings_key_name = GROUPS_KEY;
		app_settings_key_name += "\\";
		app_settings_key_name += group.GetName();
		app_settings_key_name += '\\';
		app_settings_key_name += APPLICATION_SETTINGS_KEY;
		VS_RegistryKey app_settings_key(false, app_settings_key_name);

		std::unique_ptr<void, free_deleter> buffer;
		RegistryVT type;
		std::string valueName;

		if (app_settings_key.IsValid()) {
			app_settings_key.ResetKey();
			VS_RegistryKey current;
			while (app_settings_key.NextKey(current)) {
				const char *keyName = current.GetName();

				while (current.NextValueAndType(buffer, type, valueName)) {
					switch (type)
					{
					case VS_REG_INTEGER_VT:
						if (valueName == "Value") {
							group_info.ApplicationSettings[keyName].integer = *static_cast<const uint32_t*>(buffer.get());
						} else if (valueName == "IsLocked") {
							group_info.ApplicationSettings[keyName].IsLocked = *static_cast<const uint32_t*>(buffer.get());
						}
						break;
					default:
						// no support
						break;
					}
				}
			}
		}

		m_reg_groups[gid] = group_info;
	}

	// update default rights
	auto defgroup = m_reg_groups.find("@no_group");
	if (defgroup != m_reg_groups.end())
		m_defRights = (VS_UserData::UserRights)defgroup->second.rights;
	else
		m_defRights = m_physRights;
}

bool VS_GroupManager::GetRegGroups(std::map<std::string, VS_RegGroupInfo>& reg_groups)
{
	UpdateGroupList();
	std::lock_guard<std::mutex> lock(m_reg_groups_lock);
	reg_groups = m_reg_groups;
	return true;
}

VS_UserData::UserRights VS_GroupManager::GroupToUserRights(GroupRights grpRights)
{
	long r =
		VS_UserData::UR_APP_MASK
		| VS_UserData::UR_COMM_PASSWORDMULTI
		| VS_UserData::UR_COMM_MULTI
		| VS_UserData::UR_COMM_EDITAB
		| VS_UserData::UR_COMM_UPDATEAB
		| VS_UserData::UR_COMM_SEARCHEXISTS
		| VS_UserData::UR_COMM_HDVIDEO
		| VS_UserData::UR_COMM_MOBILEPRO
		| VS_UserData::UR_COMM_DIALER
		| VS_UserData::UR_COMM_CHANGEPASSWORD
		| VS_UserData::UR_COMM_EDITGROUP;
	//|VS_UserData::UR_COMM_EDITDIAL; removed by SMK: in 3.4.0 disabled

	if (grpRights&GR_CALL)
		r |= VS_UserData::UR_COMM_CALL;


	if (grpRights&GR_COLLABORATION)
		r |= VS_UserData::UR_COMM_FILETRANSFER
		| VS_UserData::UR_COMM_WHITEBOARD
		| VS_UserData::UR_COMM_SLIDESHOW
		| (VS_UserData::UR_COMM_DSHARING | VS_UserData::UR_COMM_SHARE_CONTROL)
		| VS_UserData::UR_COMM_RECORDING;

	if (grpRights&GR_CREATEMULTI)
		r |= VS_UserData::UR_COMM_BROADCAST | VS_UserData::UR_COMM_CREATEMULTI;

	if (!(grpRights&GR_EDIT_GROUP_AB))
		r &= ~(VS_UserData::UR_COMM_EDITAB|VS_UserData::UR_COMM_EDITGROUP);

	if (grpRights&GR_CHAT)
		r |= VS_UserData::UR_COMM_CHAT;

	return (VS_UserData::UserRights)r;
}

VS_UserData::UserRights VS_GroupManager::GuestRights()
{
	const unsigned long GUEST_RIGHTS = GR_CALL | GR_COLLABORATION | GR_CREATEMULTI | GR_CHAT;
	VS_UserData::UserRights gr = GroupToUserRights((GroupRights)GUEST_RIGHTS);
	gr = (VS_UserData::UserRights)(gr & ~VS_UserData::UR_COMM_SEARCHEXISTS); // exclude simple and directory search

	return gr;
}