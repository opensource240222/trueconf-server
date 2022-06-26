/**
 **************************************************************************
 * \file VS_AddressBookManager.h
 * (c) 2002-2008 Visicron Inc.  http://www.visicron.net/
 * \brief adress book manager
 *
 * \b Project AdressBookCache
 * \author SMirnovK, Ushakov
 * \date 04.12.2008
 *
 * $Revision: 14 $
 *
 * $History: VS_AddressBookManager.h $
 *
 * *****************  Version 14  *****************
 * User: Dront78      Date: 3.12.09    Time: 11:13
 * Updated in $/VSNA/AddressBookCache
 * - code cleanup vzochat7
 *
 * *****************  Version 13  *****************
 * User: Dront78      Date: 2.11.09    Time: 16:12
 * Updated in $/VSNA/AddressBookCache
 * - update thread fixed
 *
 * *****************  Version 12  *****************
 * User: Dront78      Date: 27.10.09   Time: 18:29
 * Updated in $/VSNA/AddressBookCache
 * - cache fixed
 *
 * *****************  Version 11  *****************
 * User: Dront78      Date: 5.06.09    Time: 18:12
 * Updated in $/VSNA/AddressBookCache
 * - added missed calls
 * - fixed banlist
 * - bugfixes
 *
 * *****************  Version 10  *****************
 * User: Dront78      Date: 27.01.09   Time: 15:57
 * Updated in $/VSNA/AddressBookCache
 *
 * *****************  Version 9  *****************
 * User: Dront78      Date: 13.01.09   Time: 19:14
 * Updated in $/VSNA/AddressBookCache
 * - chat fixed
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 25.12.08   Time: 18:38
 * Updated in $/VSNA/AddressBookCache
 * - added chat
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 18.12.08   Time: 18:13
 * Updated in $/VSNA/AddressBookCache
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 8.12.08    Time: 20:52
 * Updated in $/VSNA/AddressBookCache
 * - statuses added
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 4.12.08    Time: 21:10
 * Updated in $/VSNA/AddressBookCache
 * - ab cache beta
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 4.12.08    Time: 16:12
 * Updated in $/VSNA/AddressBookCache
 * - ab cache alfa2
 *
 ****************************************************************************/
#ifndef VS_ADDRESS_BOOK_MANAGER_H
#define VS_ADDRESS_BOOK_MANAGER_H

#include "VS_AdressBookInterface.h"
#include "VS_AddressBookCacheTypes.h"
#include "../VSClient/VSTrClientProc.h"

class VS_AddressBookManager
{
	bool					m_isValid;
	VS_SimpleStr			m_call_id;
	VS_SimpleStr			m_server;
	VS_MapS					m_statuses;
	VS_MapS					m_subs;

	VS_AddressBookCache		m_addr_book;
	VS_BanListCache			m_ban_list;
	VS_UserPhoneCache		m_phone_book;
	VS_UserCacheCollect		m_users;
	static VS_AddressBookCallBackInterface	*m_call_back;
public:
	static void AddCallBack(VS_AddressBookCallBackInterface	*call_back) {m_call_back = call_back;}
	static void DelCallBack(VS_AddressBookCallBackInterface	*call_back) {m_call_back = 0;}
private:

	void NotifyAddressBook(const VS_AddressBook ab, long hash);
	void NotifyAddressBookHash(const VS_AddressBook ab);
	void NotifyAddressBookError(const long error, const VS_AddressBook ab);
	void NotifyAddressBookUser(const char *call_id, const wchar_t *dn, const VS_AB_Action action, const long ab, long hash, bool update = false);
	void NotifyUserDetailes(const char *call_id);
	void NotifyUserPicture(const char *call_id, long hash);
	void NotifyUserPictureHash(const char *call_id);
	//void NotifyPhoneBook();
public:
	VS_AddressBookManager();
	virtual ~VS_AddressBookManager();
	bool Init(const char *call_id, const char *server);
	bool IsValid() const;
	void ParseIncomimgMessage(VS_Container &cnt);
	// public queries
	bool GetAddressBook();
	bool GetAddressBook(int hash);
	bool GetBanList();
	bool GetBanList(int hash);
	bool GetMissedCalls();
	bool GetMissedCalls(int hash);
	bool GetUserDetailes(const char *call_id);
	bool GetUserPicture(const char *call_id);
	bool GetUserPicture(const char *call_id, int hash);
	bool GetPhones();
	long GetStatus(const char *call_id);
	void Subscribe(VS_ABUser *users, int num, bool plus);
	bool SetDisplayName(const char *call_id, const wchar_t *dm);
	bool SetAvatar(const void *pic_buf, const unsigned long sz); //only for myself
	bool AddToAddressBook(const char * call_id);
	bool AddToBanList(const char * call_id);
	bool RemoveFromAddressBook(const char *call_id);
	bool RemoveFromBanList(const char *call_id);
	bool ChatSend(wchar_t *Message, char * To);
	bool UserSearch(const char *query);
	bool Clear();

private:
	unsigned long ComposeSend(VS_Container &cnt, const char* service = ADDRESSBOOK_SRV, const char *user = 0, const char *server = 0);
	unsigned long SendUserQuery(const char* user, const char* method, long addressBook, const char* query, long hash = 0);
};

#endif /*VS_ADDRESS_BOOK_MANAGER_H*/