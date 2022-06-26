#include "ServerMediaSessionProxy.hh"

class ServerMediaSubsessionProxy : public ServerMediaSubsession
{
public:
	static ServerMediaSubsessionProxy* createNew(UsageEnvironment& env, ServerMediaSubsession* smss)
	{
		return new ServerMediaSubsessionProxy(env, smss);
	}

private:
	ServerMediaSubsessionProxy(UsageEnvironment& env, ServerMediaSubsession* smss)
		: ServerMediaSubsession(env)
		, fSMSS(smss)
	{
		//fServerAddressForSDP = smss->fServerAddressForSDP;
		//fPortNumForSDP = smss->fPortNumForSDP;
	}

	virtual char const* sdpLines()
	{
		fSMSS->setServerAddressAndPortForSDP(fServerAddressForSDP, fPortNumForSDP);
		return fSMSS->sdpLines();
	}
	virtual void getStreamParameters(unsigned clientSessionId, netAddressBits clientAddress, Port const& clientRTPPort, Port const& clientRTCPPort, int tcpSocketNum, unsigned char rtpChannelId, unsigned char rtcpChannelId, netAddressBits& destinationAddress, u_int8_t& destinationTTL, Boolean& isMulticast, Port& serverRTPPort, Port& serverRTCPPort, void*& streamToken)
	{
		return fSMSS->getStreamParameters(clientSessionId, clientAddress, clientRTPPort, clientRTCPPort, tcpSocketNum, rtpChannelId, rtcpChannelId, destinationAddress, destinationTTL, isMulticast, serverRTPPort, serverRTCPPort, streamToken);
	}
	virtual void startStream(unsigned clientSessionId, void* streamToken, TaskFunc* rtcpRRHandler, void* rtcpRRHandlerClientData, unsigned short& rtpSeqNum, unsigned& rtpTimestamp, ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler, void* serverRequestAlternativeByteHandlerClientData)
	{
		return fSMSS->startStream(clientSessionId, streamToken, rtcpRRHandler, rtcpRRHandlerClientData, rtpSeqNum, rtpTimestamp, serverRequestAlternativeByteHandler, serverRequestAlternativeByteHandlerClientData);
	}
	virtual void pauseStream(unsigned clientSessionId, void* streamToken)
	{
		return fSMSS->pauseStream(clientSessionId, streamToken);
	}
	virtual void seekStream(unsigned clientSessionId, void* streamToken, double& seekNPT, double streamDuration, u_int64_t& numBytes)
	{
		return fSMSS->seekStream(clientSessionId, streamToken, seekNPT, streamDuration, numBytes);
	}
	virtual void nullSeekStream(unsigned clientSessionId, void* streamToken, double streamEndTime, u_int64_t& numBytes)
	{
		return fSMSS->nullSeekStream(clientSessionId, streamToken, streamEndTime, numBytes);
	}
	virtual void setStreamScale(unsigned clientSessionId, void* streamToken, float scale)
	{
		return fSMSS->setStreamScale(clientSessionId, streamToken, scale);
	}
	virtual float getCurrentNPT(void* streamToken)
	{
		return fSMSS->getCurrentNPT(streamToken);
	}
	virtual FramedSource* getStreamSource(void* streamToken)
	{
		return fSMSS->getStreamSource(streamToken);
	}
	virtual void deleteStream(unsigned clientSessionId, void*& streamToken)
	{
		return fSMSS->deleteStream(clientSessionId, streamToken);
	}
	virtual void testScaleFactor(float& scale)
	{
		return fSMSS->testScaleFactor(scale);
	}
	virtual float duration() const
	{
		return fSMSS->duration();
	}
	virtual void getAbsoluteTimeRange(char*& absStartTime, char*& absEndTime) const
	{
		return fSMSS->getAbsoluteTimeRange(absStartTime, absEndTime);
	}

private:
	ServerMediaSubsession* fSMSS;
};

class ServerMediaSubsessionStub : public ServerMediaSubsession
{
public:
	static ServerMediaSubsessionStub* createNew(UsageEnvironment& env)
	{
		return new ServerMediaSubsessionStub(env);
	}

private:
	explicit ServerMediaSubsessionStub(UsageEnvironment& env)
		: ServerMediaSubsession(env)
	{
	}

	virtual char const* sdpLines()
	{
		return NULL;
	}
	virtual void getStreamParameters(unsigned clientSessionId, netAddressBits clientAddress, Port const& clientRTPPort, Port const& clientRTCPPort, int tcpSocketNum, unsigned char rtpChannelId, unsigned char rtcpChannelId, netAddressBits& destinationAddress, u_int8_t& destinationTTL, Boolean& isMulticast, Port& serverRTPPort, Port& serverRTCPPort, void*& streamToken)
	{
	}
	virtual void startStream(unsigned clientSessionId, void* streamToken, TaskFunc* rtcpRRHandler, void* rtcpRRHandlerClientData, unsigned short& rtpSeqNum, unsigned& rtpTimestamp, ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler, void* serverRequestAlternativeByteHandlerClientData)
	{
	}
};

ServerMediaSessionProxy* ServerMediaSessionProxy::createNew(UsageEnvironment& env, ServerMediaSession* sms)
{
	return new ServerMediaSessionProxy(env, sms, sms->streamName(), sms->info(), sms->description(), sms->isSSM(), sms->miscSDPLines());
}
ServerMediaSessionProxy* ServerMediaSessionProxy::createNew(UsageEnvironment& env, ServerMediaSession* sms, char const* streamName)
{
	return new ServerMediaSessionProxy(env, sms, streamName, sms->info(), sms->description(), sms->isSSM(), sms->miscSDPLines());
}
ServerMediaSessionProxy* ServerMediaSessionProxy::createNew(UsageEnvironment& env, ServerMediaSession* sms, char const* streamName, char const* info)
{
	return new ServerMediaSessionProxy(env, sms, streamName, info, sms->description(), sms->isSSM(), sms->miscSDPLines());
}
ServerMediaSessionProxy* ServerMediaSessionProxy::createNew(UsageEnvironment& env, ServerMediaSession* sms, char const* streamName, char const* info, char const* description)
{
	return new ServerMediaSessionProxy(env, sms, streamName, info, description, sms->isSSM(), sms->miscSDPLines());
}
ServerMediaSessionProxy* ServerMediaSessionProxy::createNew(UsageEnvironment& env, ServerMediaSession* sms, char const* streamName, char const* info, char const* description, Boolean isSSM)
{
	return new ServerMediaSessionProxy(env, sms, streamName, info, description, isSSM, sms->miscSDPLines());
}
ServerMediaSessionProxy* ServerMediaSessionProxy::createNew(UsageEnvironment& env, ServerMediaSession* sms, char const* streamName, char const* info, char const* description, Boolean isSSM, char const* miscSDPLines)
{
	return new ServerMediaSessionProxy(env, sms, streamName, info, description, isSSM, miscSDPLines);
}

ServerMediaSessionProxy::ServerMediaSessionProxy(UsageEnvironment& env, ServerMediaSession* sms, char const* streamName, char const* info, char const* description, Boolean isSSM, char const* miscSDPLines)
	: ServerMediaSession(env, streamName, info, description, isSSM, miscSDPLines)
	, fSMS(sms)
{
	fSMS->incrementReferenceCount();
}

ServerMediaSessionProxy::~ServerMediaSessionProxy()
{
	deleteAllSubsessions();
	fSMS->decrementReferenceCount();
	if (fSMS->referenceCount() == 0 && fSMS->deleteWhenUnreferenced())
		Medium::close(fSMS);
}

Boolean isParentSession(ServerMediaSession* sms, ServerMediaSubsession* smss)
{
	// return smss->fParentSession == sms;

	ServerMediaSubsessionIterator smssIt(*sms);
	ServerMediaSubsession* x;
	while (NULL != (x = smssIt.next()))
		if (x == smss)
			return True;
	return False;
}

Boolean ServerMediaSessionProxy::addSubsession(ServerMediaSubsession* subsession)
{
	if (!isParentSession(fSMS, subsession))
		return False;

	if (subsession->trackNumber() < ServerMediaSession::numSubsessions()+1)
		return False;
	while (subsession->trackNumber() > ServerMediaSession::numSubsessions()+1)
	{
		ServerMediaSubsession* smss = ServerMediaSubsessionStub::createNew(envir());
		if (!ServerMediaSession::addSubsession(smss))
		{
			Medium::close(smss);
			return False;
		}
	}

	ServerMediaSubsession* smss = ServerMediaSubsessionProxy::createNew(envir(), subsession);
	if (!ServerMediaSession::addSubsession(smss))
	{
		Medium::close(smss);
		return False;
	}
	return True;
}

ServerMediaSession* ServerMediaSessionProxy::getBaseSession() const
{
	return fSMS;
}
