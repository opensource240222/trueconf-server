/**
 ****************************************************************************
 * Project: server services
 * \brief Base for AB Storage
 *
 ****************************************************************************/

#ifndef VS_SERVER_AB_STORAGE_H
#define VS_SERVER_AB_STORAGE_H

#include "Common.h"
#include "VS_LogABLimit_Interface.h"

#include <boost/algorithm/string/predicate.hpp>

#include <memory>

struct ci_less: std::binary_function<std::string,std::string,bool>
{
	bool operator() (const std::string& s1,const std::string& s2) const {
		return  boost::ilexicographical_compare(s1,s2);
	}
};

class VS_AbCommonMap_Item
{
public:
	VS_AbCommonMap_Item() : IsCustomContactOfGroup(false), IsCustomContactOfUser(false)
	{
	}
	VS_AbCommonMap_Item(const std::string& displayName_) : displayName(displayName_), IsCustomContactOfGroup(false), IsCustomContactOfUser(false)
	{
	}
	std::string displayName;
	bool IsCustomContactOfGroup;
	bool IsCustomContactOfUser;

#ifdef _SVKS_M_BUILD_
	VS_AbCommonMap_Item(const std::wstring& displayName_, const std::wstring& mvdPosition_, const std::wstring& ou_): displayName(displayName_), mvdPosition(mvdPosition_), ou(ou_)
	{
	}
	std::wstring mvdPosition;
	std::wstring ou;
#endif
};

typedef std::map<std::string, VS_AbCommonMap_Item, ci_less> VS_AbCommonMap;

class VS_IABSink
{
public:
  virtual bool GetDisplayName(const vs_user_id& user_id,std::string& display_name)=0;
  virtual int  SearchUsers(VS_Container& cnt, const std::string& query,VS_Container* in_cnt)=0;
  virtual bool FindUser_Sink(const vs_user_id& user_id, VS_StorageUserData& ude, bool onlyCached = false) = 0;
  virtual bool GetRegGroups(std::map<std::string, VS_RegGroupInfo>& reg_groups)=0;
  virtual bool GetAllUsers(std::shared_ptr<VS_AbCommonMap>& users) = 0;
  virtual bool GetABForUserImp(const vs_user_id& /*owner*/, VS_AbCommonMap& /*m*/) { return false; }
  virtual bool IsCacheReady() const
  {	  return true;	}
  virtual bool IsLDAP_Sink() const = 0;
};

struct VS_IABSink_GetRegGroupUsers
{
	virtual bool GetRegGroupUsers(const std::string& gid, std::shared_ptr<VS_AbCommonMap>& users) = 0;
};

class VS_ABStorage
{

protected:
  VS_IABSink* m_sink;
  VS_IABSink_GetRegGroupUsers* m_sink2;

public:

  void SetSink(VS_IABSink* sink, VS_IABSink_GetRegGroupUsers* sink2)
  {
	  m_sink = sink;
	  m_sink2 = sink2;
  }

  virtual bool IsValid()=0;
  virtual bool CanEdit()=0;
  virtual bool InMemory()
  {
    return false;
  };
  virtual ~VS_ABStorage() {}


	///address book functions
	virtual int	    ABFind(VS_Container& cnt, int& entries, VS_AddressBook ab,const char* owner=nullptr, const std::string& query="", long hash=0, VS_Container* in_cnt=0)=0;
	virtual int     ABAdd(VS_AddressBook /*ab*/,const vs_user_id& /*user_id1*/, const char* /*call_id2*/, const char* /*display_name*/, long& /*hash*/, bool /*IsFromServer*/=false)
	{ return -1; };
	virtual int     ABAdd(VS_AddressBook /*ab*/,const vs_user_id& /*user_id1*/, const char* /*call_id2*/, const char* /*display_name*/, long& /*hash*/, VS_SimpleStr& /*add_call_id*/,std::string& /*add_display_name*/, bool /*use_full_name*/=false, bool /*IsFromServer*/=false)
	{ return -1; };
	virtual int     ABRemove(VS_AddressBook /*ab*/,const vs_user_id& /*user_id1*/,const vs_user_id& /*user_id2*/, long& /*hash*/, bool /*IsFromServer*/=false)
	{ return -1; };
	virtual int     ABUpdate(VS_AddressBook /*ab*/, const vs_user_id& /*user_id1*/, const char* /*call_id2*/, VS_Container& /*cnt*/, long& /*hash*/)
	{ return -1; };
	virtual bool GetABForUser(const char* /*owner*/, VS_AbCommonMap& /*m*/, long& /*server_hash*/, VS_LogABLimit_Interface* /*limits*/ = 0)
	{ return false; };
};

#endif //VS_SERVER_AB_STORAGE_H
