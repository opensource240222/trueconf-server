#pragma once

#include "../VS_FakeEndpointBase.h"
#include "../VS_FakeEndpointManager.h"

class VS_Container;

class VS_FakeClientTest : public VS_FakeEndpointBase
{
public:
	VS_FakeClientTest();
	~VS_FakeClientTest();

	static void StartTesting();

	void onError(unsigned int err);
	void onReceive(VS_ClientMessage &msg);

private:
	HANDLE m_wait;
	std::string m_logged_in_name, m_tmp;
	std::string m_conference;

	std::string m_action_list;

	void LoginUser(const char *name, const char *pass);
	void Logout();
	void SendChatMessage(const char *to_user, const char *text);

	void onLoginResponse(VS_Container &cnt);
	void onLogoutResponse(VS_Container &cnt);

	void ComposeSend(VS_Container &cnt, const char* service, const char* server, const char *user, unsigned long timeout);

	static void __cdecl StartTestingPrivate(void *);

	void StartTesting(const char *action_list);
	bool PerformActionSynchronously( std::string &action);
	bool WaitForResult(const std::string & action );

	void Timeout(){}
};