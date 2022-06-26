/**
****************************************************************************
* Project: server services
*
* ************************************************/

/**
* \file VS_RegistryStorage.h
* Server Registry Storage class definition
*
*/
#ifndef VS_SERVER_REGISTRY_STORAGE_H
#define VS_SERVER_REGISTRY_STORAGE_H

#include "VS_ConfMemStorage.h"


/**
*  Class for encapsulation of registry storage operations
*/

class VS_RegistryStorage : public VS_SimpleStorage, public VS_IABSink
{
public:
	typedef VS_StrIStrMap LoginMap;
private:
	long          m_hash;
	bool          m_useGroups;
	std::string	  m_Company;

	void		UpdateUserData(const VS_StorageUserData& realLogin);
	void GetGroupsUsers(std::map<std::string, std::vector<VS_RealUserLogin>>& groups_users);

	int FindUsersPhones(VS_Container& cnt, int& entries, const vs_user_id& owner, long client_hash);

protected:
	bool Read (VS_RegistryKey &reg_user,VS_StorageUserData&   user);
	bool Read (VS_RegistryKey &reg_user,VS_StorageUserData&   user, std::map<std::string, std::vector<VS_RealUserLogin>>& groups_users);
	long UpdatePerson(const char* call_id, VS_Container& cnt, const char *fields);
	std::string GetDefauldEditableFields() override;
	virtual bool GetParticipantLimit(const vs_user_id& user_id,VS_ParticipantDescription::Type type, int& rights, double& limit, double& decLimit) override;

public:
	virtual bool FetchRights(const VS_StorageUserData& user, VS_UserData::UserRights& rights) override;
	VS_UserData::UserRights GetPhysRights();

	VS_RegistryStorage(VS_ABStorage* ab_storage, bool useGroups,const VS_SimpleStr& broker_id, const std::weak_ptr<VS_TranscoderLogin> &transLogin);
	virtual ~VS_RegistryStorage();

	bool Init(const VS_SimpleStr& broker_id) override;

	//testable interface
	virtual bool Test( void ) override {return true;}

	// absink interface
	virtual bool GetDisplayName(const vs_user_id& user_id, std::string& display_name) override;
	virtual int  SearchUsers(VS_Container& cnt, const std::string& query, VS_Container* in_cnt) override;
	virtual bool FindUser_Sink(const vs_user_id& user_id, VS_StorageUserData& ude, bool /*onlyCached*/ = false) override
	{ return FindUser(user_id.m_str, ude); }
	virtual bool GetRegGroups(std::map<std::string, VS_RegGroupInfo>& reg_groups) override;
	virtual bool GetRegGroupUsers(const std::string& gid, std::shared_ptr<VS_AbCommonMap>& users) override;
	virtual bool GetAllUsers(std::shared_ptr<VS_AbCommonMap>& users) override;
	virtual bool IsCacheReady() const override;
	virtual bool IsLDAP_Sink() const override;

	// log
	virtual bool LogParticipantLeave (const VS_ParticipantDescription& pd) override;

	//users
	virtual bool FindUser(const vs_user_id& id, VS_UserData& user, bool find_by_call_id_only = true) override;
	virtual bool FindUser(const vs_user_id& id, VS_StorageUserData& user) override;
	virtual bool FindUserByAlias(const std::string& /*alias*/, VS_StorageUserData& /*user*/) override;

	int GetUserRights(const vs_user_id& id);
	virtual bool DeleteUser(const vs_user_id& id) override;

	virtual void SetUserStatus(const VS_SimpleStr& call_id,int status, const VS_ExtendedStatusStorage &extStatus, bool set_server,const VS_SimpleStr& server) override;

	//address book
	virtual int	 FindUsers(VS_Container& cnt, int& entries, VS_AddressBook ab, const vs_user_id& owner, const std::string& query, long client_hash, VS_Container& in_cnt) override;

	//SBS specifics
	virtual bool OnUserChange(const char* user, long type, const char* pass) override;

	int LoginAsUser(const VS_SimpleStr& login,const VS_SimpleStr& password, const VS_SimpleStr& appID, VS_SimpleStr& autoKey, VS_StorageUserData& ude, VS_Container& prop_cnt, const VS_ClientType &client_type = CT_SIMPLE_CLIENT) override;

    bool GetMissedCallMailTemplate(const std::chrono::system_clock::time_point  missed_call_time, const char * fromId, std::string& inOutFromDn, const char * toId, std::string& inOutToDn, VS_SimpleStr & from_email, VS_SimpleStr & to_email, std::string & subj_templ, std::string & body_templ) override;
    bool GetInviteCallMailTemplate(const std::chrono::system_clock::time_point  missed_call_time, const char *fromId, std::string& inOutFromDn, const char *toId, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ) override;
    bool GetMultiInviteMailTemplate(const char* fromId, std::string& inOutFromDn, const char* toId, std::string& inOutToDn, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ);
    bool GetMissedNamedConfMailTemplate(const char* fromId, std::string& inOutFromDn, const char* toId, std::string& inOutToDn, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ) override;

	bool IsDisabledUser(const char* user) override;

	virtual long ChangePassword(const char* call_id, const char* old_pass, const char* new_pass, const VS_SimpleStr& from_app_id) override;
	virtual long UpdatePerson(const char* call_id, VS_Container& cnt) override;

	vs::fast_mutex m_common_lock;
};


#endif
