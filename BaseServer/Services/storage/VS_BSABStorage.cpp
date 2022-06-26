#ifdef _WIN32	// not ported
#include "VS_DBStorage.h"
#include "VS_BSABStorage.h"

bool VS_BSABStorage::FindUser(const vs_user_id &id, VS_UserData &user, bool find_by_call_id_only)
{
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return false;
	return dbStorage->FindUser(id,user,find_by_call_id_only);
}

int VS_BSABStorage::RemoveFromAddressBook(VS_AddressBook ab, const vs_user_id &user_id1, const vs_user_id &user_id2, VS_Container &cnt, long &hash, VS_Container &rCnt)
{
	auto dbStorage = g_dbStorage;
	if(!dbStorage)
		return -1;
	return dbStorage->RemoveFromAddressBook(ab,user_id1,user_id2,cnt,hash,rCnt);
}
#endif