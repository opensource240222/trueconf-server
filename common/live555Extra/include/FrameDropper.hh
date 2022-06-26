#ifndef _FRAME_DROPPER_HH
#define _FRAME_DROPPER_HH

#include <FramedFilter.hh>

class FrameDropper : public FramedFilter
{
public:
	unsigned numDropped()
	{
		return fNumDroppedFrames;
	}

protected:
	FrameDropper(UsageEnvironment& env, FramedSource* inputSource);

	virtual void doGetNextFrame();
	static void afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
	void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);

	virtual Boolean shouldDrop() = 0;

private:
	unsigned int fNumDroppedFrames;
	Boolean fInGetNextFrame;
	Boolean fNeedMoreFrames;
};

#endif
