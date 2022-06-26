#include "FrameDropper.hh"

FrameDropper::FrameDropper(UsageEnvironment& env, FramedSource* inputSource)
	: FramedFilter(env, inputSource)
	, fNumDroppedFrames(0)
	, fInGetNextFrame(False)
	, fNeedMoreFrames(False)
{
}

void FrameDropper::doGetNextFrame()
{
	// Avoiding recursive calls to this function
	if (fInGetNextFrame)
	{
		fNeedMoreFrames = True;
		return;
	}

	fInGetNextFrame = True;
	do
	{
		fNeedMoreFrames = False;
		fInputSource->getNextFrame(fTo, fMaxSize, FrameDropper::afterGettingFrame, this, FramedSource::handleClosure, this);
	}
	while (fNeedMoreFrames);
	fInGetNextFrame = False;
}

void FrameDropper::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	static_cast<FrameDropper*>(clientData)->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

void FrameDropper::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	fFrameSize = frameSize;
	fNumTruncatedBytes = numTruncatedBytes;
	fPresentationTime = presentationTime;
	fDurationInMicroseconds = durationInMicroseconds;
	if (shouldDrop())
	{
		++fNumDroppedFrames;
		FrameDropper::doGetNextFrame();
	}
	else
	{
		fNumDroppedFrames = 0;
		afterGetting(this);
	}
}
