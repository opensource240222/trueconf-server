#ifndef _GSM_AUDIO_STREAM_FRAMER_HH
#define _GSM_AUDIO_STREAM_FRAMER_HH

#include <FramedFilter.hh>

class GSMAudioStreamFramer : public FramedFilter
{
public:
	static GSMAudioStreamFramer* createNew(UsageEnvironment& env, FramedSource* inputSource, int format = 0);
	// format == 0 => 33 bytes:          GSM magic (0xD0) + 260 bit GSM frame
	// format == 1 => 32 bytes + 4 bits: 260 bit GSM frame
	// format == 2 => 33 bytes:          260 bit GSM frame + 4 bit padding

private:
	GSMAudioStreamFramer(UsageEnvironment& env, FramedSource* inputSource, int format);
	~GSMAudioStreamFramer();

	virtual void doGetNextFrame();
	void continueReading();
	static void afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
	void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes);

private:
	int fFormat;
	bool fEvenFrame;
	unsigned char fSavedFrameBeginning;
	struct timeval fNextFramePresentationTime;
};

#endif
