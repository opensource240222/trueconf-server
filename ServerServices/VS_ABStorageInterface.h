#pragma once

#include "Common.h"
#include "ldap_core/common/Common.h"
#include "std/cpplib/VS_Protocol.h"

#include <vector>

class VS_TransportRouterServiceHelper;

class VS_ABStorageInterface
{
public:
	virtual bool	FindUser(const vs_user_id& id, VS_UserData& user, bool find_by_call_id_only = true) = 0;
	virtual int		AddToAddressBook(VS_AddressBook ab,const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt, VS_SimpleStr& add_call_id, std::string& add_display_name, VS_TransportRouterServiceHelper* srv);
	virtual int		UpdateAddressBook(VS_AddressBook ab, const vs_user_id& user_id1, const char* call_id2, VS_Container& cnt, long& hash, VS_Container& rCnt);

	virtual int		RemoveFromAddressBook(VS_AddressBook ab,const vs_user_id& user_id1,const vs_user_id& user_id2, VS_Container &cnt, long& hash, VS_Container &rCnt) = 0;
	virtual void	GetBSEvents(std::vector<BSEvent> &vec);
	virtual int		FindUsers(VS_Container& cnt, int& entries, VS_AddressBook ab, const vs_user_id& owner, const std::string& query, long client_hash, VS_Container& in_cnt);

	virtual bool 	ManageGroups_CreateGroup(const vs_user_id& owner, const VS_WideStr& gname, long& gid, long& hash);
	virtual bool 	ManageGroups_DeleteGroup(const vs_user_id& owner, const long gid, long& hash);
	virtual bool 	ManageGroups_RenameGroup(const vs_user_id& owner, const long gid, const VS_WideStr& gname, long& hash);
	virtual bool 	ManageGroups_AddUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash);
	virtual bool 	ManageGroups_DeleteUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash);
};
