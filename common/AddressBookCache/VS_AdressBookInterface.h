/**
 **************************************************************************
 * \file VS_AdressBookInterface.h
 * (c) 2002-2008 Visicron Inc.  http://www.visicron.net/
 * \brief Contain interfaces for adress book and user info retriving
 *
 * \b Project AdressBookCache
 * \author SMirnovK
 * \date 26.11.2008
 *
 * $Revision: 4 $
 *
 * $History: VS_AdressBookInterface.h $
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 2.11.09    Time: 16:12
 * Updated in $/VSNA/AddressBookCache
 * - update thread fixed
 *
 * *****************  Version 3  *****************
 * User: Dront78      Date: 27.10.09   Time: 18:29
 * Updated in $/VSNA/AddressBookCache
 * - cache fixed
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 4.12.08    Time: 16:12
 * Updated in $/VSNA/AddressBookCache
 * - ab cache alfa2
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 3.12.08    Time: 12:59
 * Created in $/VSNA/AddressBookCache
 * - cache rewrited, alfa version
 *
 ****************************************************************************/
 #ifndef VS_ADRESS_BOOK_INTERFACE_H
#define VS_ADRESS_BOOK_INTERFACE_H

#include "..\std\cpplib\VS_Container.h"
#include "..\std\cpplib\VS_Protocol.h"
#include "..\std\cpplib\VS_SimpleStr.h"
#include "..\std\cpplib\VS_WideStr.h"

enum VS_AB_Action
{
	ABA_LIST,
	ABA_ADD,
	ABA_REMOVE
};

struct VS_ABUser
{
    VS_SimpleStr	m_callId;
    VS_WideStr		m_displayName;
    VS_WideStr		m_statusText;
    int				m_status;
	VS_BinBuff		m_avatar;
	time_t			m_lastEventTime;
};

struct VS_ABUserBirthDay
{
	VS_ABUserBirthDay();
	unsigned short	m_day;
	unsigned short	m_month;
	unsigned short	m_year;
};

struct VS_ABUserInfo
{
	VS_SimpleStr		m_callId;
	VS_WideStr			m_DisplayName;
	VS_WideStr			m_about;
	VS_WideStr			m_description;
	VS_SimpleStr		m_gender;
	VS_WideStr			m_marital_status;
	VS_WideStr			m_occupation;
	VS_WideStr			m_location;
	VS_WideStr			m_interests;
	VS_WideStr			m_country;
	long				m_avatar_id;
	long				m_age;
	VS_ABUserBirthDay	m_birth_date;
	VS_BinBuff			m_avatar;
	VS_WideStr			m_firstName;
	VS_WideStr			m_lastName;
	VS_SimpleStr		m_phoneNumber;
	void Clear();
};

struct VS_ABPhoneNumber
{
	VS_SimpleStr  m_id;			//ID_PARAM
	VS_SimpleStr  m_callId;		//CALLID_PARAM
	VS_SimpleStr  m_phone;		//USERPHONE_PARAM
	long		  m_type;		//TYPE_PARAM
	bool		  m_isEditable;	//EDITABLE_PARAM
};

class VS_AddressBookCallBackInterface
{
public:
	VS_AddressBookCallBackInterface();
	virtual ~VS_AddressBookCallBackInterface();
	virtual void AddressBook(const VS_ABUser *addr, const unsigned long count, VS_AB_Action action, VS_AddressBook book, long hash, bool update = false) = 0;
	virtual void AddressBookHash(const VS_ABUser *addr, const unsigned long count, VS_AB_Action action, VS_AddressBook book) = 0;
	virtual void ClearBookHash(VS_AddressBook book) = 0;
	virtual void UserDetailes(const VS_ABUserInfo &user_detailes) = 0;
	virtual void PhoneBook(const VS_ABPhoneNumber * phones, unsigned long count) = 0;
};

#endif /*VS_ADRESS_BOOK_INTERFACE_H*/