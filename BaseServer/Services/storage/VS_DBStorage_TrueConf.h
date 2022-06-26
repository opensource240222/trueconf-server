#pragma once
#include "VS_DBStorage.h"

class VS_DBStorage_TrueConf: public VS_DBStorage
{
public:
	VS_DBStorage_TrueConf();
	virtual ~VS_DBStorage_TrueConf();

	virtual bool IsConferendoBS() const;

	virtual void GetSipProviderByCallId(const char* call_id, std::vector<VS_ExternalAccount>& external_accounts, VS_DBObjects* dbo=0);

	virtual int	FindUsers(VS_Container& cnt, int& entries, VS_AddressBook ab, const vs_user_id& owner, const std::string& query, long client_hash, VS_Container& in_cnt);
	virtual int AddToAddressBook(VS_AddressBook ab,const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt, VS_SimpleStr& add_call_id, std::string& add_display_name, VS_TransportRouterServiceHelper* srv);
	virtual int	RemoveFromAddressBook(VS_AddressBook ab,const vs_user_id& user_id1,const vs_user_id& user_id2, VS_Container& cnt, long& hash, VS_Container& rCnt);
	virtual int	UpdateAddressBook(VS_AddressBook ab, const vs_user_id& user_id1, const char* call_id2, VS_Container& cnt, long& hash, VS_Container& rCnt);

	virtual bool ManageGroups_CreateGroup(const vs_user_id& owner, const VS_WideStr& gname, long& gid, long& hash);
	virtual bool ManageGroups_DeleteGroup(const vs_user_id& owner, const long gid, long& hash);
	virtual bool ManageGroups_RenameGroup(const vs_user_id& owner, const long gid, const VS_WideStr& gname, long& hash);
	virtual bool ManageGroups_AddUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash);
	virtual bool ManageGroups_DeleteUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash);

private:
	int FindUsersGroups(const vs_user_id& owner, VS_Container& cnt, int& entries, VS_Container& rCnt);
	int FindUsersPhones(const vs_user_id& owner, VS_Container& cnt, int& entries, VS_Container& rCnt);

	int AddToAddressBook_Phones(VS_AddressBook ab,const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt);
	int UpdateAddressBook_Phones(VS_AddressBook ab,const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt);
	int RemoveFromAddressBook_Phones(VS_AddressBook ab,const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt);

	VS_UserPhoneType VS_UserPhoneType_ToEnum(const char* type);
	const char* VS_UserPhoneType_ToStr(VS_UserPhoneType type);
};
