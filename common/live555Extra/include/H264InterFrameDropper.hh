#ifndef _H264_INTER_FRAME_FRAME_DROPPER_HH
#define _H264_INTER_FRAME_FRAME_DROPPER_HH

#include <FrameDropper.hh>

class H264InterFrameDropper : public FrameDropper
{
public:
	static H264InterFrameDropper* createNew(UsageEnvironment& env, FramedSource* inputSource, Boolean waitForIntraFrame = True);

	Boolean isWaitingForIntraFrame() const
	{
		return fWaitForIntraFrame;
	}

	void waitForIntraFrame()
	{
		fWaitForIntraFrame = True;
	}

private:
	H264InterFrameDropper(UsageEnvironment& env, FramedSource* inputSource, Boolean waitForIntraFrame);

	virtual Boolean shouldDrop();

private:
	Boolean fWaitForIntraFrame;
	struct timeval fDropStartPT;
};

#endif
