#include "L16AudioRTPSink.hh"

L16AudioRTPSink* L16AudioRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadType, unsigned rtpTimestampFrequency, unsigned numChannels, Boolean packFrames)
{
	return new L16AudioRTPSink(env, RTPgs, rtpPayloadType, rtpTimestampFrequency, numChannels, packFrames);
}

unsigned L16RTPPayloadType(unsigned samplingRate, unsigned numChannels, unsigned rtpPayloadTypeIfDynamic)
{
	if (samplingRate == 44100 && numChannels == 1)
		return 11;
	else if (samplingRate == 44100 && numChannels == 2)
		return 10;
	else
	 return rtpPayloadTypeIfDynamic;
}


L16AudioRTPSink::L16AudioRTPSink(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadType, unsigned rtpTimestampFrequency, unsigned numChannels, Boolean packFrames)
	: AudioRTPSink(env, RTPgs, L16RTPPayloadType(rtpTimestampFrequency, numChannels, rtpPayloadType), rtpTimestampFrequency, "L16", numChannels)
	, fPackFrames(packFrames)
{
}

L16AudioRTPSink::~L16AudioRTPSink()
{
}

Boolean L16AudioRTPSink::frameCanAppearAfterPacketStart(unsigned char const* frameStart, unsigned numBytesInFrame) const
{
	return fPackFrames;
}
