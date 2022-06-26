#ifndef _G711U_AUDIO_RTP_SINK_HH
#define _G711U_AUDIO_RTP_SINK_HH

#include <AudioRTPSink.hh>

class G711uAudioRTPSink : public AudioRTPSink
{
public:
	static G711uAudioRTPSink* createNew(UsageEnvironment& env, Groupsock* RTPgs, Boolean packFrames);

private:
	G711uAudioRTPSink(UsageEnvironment& env, Groupsock* RPTgs, Boolean packFrames);
	~G711uAudioRTPSink();

	virtual Boolean frameCanAppearAfterPacketStart(unsigned char const* frameStart, unsigned numBytesInFrame) const;

	Boolean fPackFrames;
};

#endif
