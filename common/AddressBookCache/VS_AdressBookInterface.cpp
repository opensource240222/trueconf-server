
#include "VS_AdressBookInterface.h"
#include "VS_AddressBookManager.h"

/////////////
VS_ABUserBirthDay::VS_ABUserBirthDay()
{
	m_day = m_month = m_year = 0;
}

/////////////
void VS_ABUserInfo::Clear()
{
	m_callId.Empty();
	m_DisplayName.Empty();
	m_about.Empty();
	m_description.Empty();
	m_gender.Empty();
	m_marital_status.Empty();
	m_occupation.Empty();
	m_location.Empty();
	m_interests.Empty();
	m_country.Empty();
	m_avatar_id = 0;
	m_age = 0;
	m_birth_date.m_day = m_birth_date.m_month = m_birth_date.m_year = 0;
	m_avatar.Empty();
}


/////////////
VS_AddressBookCallBackInterface::VS_AddressBookCallBackInterface()
{
	VS_AddressBookManager::AddCallBack(this);
}

VS_AddressBookCallBackInterface::~VS_AddressBookCallBackInterface()
{
	VS_AddressBookManager::DelCallBack(this);
}
