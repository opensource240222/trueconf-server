#ifndef _LATE_FRAME_DROPPER_HH
#define _LATE_FRAME_DROPPER_HH

#include <FrameDropper.hh>

class LateFrameDropper : public FrameDropper
{
public:
	static LateFrameDropper* createNew(UsageEnvironment& env, FramedSource* inputSource, unsigned timeoutMaxMicroseconds, unsigned timeoutMinMicroseconds);

private:
	LateFrameDropper(UsageEnvironment& env, FramedSource* inputSource, unsigned timeoutMaxMicroseconds, unsigned timeoutMinMicroseconds);

	virtual Boolean shouldDrop();

private:
	unsigned fTimeoutMaxMicroseconds;
	unsigned fTimeoutMinMicroseconds;
};

#endif
