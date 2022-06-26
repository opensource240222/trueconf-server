/**
 ****************************************************************************
 * Project: server services
 * \brief In-memory read-only  AB storage
 *
 ****************************************************************************/

#ifndef VS_SERVER_READ_ONLY_AB_STORAGE_H
#define VS_SERVER_READ_ONLY_AB_STORAGE_H

#include "VS_RegistryStorage.h"
#include "../../common/ldap_core/common/VS_ABStorage.h"



class VS_ReadOnlyABStorage
: public  VS_ABStorage
{
  vs::Synchronized<VS_RegistryStorage::UserMap, std::recursive_mutex>* m_ab_users;
  long* m_hash;

public:

  VS_ReadOnlyABStorage()
    :m_ab_users(0),m_hash(0)
  {};

  bool IsValid()
  { return m_ab_users!=0;  };

  bool CanEdit()
  { return false; }
  bool InMemory()
  { return true;  }

  ///address book functions
	virtual int ABFind(VS_Container& cnt, int& entries, VS_AddressBook ab, const char*  owner=nullptr, const std::string& query = "", long hash=0, VS_Container* in_cnt=0) override;

  void SetUsers(vs::Synchronized<VS_RegistryStorage::UserMap,std::recursive_mutex>* users,long* phash)
  {
    m_ab_users=users;
    m_hash=phash;
  };
};

#endif //VS_SERVER_READ_ONLY_AB_STORAGE_H
