#include "G711uAudioRTPSink.hh"

G711uAudioRTPSink* G711uAudioRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs, Boolean packFrames)
{
	return new G711uAudioRTPSink(env, RTPgs, packFrames);
}

G711uAudioRTPSink::G711uAudioRTPSink(UsageEnvironment& env, Groupsock* RTPgs, Boolean packFrames)
	: AudioRTPSink(env, RTPgs, 0, 8000, "PCMU")
	, fPackFrames(packFrames)
{
}

G711uAudioRTPSink::~G711uAudioRTPSink()
{
}

Boolean G711uAudioRTPSink::frameCanAppearAfterPacketStart(unsigned char const* frameStart, unsigned numBytesInFrame) const
{
	return fPackFrames;
}
