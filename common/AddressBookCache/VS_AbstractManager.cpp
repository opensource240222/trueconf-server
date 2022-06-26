#include "VS_AbstractManager.h"

VS_AbstractManager::VS_AbstractManager()
{
}

VS_AbstractManager::~VS_AbstractManager()
{
}

unsigned long VS_AbstractManager::ComposeSend(VS_Container &cnt, const char* service, const char *user, const char *server)
{
	unsigned long bodySize;	void *body;
	if (cnt.SerializeAlloc(body, bodySize)) {
		VS_ClientMessage tMsg(service, VSTR_PROTOCOL_VER, 0, service, 20000, body, bodySize, user, 0, server ? server : m_server);
		free(body);
		return tMsg.Send();
	}
	else
		return 0;
}

bool VS_AbstractManager::Is(const char *method, const char *whatIs)
{
	return (method && whatIs && (_stricmp(method, whatIs) == 0));
}

void VS_AbstractManager::SetHomeServer(const char *server)
{
	m_server = server;
}