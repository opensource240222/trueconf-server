#include "VS_ABStorageInterface.h"
#include "BaseServer/Services/storage/VS_DBStorageInterface.h"
#include "BaseServer/Services/VS_AddressBookService.h"		// for SearchAddressBook_Task and AddToAddressBook_Task

#define DEBUG_CURRENT_MODULE VS_DM_RSTOR

bool VS_ABStorageInterface::ManageGroups_CreateGroup(const vs_user_id& owner, const VS_WideStr& gname, long& gid, long& hash)
{
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return false;
	return dbStorage->ManageGroups_CreateGroup(owner, gname, gid, hash);
}

bool VS_ABStorageInterface::ManageGroups_DeleteGroup(const vs_user_id& owner, const long gid, long& hash)
{
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return false;
	return dbStorage->ManageGroups_DeleteGroup(owner, gid, hash);
}

bool VS_ABStorageInterface::ManageGroups_RenameGroup(const vs_user_id& owner, const long gid, const VS_WideStr& gname, long& hash)
{
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return false;
	return dbStorage->ManageGroups_RenameGroup(owner, gid, gname, hash);
}

bool VS_ABStorageInterface::ManageGroups_AddUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash)
{
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return false;
	return dbStorage->ManageGroups_AddUser(owner, gid, call_id, hash);
}

bool VS_ABStorageInterface::ManageGroups_DeleteUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash)
{
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return false;
	return dbStorage->ManageGroups_DeleteUser(owner, gid, call_id, hash);
}

void VS_ABStorageInterface::GetBSEvents(std::vector<BSEvent> &vec)
{
	auto dbStorage = g_dbStorage;
	if(dbStorage)
		dbStorage->GetBSEvents(vec);
}

int VS_ABStorageInterface::AddToAddressBook(VS_AddressBook ab,const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt, VS_SimpleStr& add_call_id, std::string& add_display_name, VS_TransportRouterServiceHelper* srv)
{
	auto dbStorage = g_dbStorage;
    int ret=-1;
	if(dbStorage)
	  ret= dbStorage->AddToAddressBook(ab,user_id1,cnt,hash,rCnt,add_call_id,add_display_name,srv);
    return ret;
}

int VS_ABStorageInterface::UpdateAddressBook(VS_AddressBook ab, const vs_user_id &user_id1, const char *call_id2, VS_Container &cnt, long &hash, VS_Container& rCnt)
{
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return -1;
	return dbStorage->UpdateAddressBook(ab,user_id1,call_id2,cnt,hash,rCnt);
}

int VS_ABStorageInterface::FindUsers(VS_Container &cnt, int &entries, VS_AddressBook ab, const vs_user_id &owner, const std::string &query, long client_hash, VS_Container& in_cnt)
{
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return 0;
	return dbStorage->FindUsers(cnt,entries,ab,owner,query,client_hash,in_cnt);
}
