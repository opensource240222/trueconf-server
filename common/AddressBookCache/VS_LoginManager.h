#ifndef VS_LOGINMANAGER_H
#define VS_LOGINMANAGER_H

#include "VS_AbstractManager.h"
#include <string>

class VS_LoginManager : public VS_AbstractManager
{
private:
	VS_SimpleStr m_AppId;
	VS_Container m_LoginRetryCnt;
	std::string m_user;
	std::string m_password;

protected:
	virtual void OnLogin(const char *user, long result) = 0;
	virtual void OnLogout(long result) = 0;

public:
	VS_LoginManager();
	virtual ~VS_LoginManager();
	unsigned long Login(const char* user, const char* password);
	unsigned long Logout(bool clearAutoLogin = false);
	unsigned long AutoLogin();
	unsigned long RetryLastLogin();
	const char *GetCurrentUser();
	virtual bool ParseIncomimgMessage(VS_Container &cnt);
};

#endif