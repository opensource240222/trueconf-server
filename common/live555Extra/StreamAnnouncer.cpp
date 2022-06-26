#include "StreamAnnouncer.hh"
#include "RTSPClient.hh"
#include "ServerMediaSession.hh"
#include "MediaSession.hh"
#include "GroupsockHelper.hh"

// Things would have been much easier if RTSPClient had sane API for handlers
class RTSPClientHelper : public RTSPClient
{
public:
	static RTSPClientHelper* createNew(UsageEnvironment& env, char const* rtspURL, int verbosityLevel = 0, char const* applicationName = NULL, portNumBits tunnelOverHTTPPortNum = 0, int socketNumToServer = -1)
	{
		return new RTSPClientHelper(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, socketNumToServer);
	}

	StreamAnnouncer* owner;

private:
	RTSPClientHelper(UsageEnvironment& env, char const* rtspURL, int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum, int socketNumToServer)
		: RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, socketNumToServer)
	{
	}
};

StreamAnnouncer* StreamAnnouncer::createNew(UsageEnvironment& env, ServerMediaSession* sms, const char* url, Boolean streamUsingTCP, Authenticator* authenticator, unsigned rtspTimeout, unsigned livenessTimeout, int clientVerbosityLevel)
{
	return new StreamAnnouncer(env, sms, url, streamUsingTCP, authenticator, rtspTimeout, livenessTimeout, clientVerbosityLevel);
}

StreamAnnouncer::StreamAnnouncer(UsageEnvironment& env, ServerMediaSession* sms, const char* url, Boolean streamUsingTCP, Authenticator* authenticator, unsigned rtspTimeout, unsigned livenessTimeout, int clientVerbosityLevel)
	: Medium(env)
	, fStreamUsingTCP(streamUsingTCP)
	, fClient(NULL)
	, fClientVerbosityLevel(clientVerbosityLevel)
	, fURL(url? url : "")
	, fSDP(NULL)
	, fServerSession(sms)
	, fClientSession(NULL)
	, fSubsessions(NULL)
	, fCurrentSubsession(NULL)
	, fRTSPTimeout(rtspTimeout)
	, fRTSPTimeoutTask(0)
	, fRTSPTimeoutReached(False)
	, fLivenessTimeout(livenessTimeout)
	, fLivenessTimeoutTask(0)
	, fLivenessTimeoutReached(False)
	, fCheckSocketTask(0)
	, fCompletionHandler(NULL)
	, fCompletionHandlerClientData(NULL)
	, fLivenessTimeoutHandler(NULL)
	, fLivenessTimeoutHandlerClientData(NULL)
	, fState(state_initial)
{
	fServerSession->incrementReferenceCount();

	char* username;
	char* password;
	NetAddress address;
	portNumBits portNum; // unused
	if (!RTSPClient::parseRTSPURL(envir(), url, username, password, address, portNum))
	{
		fState = state_local_error;
		envir().prependToResultMsg("Failed to parse url: ");
		return;
	}
	if (authenticator != NULL)
		fAuthenticator = *authenticator;
	else if (username != NULL && password != NULL)
		fAuthenticator.setUsernameAndPassword(username, password);
	delete[] username;
	delete[] password;

	ServerMediaSubsessionIterator smssIt(*fServerSession);
	ServerMediaSubsession* smss;
	while (NULL != (smss = smssIt.next()))
	{
		smss->setServerAddressAndPortForSDP(*(netAddressBits*)address.data(), 0);
	}
	fSDP = fServerSession->generateSDPDescription();
	fClientSession = MediaSession::createNew(envir(), fSDP);

	if (fClientSession == NULL)
	{
		fState = state_local_error;
		envir().prependToResultMsg("Failed to create client session: ");
		return;
	}

	smssIt.reset();
	MediaSubsessionIterator mssIt(*fClientSession);
	SubsessionData* lastSD = NULL;
	while (NULL != (smss = smssIt.next()))
	{
		MediaSubsession* mss = mssIt.next();
		if (mss == NULL)
		{
			fState = state_local_error;
			envir().setResultMsg("Internal error: Server subsessions > client subsessions");
			return;
		}

		SubsessionData* sd(new SubsessionData);
		sd->next = NULL;
		sd->smss = smss;
		sd->mss = mss;
		sd->streamToken = NULL;

		if (fSubsessions == NULL)
			fSubsessions = sd;
		if (lastSD != NULL)
			lastSD->next = sd;
		lastSD = sd;
	}
	if (mssIt.next() != NULL)
	{
		fState = state_local_error;
		envir().setResultMsg("Internal error: Server subsessions < client subsessions");
		return;
	}
	if (fSubsessions == NULL)
	{
		fState = state_local_error;
		envir().setResultMsg("Internal error: Empty server session");
		return;
	}
}

StreamAnnouncer::~StreamAnnouncer()
{
	stop();

	SubsessionData* sd = fSubsessions;
	while (sd != NULL)
	{
		SubsessionData* nextSD = sd->next;
		delete sd;
		sd = nextSD;
	}

	Medium::close(fClientSession);

	fServerSession->decrementReferenceCount();
	if (fServerSession->referenceCount() == 0 && fServerSession->deleteWhenUnreferenced())
		Medium::close(fServerSession);

	delete[] fSDP;
}

void StreamAnnouncer::handleCommandError(const char* command, int resultCode, const char* resultString)
{
	if (resultCode > 0) {
		fState = state_remote_error;
		envir().prependToResultMsg(" failed: ");
		envir().prependToResultMsg(command);
	} else if (resultCode < 0) {
		fState = state_local_error;
		envir().setResultErrMsg(" failed: ", -resultCode);
		envir().prependToResultMsg(command);
	}
	if (fCompletionHandler)
		fCompletionHandler(fCompletionHandlerClientData);
}

Boolean StreamAnnouncer::start()
{
	if (fState != state_initial)
		return False;
	fState = state_in_progress;

	fClient = RTSPClientHelper::createNew(envir(), fURL.c_str(), fClientVerbosityLevel);
	fClient->owner = this;
	if (fRTSPTimeout > 0)
		fRTSPTimeoutTask = envir().taskScheduler().scheduleDelayedTask(fRTSPTimeout*1000000, StreamAnnouncer::onRTSPTimeout, this);
	fClient->sendAnnounceCommand(fSDP, StreamAnnouncer::handleANNOUNCEResponse, &fAuthenticator);
	return True;
}

void StreamAnnouncer::handleANNOUNCEResponse(RTSPClient* rtspClient, int resultCode, char* resultString)
{
	static_cast<RTSPClientHelper*>(rtspClient)->owner->handleANNOUNCEResponse(resultCode, resultString);
}

void StreamAnnouncer::handleANNOUNCEResponse(int resultCode, char* resultString)
{
	if (fRTSPTimeoutReached)
		return;
	envir().taskScheduler().unscheduleDelayedTask(fRTSPTimeoutTask);

	if (resultCode != 0)
	{
		handleCommandError("ANNOUNCE", resultCode, resultString);
		return;
	}
	delete[] resultString;

	fCurrentSubsession = fSubsessions;
	if (fRTSPTimeout > 0)
		fRTSPTimeoutTask = envir().taskScheduler().scheduleDelayedTask(fRTSPTimeout*1000000, StreamAnnouncer::onRTSPTimeout, this);
	fClient->sendSetupCommand(*fCurrentSubsession->mss, StreamAnnouncer::handleSETUPResponse, True, fStreamUsingTCP, False, &fAuthenticator);
}

void StreamAnnouncer::handleSETUPResponse(RTSPClient* rtspClient, int resultCode, char* resultString)
{
	static_cast<RTSPClientHelper*>(rtspClient)->owner->handleSETUPResponse(resultCode, resultString);
}

void StreamAnnouncer::handleSETUPResponse(int resultCode, char* resultString)
{
	if (fRTSPTimeoutReached)
		return;
	envir().taskScheduler().unscheduleDelayedTask(fRTSPTimeoutTask);

	if (resultCode != 0)
	{
		handleCommandError("SETUP", resultCode, resultString);
		return;
	}
	delete[] resultString;

	fCurrentSubsession = fCurrentSubsession->next;
	if (fRTSPTimeout > 0)
		fRTSPTimeoutTask = envir().taskScheduler().scheduleDelayedTask(fRTSPTimeout*1000000, StreamAnnouncer::onRTSPTimeout, this);
	if (fCurrentSubsession != NULL)
		fClient->sendSetupCommand(*fCurrentSubsession->mss, StreamAnnouncer::handleSETUPResponse, True, fStreamUsingTCP, False, &fAuthenticator);
	else
		fClient->sendRecordCommand(*fClientSession, StreamAnnouncer::handleRECORDResponse, &fAuthenticator);
}

void StreamAnnouncer::handleRECORDResponse(RTSPClient* rtspClient, int resultCode, char* resultString)
{
	static_cast<RTSPClientHelper*>(rtspClient)->owner->handleRECORDResponse(resultCode, resultString);
}

void StreamAnnouncer::handleRECORDResponse(int resultCode, char* resultString)
{
	if (fRTSPTimeoutReached)
		return;
	envir().taskScheduler().unscheduleDelayedTask(fRTSPTimeoutTask);

	if (resultCode != 0)
	{
		handleCommandError("RECORD", resultCode, resultString);
		return;
	}
	delete[] resultString;

	if (fStreamUsingTCP)
		increaseSendBufferTo(envir(), fClient->socketNum(), 1024*1024);
	for (SubsessionData* sd = fSubsessions; sd != NULL; sd = sd->next)
	{
		netAddressBits destinationAddress = 0; // unused
		u_int8_t destinationTTL = 255; // unused
		Boolean isMulticast = False; // unused
		Port serverRTPPort(0); // unused
		Port serverRTCPPort(0); // unused
		sd->smss->getStreamParameters(
			announceFakeSessionId, // We are using the same fake session ID for all announcements
			sd->mss->connectionEndpointAddress(), // Client address
			sd->mss->serverPortNum, // RTP port
			sd->mss->serverPortNum+1, // RTCP port
			fStreamUsingTCP ? fClient->socketNum() : -1,
			sd->mss->rtpChannelId,
			sd->mss->rtcpChannelId,
			destinationAddress,
			destinationTTL,
			isMulticast,
			serverRTPPort,
			serverRTCPPort,
			sd->streamToken
		);
		unsigned short rtpSeqNum = 0; // unused
		unsigned rtpTimestamp = 0; // unused
		sd->smss->startStream(
			announceFakeSessionId,
			sd->streamToken,
			StreamAnnouncer::noteLiveness, this,
			rtpSeqNum,
			rtpTimestamp,
			NULL, NULL);
	}
	if (fLivenessTimeout > 0)
		fLivenessTimeoutTask = envir().taskScheduler().scheduleDelayedTask(fLivenessTimeout*1000000, StreamAnnouncer::onLivenessTimeout, this);

	fCheckSocketTask = envir().taskScheduler().scheduleDelayedTask(checkSocketInterval, StreamAnnouncer::checkSocket, this);

	fState = state_success;
	if (fCompletionHandler)
		fCompletionHandler(fCompletionHandlerClientData);
}

void StreamAnnouncer::stop()
{
	envir().taskScheduler().unscheduleDelayedTask(fCheckSocketTask);
	envir().taskScheduler().unscheduleDelayedTask(fRTSPTimeoutTask);
	envir().taskScheduler().unscheduleDelayedTask(fLivenessTimeoutTask);

	for (SubsessionData* sd = fSubsessions; sd != NULL; sd = sd->next)
	{
		if (sd->streamToken != NULL)
		{
			sd->smss->pauseStream(announceFakeSessionId, sd->streamToken);
			sd->smss->deleteStream(announceFakeSessionId, sd->streamToken);
		}
	}

	if (fClient && fCurrentSubsession != fSubsessions)
		fClient->sendTeardownCommand(*fClientSession, NULL);
	Medium::close(fClient);
	fClient = NULL;
	fCurrentSubsession = fSubsessions;

	fState = state_initial;
}

void StreamAnnouncer::noteLiveness(void* clientData)
{
	((StreamAnnouncer*)clientData)->noteLiveness();
}

void StreamAnnouncer::noteLiveness()
{
	if (fLivenessTimeoutReached)
		return;
	if (fLivenessTimeout > 0)
		envir().taskScheduler().rescheduleDelayedTask(fLivenessTimeoutTask, fLivenessTimeout*1000000, StreamAnnouncer::onLivenessTimeout, this);
	else if (fLivenessTimeoutTask)
		envir().taskScheduler().unscheduleDelayedTask(fLivenessTimeoutTask);
}

void StreamAnnouncer::onLivenessTimeout(void* clientData)
{
	((StreamAnnouncer*)clientData)->onLivenessTimeout();
}

void StreamAnnouncer::onLivenessTimeout()
{
	fLivenessTimeoutReached = True;
	fState = state_remote_error;
	envir().setResultMsg("Remote end not responding");
	if (fLivenessTimeoutHandler)
		fLivenessTimeoutHandler(fLivenessTimeoutHandlerClientData);
}

void StreamAnnouncer::onRTSPTimeout(void* clientData)
{
	((StreamAnnouncer*)clientData)->onRTSPTimeout();
}

void StreamAnnouncer::onRTSPTimeout()
{
	fRTSPTimeoutReached = True;
	fState = state_remote_error;
	envir().setResultMsg("RTSP request timed out");
	if (fCompletionHandler)
		fCompletionHandler(fCompletionHandlerClientData);
}

void StreamAnnouncer::checkSocket(void* clientData)
{
	((StreamAnnouncer*)clientData)->checkSocket();
}

void StreamAnnouncer::checkSocket()
{
	char buffer;
	if (::recv(fClient->socketNum(), &buffer, sizeof(buffer), MSG_PEEK) < 0)
	{
		int err = envir().getErrno();
#if defined(_WIN32)
		if (err != WSAEWOULDBLOCK && err != WSAEINTR && err != WSAEINPROGRESS)
#else
		if (err != EAGAIN && err != EWOULDBLOCK && err != EINTR)
#endif
		{
			fState = state_remote_error;
			envir().setResultErrMsg("RTSP connection broken: ", err);
			if (fLivenessTimeoutHandler)
				fLivenessTimeoutHandler(fLivenessTimeoutHandlerClientData);
			return;
		}
	}

	fCheckSocketTask = envir().taskScheduler().scheduleDelayedTask(checkSocketInterval, StreamAnnouncer::checkSocket, this);
}
