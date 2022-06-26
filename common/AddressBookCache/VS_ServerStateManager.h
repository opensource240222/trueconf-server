#ifndef VS_SERVERSTATEMANAGER_H
#define VS_SERVERSTATEMANAGER_H

#include "VS_AbstractManager.h"

class VS_ServerStateManager : public VS_AbstractManager
{
protected:
	virtual void OnConnect(const char *server) = 0;
	virtual void OnDisconnect(long error = 0) = 0;

public:
	VS_ServerStateManager();
	virtual ~VS_ServerStateManager();
	virtual bool ParseIncomimgMessage(VS_Container &cnt);
	virtual void DisconnectWithError(long);
};

#endif