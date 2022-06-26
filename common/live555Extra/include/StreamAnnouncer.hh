#ifndef _STREAM_ANNOUNCER_HH
#define _STREAM_ANNOUNCER_HH

#include <Media.hh>
#include <DigestAuthentication.hh>
#include <string>

class MediaSession;
class MediaSubsession;
class RTSPClient;
class RTSPClientHelper;
class ServerMediaSession;
class ServerMediaSubsession;

class StreamAnnouncer : public Medium {
public:
	static const unsigned announceFakeSessionId = 0x414E4E43; // "ANNC"
	static const unsigned checkSocketInterval = 1000000;

	static StreamAnnouncer* createNew(UsageEnvironment& env, ServerMediaSession* sms, const char* url, Boolean streamUsingTCP = False, Authenticator* authenticator = NULL, unsigned rtspTimeout = 30, unsigned livenessTimeout = 60, int clientVerbosityLevel = 0);

	void setCompletionHandler(TaskFunc handler, void* clientData)
	{
		fCompletionHandler = handler;
		fCompletionHandlerClientData = clientData;
	}
	void setLivenessTimeoutHandler(TaskFunc handler, void* clientData)
	{
		fLivenessTimeoutHandler = handler;
		fLivenessTimeoutHandlerClientData = clientData;
	}

	Boolean start();
	void stop();

	enum State { state_initial, state_in_progress, state_local_error, state_remote_error, state_success };
	State state() const { return fState; }

private:
	StreamAnnouncer(UsageEnvironment& env, ServerMediaSession* sms, const char* url, Boolean streamUsingTCP, Authenticator* authenticator, unsigned rtspTimeout, unsigned livenessTimeout, int clientVerbosityLevel);
	~StreamAnnouncer();
	void handleCommandError(const char* command, int resultCode, const char* resultString);

	static void handleANNOUNCEResponse(RTSPClient* rtspClient, int resultCode, char* resultString);
	void handleANNOUNCEResponse(int resultCode, char* resultString);
	static void handleSETUPResponse(RTSPClient* rtspClient, int resultCode, char* resultString);
	void handleSETUPResponse(int resultCode, char* resultString);
	static void handleRECORDResponse(RTSPClient* rtspClient, int resultCode, char* resultString);
	void handleRECORDResponse(int resultCode, char* resultString);

	static void noteLiveness(void* clientData);
	void noteLiveness();
	static void onLivenessTimeout(void* clientData);
	void onLivenessTimeout();
	static void onRTSPTimeout(void* clientData);
	void onRTSPTimeout();
	static void checkSocket(void* clientData);
	void checkSocket();

	Boolean fStreamUsingTCP;
	RTSPClientHelper* fClient;
	int fClientVerbosityLevel;
	std::string fURL;
	char* fSDP;
	ServerMediaSession* fServerSession;
	MediaSession* fClientSession;
	Authenticator fAuthenticator;

	struct SubsessionData
	{
		SubsessionData* next;
		ServerMediaSubsession* smss;
		MediaSubsession* mss;
		void* streamToken;
	};
	SubsessionData* fSubsessions;
	SubsessionData* fCurrentSubsession;

	unsigned fRTSPTimeout;
	TaskToken fRTSPTimeoutTask;
	Boolean fRTSPTimeoutReached; // Because RTSPClient::changeResponseHandler doesn't work when requests are getting resend
	unsigned fLivenessTimeout;
	TaskToken fLivenessTimeoutTask;
	Boolean fLivenessTimeoutReached;
	TaskToken fCheckSocketTask;

	TaskFunc* fCompletionHandler;
	void* fCompletionHandlerClientData;
	TaskFunc* fLivenessTimeoutHandler;
	void* fLivenessTimeoutHandlerClientData;

	State fState;
};

#endif
