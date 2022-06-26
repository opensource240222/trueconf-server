#pragma once
#include "../../../ServerServices/VS_ABStorageInterface.h"

class VS_BSABStorage : public VS_ABStorageInterface
{
public:
	VS_BSABStorage(){}
	virtual ~VS_BSABStorage(){}

	virtual bool	FindUser(const vs_user_id& id, VS_UserData& user, bool find_by_call_id_only = true);
	virtual int		RemoveFromAddressBook(VS_AddressBook ab,const vs_user_id& user_id1,const vs_user_id& user_id2, VS_Container &cnt, long& hash, VS_Container &rCnt);
};