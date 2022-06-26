#pragma once

#include "std-generic/cpplib/string_view.h"

#include <RTSPServer.hh>

#include <string>

class VS_Live555RTSPServer : public RTSPServer
{
	class ClientSession;
public:
	static VS_Live555RTSPServer* createNew(UsageEnvironment& env, Port& ourPort, UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds, string_view secret);

private:
	VS_Live555RTSPServer(UsageEnvironment& env, int ourSocket, Port& ourPort, UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds, string_view secret);

	RTSPServer::RTSPClientConnection* createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr) override;

	ServerMediaSession* lookupServerMediaSession(char const* streamName, Boolean isFirstLookupInSession) override;
	void removeServerMediaSession(ServerMediaSession* serverMediaSession) override;
	void closeAllClientSessionsForServerMediaSession(ServerMediaSession* serverMediaSession) override;

	RTSPClientSession* createNewClientSession(u_int32_t sessionId) override;

	std::string m_secret;
};
