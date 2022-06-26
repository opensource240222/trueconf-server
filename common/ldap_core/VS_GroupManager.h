#ifndef VS_GROUP_MANAGER
#define VS_GROUP_MANAGER

#include "common/VS_ABStorage.h"
#include <mutex>

struct VS_GroupManager final : public VS_IABSink{
	bool			m_IsLDAP;
	std::mutex		m_reg_groups_lock;
	std::string		m_last_write_time;
	std::map<std::string, VS_RegGroupInfo> m_reg_groups;

	VS_UserData::UserRights  m_defRights, m_physRights;

	VS_GroupManager(bool IsLDAP) : m_IsLDAP(IsLDAP), m_defRights(VS_UserData::UserRights::UR_NONE), m_physRights(VS_UserData::UserRights::UR_NONE){}

	enum GroupRights
	{
		GR_CALL = 0x01,
		GR_COLLABORATION = 0x02,
		GR_CREATEMULTI = 0x04,
		GR_EDIT_GROUP_AB = 0x08,
		GR_CHAT = 0x10
	};

	void UpdateGroupList();
	virtual bool GetRegGroups(std::map<std::string, VS_RegGroupInfo>& reg_groups) override;
	virtual bool IsLDAP_Sink() const override { return m_IsLDAP; }

	virtual bool GetDisplayName(const vs_user_id& /*user_id*/, std::string& /*display_name*/) override { return false; };
	virtual int  SearchUsers(VS_Container& /*cnt*/, const std::string& /*query*/, VS_Container* /*in_cnt*/) override { return -1; };
	virtual bool FindUser_Sink(const vs_user_id& /*user_id*/, VS_StorageUserData& /*ude*/, bool /*onlyCached*/ = false) override { return false; };
	virtual bool GetAllUsers(std::shared_ptr<VS_AbCommonMap>& /*users*/) override { return false; };
	VS_UserData::UserRights GuestRights();
private:
	VS_UserData::UserRights GroupToUserRights(GroupRights grpRights);

};

#endif /* VS_GROUP_MANAGER */