/**
 ****************************************************************************
 * Project: server services
 * \author stass
 * \brief Registry address book storage
 ****************************************************************************/

#ifndef VS_SERVER_REGISTRY_AB_STORAGE_H
#define VS_SERVER_REGISTRY_AB_STORAGE_H

#include "VS_ABStorage.h"
#include "VS_LogABLimit_Interface.h"

 class VS_RegistryKey;

class VS_RegABStorage
: public VS_ABStorage
{
private:
	void RemoveDups(VS_RegistryKey& key);
	bool GetABForUserFromGroups(const char* owner, VS_AbCommonMap& m, VS_LogABLimit_Interface* limits = 0);			// reads registry key "GROUPS\_id_\Address Book\", subkeys "Contacts" and "Groups"
	bool GetABForUserFromRegistry(const vs_user_id& owner, VS_AbCommonMap& m, VS_LogABLimit_Interface* limits = 0);			// reads registry key "USERS\_id_\Address Book"
public:

  VS_RegABStorage()
  {};

  bool IsValid() override
  { return true; }


	bool CanEdit() override;

	///address book functions
	virtual int ABFind(VS_Container& cnt, int& entries, VS_AddressBook ab, const char*  owner=0, const std::string& query="", long hash=0, VS_Container* in_cnt=0) override;
	virtual int ABAdd(VS_AddressBook ab, const vs_user_id& user_id1, const char* call_id2, const char* display_name, long& hash, bool IsFromServer=false) override;
  virtual int ABAdd(VS_AddressBook ab, const vs_user_id& user_id1, const char* call_id2, const char* display_name, long& hash, VS_SimpleStr& add_call_id, std::string& add_display_name, bool use_full_name=false, bool IsFromServer=false) override;
	virtual int ABRemove(VS_AddressBook ab, const vs_user_id& user_id1, const vs_user_id& user_id2, long& hash, bool IsFromServer=false) override;
	virtual int ABUpdate(VS_AddressBook ab, const vs_user_id& user_id1, const char* call_id2, VS_Container& cnt, long& hash) override;
	virtual bool GetABForUser(const char* owner, VS_AbCommonMap& m, long& server_hash, VS_LogABLimit_Interface* limits = 0) override;
};

#endif //VS_SERVER_REGISTRY_AB_STORAGE_H
