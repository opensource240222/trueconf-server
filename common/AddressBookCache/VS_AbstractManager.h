#ifndef VS_ABSTRACTMANAGER_H
#define VS_ABSTRACTMANAGER_H

#include "../std/cpplib/VS_Container.h"
#include "../std/cpplib/VS_Protocol.h"
#include "../std/cpplib/VS_SimpleStr.h"
#include "../std/cpplib/VS_WideStr.h"
#include "../VSClient/VSTrClientProc.h"
#include "../VSClient/VS_Dmodule.h"

class VS_AbstractManager
{
protected:
	VS_SimpleStr m_server;
	unsigned long ComposeSend(VS_Container &cnt, const char* service, const char *user = 0, const char *server = 0);
	bool Is(const char *method, const char *whatIs);
	virtual bool ParseIncomimgMessage(VS_Container &cnt) = 0;

public:
	VS_AbstractManager();
	virtual ~VS_AbstractManager();
	void SetHomeServer(const char *server);
};

#endif