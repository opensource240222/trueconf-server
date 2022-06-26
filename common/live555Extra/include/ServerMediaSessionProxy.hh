#ifndef _SERVER_MEDIA_SESSION_PROXY_HH
#define _SERVER_MEDIA_SESSION_PROXY_HH

#include <ServerMediaSession.hh>

class ServerMediaSessionProxy : public ServerMediaSession
{
public:
	static ServerMediaSessionProxy* createNew(UsageEnvironment& env, ServerMediaSession* sms);
	static ServerMediaSessionProxy* createNew(UsageEnvironment& env, ServerMediaSession* sms, char const* streamName);
	static ServerMediaSessionProxy* createNew(UsageEnvironment& env, ServerMediaSession* sms, char const* streamName, char const* info);
	static ServerMediaSessionProxy* createNew(UsageEnvironment& env, ServerMediaSession* sms, char const* streamName, char const* info, char const* description);
	static ServerMediaSessionProxy* createNew(UsageEnvironment& env, ServerMediaSession* sms, char const* streamName, char const* info, char const* description, Boolean isSSM);
	static ServerMediaSessionProxy* createNew(UsageEnvironment& env, ServerMediaSession* sms, char const* streamName, char const* info, char const* description, Boolean isSSM, char const* miscSDPLines);

	Boolean addSubsession(ServerMediaSubsession* subsession);
	ServerMediaSession* getBaseSession() const;

private:
	ServerMediaSessionProxy(UsageEnvironment& env, ServerMediaSession* sms, char const* streamName, char const* info, char const* description, Boolean isSSM, char const* miscSDPLines);
	~ServerMediaSessionProxy();

	ServerMediaSession* fSMS;
};

#endif
