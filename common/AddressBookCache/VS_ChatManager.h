#ifndef VS_CHATMANAGER_H
#define VS_CHATMANAGER_H

#include "VS_AbstractManager.h"

class VS_ChatManager : public VS_AbstractManager
{
private:
    void parseSendCommand(VS_Container &cnt);
    void parseChat(VS_Container &cnt);

protected:
	virtual void OnChat(const char* from, const wchar_t* dn, const wchar_t* mess, const char* to) = 0;
    virtual void OnCommand(const char* from, const char* to, const wchar_t* command) = 0;

public:
	VS_ChatManager();
	virtual ~VS_ChatManager();
	virtual bool ParseIncomimgMessage(VS_Container &cnt);
};

#endif