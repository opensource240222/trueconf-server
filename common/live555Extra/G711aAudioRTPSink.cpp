#include "G711aAudioRTPSink.hh"

G711aAudioRTPSink* G711aAudioRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs, Boolean packFrames)
{
	return new G711aAudioRTPSink(env, RTPgs, packFrames);
}

G711aAudioRTPSink::G711aAudioRTPSink(UsageEnvironment& env, Groupsock* RTPgs, Boolean packFrames)
	: AudioRTPSink(env, RTPgs, 8, 8000, "PCMA")
	, fPackFrames(packFrames)
{
}

G711aAudioRTPSink::~G711aAudioRTPSink()
{
}

Boolean G711aAudioRTPSink::frameCanAppearAfterPacketStart(unsigned char const* frameStart, unsigned numBytesInFrame) const
{
	return fPackFrames;
}
