#ifndef _G711A_AUDIO_RTP_SINK_HH
#define _G711A_AUDIO_RTP_SINK_HH

#include <AudioRTPSink.hh>

class G711aAudioRTPSink : public AudioRTPSink
{
public:
	static G711aAudioRTPSink* createNew(UsageEnvironment& env, Groupsock* RTPgs, Boolean packFrames);

private:
	G711aAudioRTPSink(UsageEnvironment& env, Groupsock* RPTgs, Boolean packFrames);
	~G711aAudioRTPSink();

	virtual Boolean frameCanAppearAfterPacketStart(unsigned char const* frameStart, unsigned numBytesInFrame) const;

	Boolean fPackFrames;
};

#endif
