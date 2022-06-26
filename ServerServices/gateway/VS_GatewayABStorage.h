#pragma once
#include "VCS/Services/VS_VCSABStorage.h"

#include "std-generic/compat/map.h"
#include <mutex>

class VS_GatewayABStorage : public VS_VCSABStorage
{
	std::mutex	m_map_lock;
	vs::map<std::string, long, vs::str_less> m_users_hashes;
	bool IsMyCallId(string_view call_id) const;
	int FindGatewayUsers(VS_Container &cnt, int &entries, VS_AddressBook ab, const vs_user_id &owner, const std::string &query, long client_hash, VS_Container& in_cnt);
public:
	VS_GatewayABStorage(){}
	virtual ~VS_GatewayABStorage(){}
	/*virtual bool	FindUser(const vs_user_id& id, VS_UserData& user, bool find_by_call_id_only = true);
	virtual int		UpdateAddressBook(VS_AddressBook ab, const vs_user_id& user_id1, const char* call_id2, VS_Container& cnt, long& hash);
	virtual int		RemoveFromAddressBook(VS_AddressBook ab,const vs_user_id& user_id1,const vs_user_id& user_id2, long& hash);
	virtual void	GetBSEvents(std::vector<BSEvent> &vec);*/
	virtual int		FindUsers(VS_Container &cnt, int &entries, VS_AddressBook ab, const vs_user_id &owner, const std::string &query, long client_hash, VS_Container& in_cnt);
};
