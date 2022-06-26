/**
 ****************************************************************************
 * Project: server LDAP integration
 *
 ****************************************************************************/
/**
* \file VS_LDAPStorage.cpp
* Server LDAP Storage class definition
*
*/

#ifndef VS_SERVER_LDAP_STORAGE_H
#define VS_SERVER_LDAP_STORAGE_H

#include "AppServer/Services/VS_PresenceService.h"
#include "VCS/Services/VS_ConfMemStorage.h"
#include "ldap_core/common/VS_RegABStorage.h"
#include "ldap_core/VS_LDAPCore.h"
#include "std/cpplib/VS_Map.h"
#include "std/cpplib/VS_MapTpl.h"
#include "std-generic/asio_fwd.h"
#ifndef _WIN32
#include "ldap_core/LdapNTLMAuthorizer.h"
#endif

#ifdef _WIN32
#define LDAP_UNICODE 1
#define SECURITY_WIN32 1

#include <windows.h>
#include <windns.h>
#include <winldap.h>

#ifndef UNICODE
#define UNICODE
#include <security.h>
#undef  UNICODE
#else
#include <security.h>
#endif

typedef VST_Map<VS_SimpleStr, PCtxtHandle> VS_EpCtxtMap;

#endif

#ifdef _SVKS_M_BUILD_
#include "../../common/sudis/unit_solutions/sudis.h"
#endif

#include <map>
#include <vector>
#include <string>
#include <chrono>
#include <boost/regex.hpp>

/**
*  Class for encapsulation of registry storage operations
*/

class VS_LDAPStorage: public VS_SimpleStorage, public VS_IABSink, public VS_PresenceServiceMember, VS_Lock
{
  enum States
  {
	  STATE_CREATED = 0,
	  STATE_RUNNING,
	  STATE_RECONNECT
  };
  States		m_state;
  std::chrono::steady_clock::time_point	m_no_ldap_tick;
  std::chrono::steady_clock::time_point	m_no_ldap_last_try_reconnect_tick;
  std::chrono::seconds		m_timeout_to_reconnect = std::chrono::seconds(30);
  vs::Synchronized<std::vector<VS_Container>> m_log_events;
  void LogEvent(string_view event);

  boost::wregex e;

  std::chrono::system_clock::time_point	m_lastupdate;
  bool          m_useGroups;

	VS_WideStr		m_ldap_att_us;

	static char* m_al_ad_primarygroup[2];
	static char* m_al_dn[2];
	std::shared_ptr<tc::LDAPCore> m_ldapCore;

#ifdef _WIN32	// not ported
  //NTLM auth support
  CredHandle    m_sec_token;
  TimeStamp     m_sec_token_expiry;
  VS_EpCtxtMap  m_sec_ctxt;
  SecPkgInfo*   m_sec_pack;
#else
	std::shared_ptr<tc_ldap::NTLMAuthorizer> m_ntlm_auth;
#endif

#ifdef _SVKS_M_BUILD_
  bool	m_UseSudis;

  class VS_AutoLog
  {
  protected:
	  unsigned long create_tick;
  public:
	  VS_AutoLog(): create_tick(GetTickCount()), result(false) {}

	  bool result;		// true = login_ok, false = login_failed
  };

  class VS_AutoLog_Login: public VS_AutoLog
  {
  public:
	  ~VS_AutoLog_Login();

	  std::string	login;
	  sudis::oid	oid;
	  std::string	error_str;
  };

  class VS_AutoLog_AB: public VS_AutoLog
  {
  public:
	  ~VS_AutoLog_AB();

	  VS_AddressBook ab;
	  sudis::oid	oid;
	  std::string	error_str;
  };
#endif

  int	LoginGuest(const VS_SimpleStr& display_name,const VS_SimpleStr& password, const VS_SimpleStr& appID, VS_SimpleStr& autoKey, const VS_SimpleStr& appServer, VS_UserData& user);
  bool	CheckGuestPassword(const VS_SimpleStr& password);

  int FindUsersPhones(VS_Container& cnt, int& entries, const vs_userpart_escaped& owner, long client_hash);

protected:
	int  GetUserRights(const vs_user_id& id);
	int  GetUserRightsImp(const vs_user_id& id);
	int GetUserRightsImp(const std::map<std::string, VS_RegGroupInfo>& reg_user_groups, bool IsInsideLoginGroup);

//  bool LDAPFetchUserFull(LDAPMessage* lmsg, VS_StorageUserData& user, std::vector<std::pair<std::wstring,std::wstring>> &detailed);
  int  LDAPDoLogin(const vs_user_id& login_,const VS_SimpleStr& password,const VS_SimpleStr& appID, VS_SimpleStr& autoKey, VS_StorageUserData& user, bool check_pass, VS_Container& prop_cnt, const VS_ClientType &client_type = CT_SIMPLE_CLIENT);

  void NTLMRenewToken();

  virtual bool GetParticipantLimit(const vs_user_id& user_id,VS_ParticipantDescription::Type type, int& rights, double& limit, double& decLimit) override;

  int WriteAvatarToLDAP(const std::string &dn, const std::string &fileName, int avatar_size, const std::string &attr);
public:
  VS_LDAPStorage(boost::asio::io_service& ios, const VS_SimpleStr& broker_id,bool useGroups, const std::weak_ptr<VS_TranscoderLogin> &transLogin);
  virtual ~VS_LDAPStorage();
  bool Init(const VS_SimpleStr& broker_id) override;

  /// testable interface
// todo(kt): access to private member of LDAPCore
  virtual bool Test(void) override { return true; }
// 	virtual bool		Test( void )
//	{return m_state==STATE_FAILED?false:true;};


  /// absink interface
  virtual bool GetDisplayName(const vs_user_id& user_id, std::string& display_name) override;
  virtual int  SearchUsers(VS_Container& cnt, const std::string& query, VS_Container* in_cnt) override;
  virtual bool FindUser_Sink(const vs_user_id& user_id, VS_StorageUserData& ude, bool onlyCached = false) override;
  virtual bool GetRegGroups(std::map<std::string, VS_RegGroupInfo>& reg_groups) override;
  virtual bool GetRegGroupUsers(const std::string& gid, std::shared_ptr<VS_AbCommonMap>& users) override;
  virtual bool GetAllUsers(std::shared_ptr<VS_AbCommonMap>& users) override;
  virtual bool GetABForUserImp(const vs_user_id& owner, VS_AbCommonMap& m) override;
  virtual bool IsCacheReady() const override;
  virtual bool IsLDAP_Sink() const override;

  //users
  virtual bool FindUser(const vs_user_id& id, VS_UserData& user, bool find_by_call_id_only = true) override;
  virtual bool FindUser(const vs_user_id& id, VS_StorageUserData& user) override;
  virtual bool FindUserByAlias(const std::string& /*alias*/, VS_StorageUserData& /*user*/) override;
  virtual int  LoginAsUser(const VS_SimpleStr& login,const VS_SimpleStr& password,const VS_SimpleStr& appID, VS_SimpleStr& autoKey, VS_StorageUserData& ude, VS_Container& prop_cnt, const VS_ClientType &client_type = CT_SIMPLE_CLIENT) override;

  /// address book
	virtual int	 FindUsers(VS_Container& cnt, int& entries, VS_AddressBook ab, const vs_user_id& owner, const std::string& query, long client_hash, VS_Container& in_cnt) override;

	virtual bool IsAutoAuthAvailable() override {
		if (m_ldapCore) return m_ldapCore->m_use_ntlm;
		else		 return false;
	}

	virtual bool Authorize(const VS_SimpleStr& ep_id, VS_Container* in_cnt, VS_Container& out_cnt, bool& request, VS_UserData& ud,const VS_ClientType &client_type = CT_SIMPLE_CLIENT) override;

	virtual void SetUserStatus(const VS_SimpleStr& call_id,int status, const VS_ExtendedStatusStorage &extStatus, bool set_server,const VS_SimpleStr& server) override;

protected:
	int FindUserPicture(VS_Container& cnt, int& entries, VS_AddressBook ab, const std::string& query, long client_hash) override;
	int SetUserPicture(VS_Container& cnt, VS_AddressBook ab, const char* callId, long& hash) override;
	int DeleteUserPicture(VS_AddressBook ab, const char* callId, long& hash) override;
public:
  //server
  void CleanUp() override;

  bool FetchRights(const VS_StorageUserData& ud, VS_UserData::UserRights& rights) override;
  VS_UserData::UserRights GetPhysRights();

  bool IsUserMemberOfLoginGroup(const char* user, long primaryGroupId, std::set<std::string> memberOf/*bool &IsNewUser*/);
  void UpdateCacheOnLogin(const VS_StorageUserData &ude);

  void Timer(unsigned long ticks, VS_TransportRouterServiceHelper* caller) override;

	bool GetMissedCallMailTemplate(const std::chrono::system_clock::time_point  missed_call_time, const char * fromId, std::string& inOutFromDn, const char * toId, std::string& inOutToDn, VS_SimpleStr & from_email, VS_SimpleStr & to_email, std::string & subj_templ, std::string & body_templ) override;
	bool GetInviteCallMailTemplate(const std::chrono::system_clock::time_point  missed_call_time, const char *fromId, std::string& inOutFromDn, const char *toId, VS_SimpleStr &from_email, VS_SimpleStr &to_email,  std::string &subj_templ, std::string &body_templ) override;
	bool GetMultiInviteMailTemplate(const char* fromId, std::string& inOutFromDn, const char* toId, std::string& inOutToDn, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ);
	bool GetMissedNamedConfMailTemplate(const char* fromId, std::string& inOutFromDn, const char* toId, std::string& inOutToDn, VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ) override;

	void UpdateUsersGroups(const std::function<bool(void)>& is_stopping) override;
	bool IsLDAP() const;

	bool OnPropertiesChange(const char *pass) override;
#if defined( _DEBUG)
	void SetAuthThreadID() override {
#ifndef _WIN32
		m_ntlm_auth->SetHomeThread();
#endif // !_WIN32
	}
#endif

#ifdef _SVKS_M_BUILD_
	virtual bool AuthByECP(VS_Container& cnt, VS_Container& rCnt, VS_UserData& ud, VS_SimpleStr& autoKey, VS_Container& prop_cnt, VS_ClientType client_type) override;
	void LDAPSetMyServerNameForUser(const char* call_id);
	ldap_error_code_t LDAPGetUserAddressBookAttribute(const vs_user_id& owner, std::vector<std::wstring>& result);
	bool LDAPGetUidAndDisplayName(const std::wstring& dn, std::wstring& uid, std::wstring& display_name);
	bool LDAPGetServerNameByDN(const std::wstring& dn, std::wstring& serverName);
	bool LDAPGetUserDN(const vs_user_id& owner, std::wstring& dn);
	bool LDAPFindMyRegion(const char* our_endpoint, VS_WideStr& my_region_dn);
	virtual int AddToAddressBook(VS_AddressBook ab,const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt, VS_SimpleStr& add_call_id, std::string& add_display_name, VS_TransportRouterService* srv) override;
	virtual int RemoveFromAddressBook(VS_AddressBook ab,const vs_user_id& user_id1,const vs_user_id& user_id2, VS_Container &cnt, long& hash, VS_Container &rCnt) override;
	virtual int	UpdateAddressBook(VS_AddressBook ab, const vs_user_id& user_id1, const char* call_id2, VS_Container& cnt, long& hash, VS_Container& rCnt) override;
	VS_Search_Result FindUsers_CustomGroups(VS_Container& cnt, int& entries, const vs_user_id& owner);
	virtual bool ManageGroups_CreateGroup(const vs_user_id& owner, const VS_WideStr& gname, long& gid, long& hash) override;
	virtual bool ManageGroups_DeleteGroup(const vs_user_id& owner, const long gid, long& hash) override;
	virtual bool ManageGroups_RenameGroup(const vs_user_id& owner, const long gid, const VS_WideStr& gname, long& hash) override;
	virtual bool ManageGroups_AddUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash) override;
	virtual bool ManageGroups_DeleteUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash) override;
private:
	bool CreateCustomGroupsFolder(const std::wstring& dn);
	long GetABGroupsHash(const vs_user_id& owner);
#endif
	virtual bool LogoutUser(const VS_SimpleStr& login) override;

	bool FetchUser(const tc::ldap_user_info& info, VS_StorageUserData& ude);

	void ProcessLDAPError(const tc::ldap_error_code_t error);
	void TryReConnect();

	void ProcessAvatars(const std::string &login, const std::multimap<tc::attr_name_t, tc::attr_value_t> &attributes);
	void CreateAvatarOnDisk(const std::string &user, const std::string &content, int avatarSize, int avatarQuality);
	bool RemoveAvatarOnDisk(const std::string& user);
};


#endif // VS_SERVER_LDAP_STORAGE_H
