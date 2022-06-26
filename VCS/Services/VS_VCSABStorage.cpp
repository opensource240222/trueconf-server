#include "VS_VCSABStorage.h"
#include "../../BaseServer/Services/storage/VS_DBStorageInterface.h"


VS_VCSABStorage::VS_VCSABStorage()
{}

bool VS_VCSABStorage::FindUser(const vs_user_id &id, VS_UserData &user, bool /*find_by_call_id_only*/)
{
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return false;
	return dbStorage->FindUser(id,user);
}

int VS_VCSABStorage::RemoveFromAddressBook(VS_AddressBook ab, const vs_user_id &user_id1, const vs_user_id &user_id2, VS_Container &cnt, long &hash, VS_Container &rCnt)
{
	auto dbStorage = g_dbStorage;
int ret=-1;
	if(dbStorage)
	  ret= dbStorage->RemoveFromAddressBook(ab,user_id1,user_id2,cnt,hash,rCnt);
    return ret;
}