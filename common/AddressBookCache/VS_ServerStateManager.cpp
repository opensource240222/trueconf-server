#include "VS_ServerStateManager.h"

VS_ServerStateManager::VS_ServerStateManager()
{
}

VS_ServerStateManager::~VS_ServerStateManager()
{
}

bool VS_ServerStateManager::ParseIncomimgMessage(VS_Container &cnt)
{
	if (!cnt.IsValid()) return false;
	char server[256] = {}; unsigned long size(255);
	cnt.GetValue("server", server, size);
	if (*server) {
		OnConnect(server);
	} else {
		OnDisconnect();
	}
	return true;
}

void VS_ServerStateManager::DisconnectWithError( long error )
{
	OnDisconnect(error);
}

