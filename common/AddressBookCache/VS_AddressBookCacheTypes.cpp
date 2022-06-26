
#include "VS_AddressBookCacheTypes.h"
#include <direct.h>
#include <time.h>

struct FileCacheHeader
{
	FileCacheHeader() : ver('ch'), type(0), time(0), hash(0), size(0) {}
	const short		ver;	// const
	short			type;	// VS_Cache_Type
	unsigned long	time;	// valid before
	long			hash;	// hash of cache
	unsigned long	size;	// size of cache
};

VS_CacheInterface::VS_CacheInterface(VS_Cache_Type type) : m_type(type)
{
	if		(type==CHT_AB) m_lifeTime = 15*60;
	else if (type==CHT_BAN) m_lifeTime = 15*60;
	else if (type==CHT_DUI) m_lifeTime = 15*60;
	else if (type==CHT_PIC) m_lifeTime = 15*60;
	else if (type==CHT_PHN) m_lifeTime = 15*60;
	m_hash = 0;
	m_ValidBefore = 0;
	m_cacheSize = 0;
	m_IsValid = false;
}

bool VS_CacheInterface::IsValid() const
{
	return m_IsValid;
}

bool VS_CacheInterface::Init(const char* CallId)
{
	if ( !CallId || !*CallId)
		return false;

	VS_SimpleStr file(1024);

	if		(m_type==CHT_AB)
		sprintf(file, "%s\\%s\\ab.che", VS_CACHE_PATH, CallId);
	else if (m_type==CHT_BAN)
		sprintf(file, "%s\\%s\\ban.che", VS_CACHE_PATH, CallId);
	else if (m_type==CHT_DUI)
		sprintf(file, "%s\\%s_dtl.che", VS_CACHE_PATH, CallId);
	else if (m_type==CHT_PIC)
		sprintf(file, "%s\\%s_pic.che", VS_CACHE_PATH, CallId);


	m_fileName = file;
	m_ValidBefore = 0;
	m_hash = 0;
	VS_Container cnt;

	FILE *f(0);// = fopen(m_fileName, "rb");
	if (f) {
		FileCacheHeader hdr;
		if (fread(&hdr, sizeof(FileCacheHeader), 1, f)==1 && hdr.type==m_type) {
			m_hash = hdr.hash;
			m_ValidBefore = hdr.time;
			m_cacheSize = hdr.size;
			if (hdr.size) {
				VS_BinBuff buff;
				buff.SetSize(hdr.size);
				if (fread((void*)buff.Buffer(), 1, hdr.size, f)==hdr.size) {
					if (cnt.Deserialize(buff.Buffer(), buff.Size()))
						DeSerialize(cnt);
				}
			}
		}
		fclose(f);
	}
	m_IsValid = true;
	return m_IsValid;
}

bool VS_CacheInterface::UpdateCache(long hash)
{
	return true; // TODO : fix cache

	m_hash = hash;
	m_ValidBefore = (unsigned long)time(NULL) + m_lifeTime;
	void* buff;
	VS_Container cnt;
	// collect all into container
	Serialize(cnt);
	if (cnt.SerializeAlloc(buff, m_cacheSize)) {
		FILE *f = fopen(m_fileName, "wb");
		if (f) {
			FileCacheHeader hdr;
			hdr.type = m_type;
			hdr.hash = m_hash;
			hdr.time = m_ValidBefore;
			hdr.size = m_cacheSize;
			fwrite(&hdr, sizeof(FileCacheHeader), 1, f);
			fwrite(buff, 1, hdr.size, f);
			fclose(f);
		}
	}
	return true;
}

void VS_CacheInterface::UpdateTime()
{
	return;

	m_ValidBefore = (unsigned long)time(NULL) + m_lifeTime;
	FILE *f = fopen(m_fileName, "r");
	if (f) {
		fclose(f);
		f = fopen(m_fileName, "r+");
	}
	else {
		f = fopen(m_fileName, "wb");
	}
	if (f) {
		FileCacheHeader hdr;
		hdr.type = m_type;
		hdr.hash = m_hash;
		hdr.time = m_ValidBefore;
		hdr.size = m_cacheSize;
		fwrite(&hdr, sizeof(FileCacheHeader), 1, f);
		fclose(f);
	}
}


bool VS_CacheInterface::IsActual()
{
	// temporary disable chache
	return false;

	if (m_ValidBefore==0)
		return false;
	unsigned long ctime = (unsigned long)time(NULL);
	if (m_ValidBefore < ctime)
		return false;
	return true;
}

long VS_CacheInterface::GetHash()
{
	return m_hash;
}


//////////////////////////////////
void VS_AddressBookCache::Serialize(VS_Container &cnt)
{
	if (!IsValid())
		return;
	cnt.Clear();

	cnt.AddValue(HASH_PARAM, m_hash);
	for (VS_MapS::Iterator iter = m_addressBook.Begin(); iter!=m_addressBook.End(); ++iter) {
		cnt.AddValue(CALLID_PARAM, (char*)iter->key);
		cnt.AddValue(DISPLAYNAME_PARAM, (wchar_t*)iter->data);
	}
}

bool VS_AddressBookCache::DeSerialize(VS_Container &cnt)
{
	if (!cnt.IsValid())
		return false;

	m_addressBook.Clear();
	cnt.Reset();
	VS_SimpleStr call_id;

	while (cnt.Next()) {
		if (_stricmp(cnt.GetName(), CALLID_PARAM) == 0) {
			call_id = cnt.GetStrValueRef();
		}
		else if(_stricmp(cnt.GetName(), DISPLAYNAME_PARAM) == 0) {
			VS_WideStr dn;
			dn.AssignUTF8(cnt.GetStrValueRef());
			m_addressBook.Assign(call_id, dn);
		}
	}
	return true;
}

bool VS_AddressBookCache::UpdateAddressBookRecord(const char *call_id, const VS_WideStr &DisplayName, long hash)
{
	if (!call_id || !*call_id)
		return false;
	if (!!DisplayName)
		m_addressBook.Assign(call_id, DisplayName);
	if (hash!=0)
		UpdateCache(hash);
	return true;
}


bool VS_AddressBookCache::RemoveFromAddressBook(const char* call_id, long hash)
{
	if (!call_id || !*call_id)
		return false;
	m_addressBook.Erase(call_id);
	if (hash!=0)
		UpdateCache(hash);
	return true;
}

bool VS_AddressBookCache::GetBook(VS_ABUser *&users, unsigned long &count)
{
	count = m_addressBook.Size();
	if (count==0)
		return false;

	users = new VS_ABUser[count];
	long i = 0;
	for (VS_MapS::Iterator iter = m_addressBook.Begin(); iter!=m_addressBook.End(); ++iter, ++i) {
		users[i].m_callId = (char*)iter->key;
		users[i].m_displayName = (wchar_t*)iter->data;
	}
	return true;
}

bool VS_AddressBookCache::GetUser(const char *call_id, VS_ABUser &user)
{
	if (!call_id || !*call_id)
		return false;
	VS_MapS::Iterator iter = m_addressBook.Find(call_id);
	if (iter!=m_addressBook.End()) {
		user.m_callId = (char*)iter->key;
		user.m_displayName = (wchar_t*)iter->data;
		return true;
	}
	else
		return false;
}

bool VS_AddressBookCache::Clear()
{
	m_addressBook.Clear();
	return true;
}

/////////////////////
void VS_BanListCache::Serialize(VS_Container &cnt)
{
	if (!IsValid())
		return;
	cnt.Clear();

	cnt.AddValue(HASH_PARAM, m_hash);
	for (VS_MapS::Iterator iter = m_banlist.Begin(); iter!=m_banlist.End(); ++iter)
		cnt.AddValue(CALLID_PARAM, (char*)iter->key);
}

bool VS_BanListCache::DeSerialize(VS_Container &cnt)
{
	if (!cnt.IsValid())
		return false;
	m_banlist.Clear();
	cnt.Reset();
	VS_SimpleStr call_id;

	while (cnt.Next()) {
		if (_stricmp(cnt.GetName(), USERNAME_PARAM) == 0) {
			call_id = cnt.GetStrValueRef();
			m_banlist.Assign(call_id, 0);
		}
	}
	return true;
}

bool VS_BanListCache::AddToBanList(const char *call_id, long hash)
{
	if (!call_id || !*call_id)
		return false;
	m_banlist.Assign(call_id, 0);
	UpdateCache(hash);
	return true;
}


bool VS_BanListCache::RemoveFromBanList(const char* call_id, long hash)
{
	if (!call_id || !*call_id)
		return false;
	m_banlist.Erase(call_id);
	UpdateCache(hash);
	return true;
}

bool VS_BanListCache::GetBook(VS_ABUser *&users, unsigned long &count)
{
	count = m_banlist.Size();
	if (count==0)
		return false;

	users = new VS_ABUser[count];
	long i = 0;
	for (VS_MapS::Iterator iter = m_banlist.Begin(); iter!=m_banlist.End(); ++iter, ++i)
		users[i].m_callId = (char*)iter->key;
	return true;
}

bool VS_BanListCache::Clear()
{
	m_banlist.Clear();
	return true;
}

//////////////
void VS_UserPicCache::Serialize(VS_Container &cnt)
{
	if (!IsValid())
		return;
	cnt.Clear();

	cnt.AddValue(HASH_PARAM, m_hash);
	cnt.AddValue("Picture", m_picture.Buffer(), m_picture.Size());
}


bool VS_UserPicCache::DeSerialize(VS_Container &cnt)
{
	if (!cnt.IsValid())
		return false;
	m_picture.Empty();
	//cnt->GetValue("PictureID",m_userPic.m_pictureId);
	//w_str.AssignUTF8(cnt->GetStrValueRef("PictureType"));
	unsigned long size(0);
	const void* pic_buf = cnt.GetBinValueRef("Picture", size);
	m_picture.Set(pic_buf, size);
	return true;
}

//////////////
void VS_UserDetaileCache::Serialize(VS_Container &cnt)
{
	if(!IsValid())
		return;
	cnt.Clear();

	cnt.AddValue(CALLID_PARAM, m_info.m_callId);
	cnt.AddValue("about", m_info.m_about);
	cnt.AddValue("description", m_info.m_description);
	cnt.AddValue("gender", m_info.m_gender);
	cnt.AddValue("marital_status", m_info.m_marital_status);

	FILETIME ft;
	SYSTEMTIME st;
	memset(&st, 0, sizeof(SYSTEMTIME));
	st.wDay = m_info.m_birth_date.m_day;
	st.wMonth = m_info.m_birth_date.m_month;
	st.wYear = m_info.m_birth_date.m_year;
	SystemTimeToFileTime(&st, &ft);
	cnt.AddValue("birth_data", &ft, sizeof(FILETIME));

	cnt.AddValue("occupation", m_info.m_occupation);
	cnt.AddValue("location", m_info.m_location);
	cnt.AddValue("interests", m_info.m_interests);
	cnt.AddValue("country", m_info.m_country);
	cnt.AddValue("avatar_id", m_info.m_avatar_id);
	cnt.AddValue("age", m_info.m_age);
}


bool VS_UserDetaileCache::DeSerialize(VS_Container &cnt)
{
	if (!cnt.IsValid())
		return false;
	m_info.Clear();

	//cnt.PrintF();

	m_info.m_callId = cnt.GetStrValueRef(CALLID_PARAM);
	m_info.m_DisplayName.AssignUTF8(cnt.GetStrValueRef(DISPLAYNAME_PARAM));
	m_info.m_about = cnt.GetStrValueRef("about");
	m_info.m_description.AssignUTF8(cnt.GetStrValueRef("description"));
	m_info.m_gender = cnt.GetStrValueRef("gender");
	m_info.m_marital_status.AssignUTF8(cnt.GetStrValueRef("marital_status"));

	unsigned long time_buf_sz = 0;
	const void *time_buf = cnt.GetBinValueRef("birth_date",time_buf_sz);
	if (time_buf && time_buf_sz == sizeof(FILETIME)) {
		FILETIME *ft = (FILETIME*)&time_buf;
		SYSTEMTIME st;
		FileTimeToSystemTime(ft, &st);
		m_info.m_birth_date.m_day = st.wDay;
		m_info.m_birth_date.m_month = st.wMonth;
		m_info.m_birth_date.m_year = st.wYear;
	}

	m_info.m_occupation.AssignUTF8(cnt.GetStrValueRef("occupation"));
	m_info.m_location.AssignUTF8(cnt.GetStrValueRef("location"));
	m_info.m_interests.AssignUTF8(cnt.GetStrValueRef("interests"));
	m_info.m_country.AssignUTF8(cnt.GetStrValueRef("country"));
	//cnt.GetValue("avatar_id",info.m_avatar_id);
	cnt.GetValue("age",m_info.m_age);
	m_info.m_firstName.AssignUTF8(cnt.GetStrValueRef(FIRSTNAME_PARAM));
	m_info.m_lastName.AssignUTF8(cnt.GetStrValueRef(LASTNAME_PARAM));
	m_info.m_phoneNumber = cnt.GetStrValueRef(USERPHONE_PARAM);
	//here is no phone, look for  SEARCHADDRESSBOOK_METHOD = AB_PHONES
	//if (cnt.GetStrValueRef(USERPHONE_PARAM))
	//{
	//	printf("callId: %s phone number: %s\n",m_info.m_callId,m_info.m_phoneNumber);
	//}


	return true;
}

////////////////////////////

void VS_UserPhoneCache::Serialize(VS_Container &cnt)
{
	if (!IsValid())
		return;
	cnt.Clear();

	cnt.AddValue(HASH_PARAM, m_hash);
	//TODO make serialization here
}


bool VS_UserPhoneCache::DeSerialize(VS_Container &cnt)
{
	if (!cnt.IsValid())
		return false;
	VS_SimpleStr call_id;

	m_phoneAB = new VS_Map;
	//Yulian: should some Constructor-Destructor stuff be added to VS_Map initialization right here?

	VS_ABPhoneNumber * phoneNum = NULL;
	cnt.Reset();
	while (cnt.Next())
	{
		if (_stricmp(cnt.GetName(), CALLID_PARAM) == 0)
		{
			phoneNum->m_callId = cnt.GetStrValueRef();
		}
		else if(_stricmp(cnt.GetName(), USERPHONE_PARAM) == 0)
		{
			phoneNum->m_phone = cnt.GetStrValueRef();
		}
		else if (_stricmp(cnt.GetName(), ID_PARAM) == 0)
		{
			if (phoneNum != NULL)
			{
				m_phoneAB->Insert(phoneNum->m_callId, phoneNum);
			}
			phoneNum = new VS_ABPhoneNumber;
			phoneNum->m_id = cnt.GetStrValueRef();
		}
		else if (_stricmp(cnt.GetName(), TYPE_PARAM) == 0)
		{
			cnt.GetValue(phoneNum->m_type);
		}
		else if (_stricmp(cnt.GetName(), EDITABLE_PARAM) == 0)
		{
			cnt.GetValue(phoneNum->m_isEditable);
		}
		else if (_stricmp(cnt.GetName(), HASH_PARAM) == 0)
		{
			continue;
		}
	}
	//save last entry
    if (phoneNum)
    {
        m_phoneAB->Insert(phoneNum->m_callId, phoneNum);

    }
	//cnt.PrintF();
	return true;
}

void VS_UserPhoneCache::GetPhones( VS_ABPhoneNumber*& numbers, long& count )
{
	count  = m_phoneAB->Size();
	numbers  = new VS_ABPhoneNumber[count];
	int i = 0;
	for (VS_Map::Iterator iter = m_phoneAB->Begin(); iter != m_phoneAB->End(); ++iter)
	{

		if (iter->data)
        {
            numbers[i] = *(static_cast<VS_ABPhoneNumber*>(iter->data));
            i++;
        }
	}
}


////////////////////////////

VS_UserCacheCollect::~VS_UserCacheCollect()
{
	for (VS_MapS::Iterator iter = m_users_detailes.Begin(); iter!=m_users_detailes.End(); iter++) {
		VS_UserDetaileCache * item = (VS_UserDetaileCache *)iter->data;
		delete item;
	}
	m_users_detailes.Clear();
	for(VS_MapS::Iterator iter = m_pic_caches.Begin(); iter!=m_pic_caches.End(); iter++) {
		VS_UserPicCache *item = (VS_UserPicCache *)iter->data;
		delete item;
	}
	m_pic_caches.Clear();
}

VS_UserDetaileCache* VS_UserCacheCollect::GetUserItem(const char *call_id)
{
	if (!call_id) return 0;
	VS_MapS::Iterator iter = m_users_detailes.Find(call_id);
	if (iter != m_users_detailes.End()) {
		return (VS_UserDetaileCache*)iter->data;
	}
	else {
		VS_UserDetaileCache	*user_cache = new VS_UserDetaileCache();
		m_users_detailes.Insert(call_id, user_cache);
		if (!user_cache->Init(call_id))
			return 0;
		else
			return user_cache;
	}
}

VS_UserPicCache *VS_UserCacheCollect::GetPicItem(const char *call_id)
{
	if (!call_id) return 0;
	VS_MapS::Iterator iter = m_pic_caches.Find(call_id);
	if (iter!= m_pic_caches.End()) {
		return (VS_UserPicCache *)iter->data;
	}
	else {
		VS_UserPicCache *pic_cache = new VS_UserPicCache();
		m_pic_caches.Insert(call_id, pic_cache);
		if(!pic_cache->Init(call_id))
			return 0;
		else
			return pic_cache;
	}
}

bool VS_UserCacheCollect::Clear()
{
	m_users_detailes.Clear();
	m_pic_caches.Clear();
	m_global_call_id.Empty();
	return true;
}

bool VS_UserCacheCollect::IsValid()
{
	return !!m_global_call_id;
}


bool VS_UserCacheCollect::Init(const char *call_id)
{
	if (!call_id || !*call_id)
		return false;
	m_global_call_id = call_id;
	return true;
}

////////////////////////






