#pragma once
#include "../std/cpplib/VS_SimpleStr.h"
#include "../std/cpplib/VS_WideStr.h"
#include "../std/cpplib/VS_Container.h"
#include "../std/cpplib/VS_Map.h"
#include "VS_AdressBookInterface.h"

#define VS_CACHE_PATH	".\\ab_cache"

enum VS_Cache_Type
{
	CHT_NONE,
	CHT_AB,
	CHT_BAN,
	CHT_DUI,
	CHT_PIC,
	CHT_PHN
};

class VS_MapS : public VS_Map
{
public:
	VS_MapS() {
		SetPredicate(VS_SimpleStr::Predicate);
		SetKeyFactory(VS_SimpleStr::Factory, VS_SimpleStr::Destructor);
	}
};

class VS_CacheInterface
{
	VS_Cache_Type	m_type;
	unsigned long	m_lifeTime;
	VS_SimpleStr	m_fileName;
	unsigned long	m_ValidBefore;
	unsigned long	m_cacheSize;
	bool			m_IsValid;
protected:
	long			m_hash;
public:
	VS_CacheInterface(VS_Cache_Type type);
	bool Init(const char* CallId);
	bool IsActual();
	long GetHash();
	void UpdateTime();
	bool UpdateCache(long hash);
	virtual bool DeSerialize(VS_Container &cnt) = 0; // the same as parse container method
protected:
	virtual void Serialize(VS_Container &cnt) = 0;

	bool IsValid() const;
};



class VS_AddressBookCache : public VS_CacheInterface
{
	VS_MapS	m_addressBook;
public:
	VS_AddressBookCache(): VS_CacheInterface(CHT_AB) {
		m_addressBook.SetDataFactory(VS_WideStr::Factory, VS_WideStr::Destructor);
	}
	void Serialize(VS_Container &cnt);
	bool DeSerialize(VS_Container &cnt);
	bool UpdateAddressBookRecord(const char * call_id, const VS_WideStr &DisplayName, long hash);
	bool RemoveFromAddressBook(const char *call_id, long hash);
	bool GetBook(VS_ABUser *&users, unsigned long &count);
	bool GetUser(const char *call_id, VS_ABUser &user);
	bool Clear();
};



class VS_BanListCache : public VS_CacheInterface
{
	VS_MapS	m_banlist;
public:
	VS_BanListCache(): VS_CacheInterface(CHT_BAN) {}
	void Serialize(VS_Container &cnt);
	bool DeSerialize(VS_Container &cnt);
	bool AddToBanList(const char * call_id, long hash);
	bool RemoveFromBanList(const char * call_id, long hash);
	bool GetBook(VS_ABUser *&users, unsigned long &count);
	bool Clear();
};


class VS_UserPicCache : public VS_CacheInterface
{
	VS_BinBuff		m_picture;
public:
	VS_UserPicCache() : VS_CacheInterface(CHT_PIC){}
	void Serialize(VS_Container &cnt);
	bool DeSerialize(VS_Container &cnt);
	void GetPic(VS_BinBuff &pic) {pic = m_picture;}
};

class VS_UserDetaileCache : public VS_CacheInterface
{
	VS_ABUserInfo	m_info;
public:
	VS_UserDetaileCache() : VS_CacheInterface(CHT_DUI){}
	void Serialize(VS_Container &cnt);
	bool DeSerialize(VS_Container &cnt);
	void GetDUI(VS_ABUserInfo &info) {info = m_info;}
};

class VS_UserPhoneCache : public VS_CacheInterface
{
	VS_Map * m_phoneAB; //Yulian: using non-heap member caused crash. why?
	int m_count;
public:
	VS_UserPhoneCache() : VS_CacheInterface(CHT_PHN){}
	void Serialize(VS_Container &cnt);
	bool DeSerialize(VS_Container &cnt);
	void GetPhones(VS_ABPhoneNumber*&, long&);
};

class VS_UserCacheCollect
{
	VS_MapS			m_users_detailes;
	VS_MapS			m_pic_caches;
	VS_SimpleStr	m_global_call_id;

	bool IsValid();
public:
	~VS_UserCacheCollect();
	bool Init(const char *call_id); //call_id - чья адресная книга

	VS_UserDetaileCache* GetUserItem(const char *call_id);
	VS_UserPicCache *GetPicItem(const char *call_id);
	bool Clear();
};

