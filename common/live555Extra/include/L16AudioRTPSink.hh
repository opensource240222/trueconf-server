#ifndef _L16_AUDIO_RTP_SINK_HH
#define _L16_AUDIO_RTP_SINK_HH

#include <AudioRTPSink.hh>

class L16AudioRTPSink : public AudioRTPSink
{
public:
	static L16AudioRTPSink* createNew(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadType, unsigned rtpTimestampFrequency, unsigned numChannels, Boolean packFrames);

private:
	L16AudioRTPSink(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadType, unsigned rtpTimestampFrequency, unsigned numChannels, Boolean packFrames);
	~L16AudioRTPSink();

	virtual Boolean frameCanAppearAfterPacketStart(unsigned char const* frameStart, unsigned numBytesInFrame) const;

	Boolean fPackFrames;
};

#endif
