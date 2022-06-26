#include "VS_FakeClientTest.h"
#include "../../std/cpplib/VS_Container.h"
#include "std/cpplib/md5.h"
#include "../../transport/Client/VS_TransportClient.h"
#include "../../VSClient/VSTrClientProc.h"

#include <process.h>

#define SEND_TIMEOUT 20*1000
#define  LOG printf

static char *test_actions[] = {
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 7 7;PAUSE 5000;LOGOUT;PAUSE 10000;",
	"PAUSE 10000;CONNECT;LOGIN 2 2;PAUSE 1000;DISCONNECT;",

	"PAUSE 10000;CONNECT;LOGIN 3 3;PAUSE 5000;CHAT light_keeper@trueconf.com hello!;PAUSE 10000;"
			"LOGOUT;PAUSE 5000;LOGIN 4 4;CHAT light_keeper@trueconf.com hello_from_4!!;PAUSE 20000;DISCONNECT;"
};

VS_FakeClientTest::VS_FakeClientTest()
{
	LOG("call VS_FakeClientTest::VS_FakeClientTest()\n");
	m_wait = CreateEvent(0,0,0,0);
}

VS_FakeClientTest::~VS_FakeClientTest()
{
	LOG("call VS_FakeClientTest::~VS_FakeClientTest() for %s\n", getCID());
	CloseHandle(m_wait);
}

void VS_FakeClientTest::ComposeSend(VS_Container &cnt, const char* service,
									const char* server = 0, const char *user = 0, unsigned long timeout = SEND_TIMEOUT)
{
	unsigned long bodySize;	void *body;
	if (cnt.SerializeAlloc(body, bodySize)) {
		VS_ClientMessage tMsg(service, VSTR_PROTOCOL_VER, 0, service, timeout, body, bodySize, user, 0, server);
		this->Send( tMsg );
		free(body);
	}
}

void VS_FakeClientTest::LoginUser( const char *name, const char *pass )
{
	char md5_result[100];
	VS_ConvertToMD5(pass, md5_result);

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, LOGINUSER_METHOD);
	cnt.AddValue(LOGIN_PARAM, name);
	cnt.AddValue(PASSWORD_PARAM, md5_result);
	cnt.AddValue(KEY_PARAM, "");
	cnt.AddValue(HASH_PARAM, "TestAppId");
	cnt.AddValue(CLIENTTYPE_PARAM, (long)CT_SIMPLE_CLIENT);
	cnt.AddValue(USER_DEFAULT_DOMAIN, "");
	cnt.AddValue(APPNAME_PARAM, "test");

	ResetEvent(m_wait);
	ComposeSend(cnt, AUTH_SRV);
}


void VS_FakeClientTest::SendChatMessage(const char *to_user, const char *text)
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, SENDMESSAGE_METHOD);
	rCnt.AddValue(FROM_PARAM, m_logged_in_name);
	rCnt.AddValue(DISPLAYNAME_PARAM, m_logged_in_name);
	rCnt.AddValue(MESSAGE_PARAM, text);

	if (to_user) {
		rCnt.AddValue(TO_PARAM, to_user);
		ComposeSend(rCnt, CHAT_SRV, 0, to_user);
	}
	else if (!m_conference.empty()) {
		rCnt.AddValue(CONFERENCE_PARAM, m_conference);
//		ComposeSend(rCnt, CHAT_SRV, ReturnBroker(m_Status.CurrConfInfo->Conference));
	}
}

void VS_FakeClientTest::Logout()
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, LOGOUTUSER_METHOD);
	ComposeSend(rCnt, AUTH_SRV);
}

void VS_FakeClientTest::onError( unsigned err )
{
	LOG("onError from IO err = %u\n", err);
}

void VS_FakeClientTest::onReceive( VS_ClientMessage &msg )
{
	LOG("onReceive\n");
	if ( strcmp(msg.DstService(), AUTH_SRV) == NULL )
	{
		void *data;
		unsigned size = msg.Body( &data );

		VS_Container cnt;
		if (cnt.Deserialize(data, size))
		{
			const char* method = 0;
			if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0)
			{
				if (_stricmp(method, USERLOGGEDIN_METHOD) == 0) {
					onLoginResponse(cnt);
				} else
				if (_stricmp(method, USERLOGGEDOUT_METHOD) == 0) {
					onLogoutResponse(cnt);
				} else
				LOG("Unknown method %s\n", method);
			}

		} else LOG("Incorrect container\n");

		SetEvent(m_wait);
	} else
	{
		LOG( "Unhandled message to service \"%s\"\n", msg.DstService() );
	}
}

void VS_FakeClientTest::onLoginResponse(VS_Container &cnt)
{
	long res = ACCESS_DENIED;
	cnt.GetValue(RESULT_PARAM, res);
	if (res == USER_LOGGEDIN_OK) m_logged_in_name = m_tmp;
	SetEvent(m_wait);
}

void VS_FakeClientTest::onLogoutResponse(VS_Container &cnt)
{
	long res = USER_ALREADY_LOGGEDOUT;
	cnt.GetValue(RESULT_PARAM, res);
	if (res == USER_LOGGEDOUT_OK) m_logged_in_name.clear();
	SetEvent(m_wait);
}


void VS_FakeClientTest::StartTesting() // static public
{
	for (int i = 0; i < sizeof(test_actions) / sizeof(char *); i++)
	{
		_beginthread(&VS_FakeClientTest::StartTestingPrivate, 0, test_actions[i]);
	}
}

void VS_FakeClientTest::StartTesting(const char *action_list ) // member, private
{
	m_action_list = action_list;

	while (!m_action_list.empty())
	{
		std::string current_action;

		int i = m_action_list.find(';');
		if (i == std::string::npos)
		{
			current_action = m_action_list;
			m_action_list.clear();
		} else
		{
			current_action.assign(m_action_list, 0, i);
			m_action_list.erase(0, i + 1);
		}
		if (current_action.empty()) continue;
		if (!PerformActionSynchronously( current_action ))
		{
			LOG("Test failed executing \"%s\"\n", current_action.c_str());
			break;
		} else
		{
			LOG("complete \"%s\".\n", current_action.c_str());
		}
	}
}

#define STARTS_WITH(s, x) (strncmp(s, x, sizeof(x) - 1) == NULL)
#define TEST_FAIL(x) do { LOG(x); return false; } while (0)

bool VS_FakeClientTest::PerformActionSynchronously( std::string &action)
{
	const char *str = action.c_str();
	ResetEvent(m_wait);

	if (STARTS_WITH(str, "PAUSE"))
	{
		const char *msec = strchr(str, ' ') + 1;
		if (msec == (const char *)1) TEST_FAIL("PAUSE action incorrect\n");
		unsigned x = atoi(msec);
		Sleep(x);
		return true;
	}
	if (STARTS_WITH(str, "CONNECT"))
	{
		std::string res = VS_FakeEndpointManager::Instance().RegisterEndpoint(
			boost::static_pointer_cast<VS_FakeEndpointBase>( shared_from_this() ) );
		LOG("registered with CID = \"%s\"\n", res.c_str());
		return !res.empty();
	}
	if (STARTS_WITH(str, "DISCONNECT"))
	{
		VS_FakeEndpointManager::Instance().DeleteEndpoint( this->getCID() );
		return true;
	}
	if (STARTS_WITH(str, "LOGIN"))
	{
		const char *login = strchr(str, ' ') + 1;
		if (login == (const char *)1) TEST_FAIL("LOGIN action incorrect\n");
		const char *passwd = strchr(login, ' ') + 1;
		if (passwd == (const char *)1) TEST_FAIL("LOGIN action incorrect\n");
		m_tmp = login;
		m_tmp.erase(m_tmp.find(' '));
		LoginUser(m_tmp.c_str(), passwd);

		return WaitForResult( action );
	}
	if (STARTS_WITH(str, "CHAT"))
	{
		const char *to = strchr(str, ' ') + 1;
		if (to == (const char *)1) TEST_FAIL("CHAT action incorrect\n");
		const char *msg = strchr(to, ' ') + 1;
		if (msg == (const char *)1) TEST_FAIL("CHAT action incorrect\n");
		std::string tmp(to);
		tmp.erase(tmp.find(' '));
		SendChatMessage(tmp.c_str(), msg);
		return true;
	}
	if (STARTS_WITH(str, "LOGOUT"))
	{
		Logout();
		return WaitForResult( action );
	}
	printf("Unknown test %s\n", str);
	return false;
}

void VS_FakeClientTest::StartTestingPrivate(void *data)
{
	const char *test = (const char *)data;
	if (test) (boost::shared_ptr<VS_FakeClientTest>(new VS_FakeClientTest()))->StartTesting(test);
}

bool VS_FakeClientTest::WaitForResult(const std::string & action )
{
	bool res = WAIT_OBJECT_0 == WaitForSingleObject(m_wait, 1000000);
	if (!res) return false;
	if (STARTS_WITH(action.c_str(), "LOGIN"))
	{
		if (m_logged_in_name.empty()) TEST_FAIL("LOGIN failed\n");
		return true;
	}
	if (STARTS_WITH(action.c_str(), "LOGOUT"))
	{
		if (!m_logged_in_name.empty()) TEST_FAIL("LOGOUT failed\n");
		return true;
	}
	printf("unknown test %s\n", action.c_str());
	return false;
}


