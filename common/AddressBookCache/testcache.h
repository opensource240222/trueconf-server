#ifndef TESTCACHE_H
#define TESTCACHE_H

class VS_TestAbInterface
{
public:
	virtual ~VS_TestAbInterface();
	// login srv
	//void Login(char* user, char* pass);
	//void Logout();
	// public ab queries
	bool GetAddressBook();
	bool GetBanList();
	bool GetUserDetailes(const char *call_id);
	bool GetUserPicture(const char *call_id);
	long GetStatus(const char *call_id);
	bool SetDisplayName(const char *call_id, const wchar_t *dm);
	bool SetAvatar(const void *pic_buf, const unsigned long sz); //only for myself
	bool AddToAddressBook(const char * call_id);
	bool AddToBanList(const char * call_id);
	bool RemoveFromAddressBook(const char *call_id);
	bool RemoveFromBanList(const char *call_id);
	// chat
	bool ChatSend(wchar_t *Message, char * To);
	// database search
	bool UserSearch(const char *query);
};

#endif