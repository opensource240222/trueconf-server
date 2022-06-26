#include "GSMAudioStreamFramer.hh"
#include "timeval_helper.hh"

#include <GroupsockHelper.hh>

inline void shr_buffer(unsigned char* buf, unsigned size, unsigned shift)
{
	const unsigned shift_bytes = shift / 8;
	const unsigned shift_bits = shift % 8;
	for (unsigned char *p = buf+size-1; p >= buf; --p)
	{
		*(p+shift_bytes) = *p >> shift_bits;
		if (p > buf)
			*(p+shift_bytes) |= *(p-1) << (8-shift_bits);
	}
}

const unsigned cGSMFrameSize = 33;

GSMAudioStreamFramer* GSMAudioStreamFramer::createNew(UsageEnvironment& env, FramedSource* inputSource, int format)
{
	return new GSMAudioStreamFramer(env, inputSource, format);
}

GSMAudioStreamFramer::GSMAudioStreamFramer(UsageEnvironment& env, FramedSource* inputSource, int format)
	: FramedFilter(env, inputSource)
	, fFormat(format)
	, fEvenFrame(false)
	, fSavedFrameBeginning(0)
{
	gettimeofday(&fNextFramePresentationTime, NULL);
}

GSMAudioStreamFramer::~GSMAudioStreamFramer()
{
}

void GSMAudioStreamFramer::doGetNextFrame()
{
	fFrameSize = 0;
	if (fMaxSize < cGSMFrameSize)
	{
		fNumTruncatedBytes = cGSMFrameSize;
		afterGetting(this);
	}

	if (fFormat == 1 && fEvenFrame)
	{
		fTo += 1;
		fFrameSize += 1;
	}

	continueReading();
}

void GSMAudioStreamFramer::continueReading()
{
	fInputSource->getNextFrame(fTo, cGSMFrameSize-fFrameSize, GSMAudioStreamFramer::afterGettingFrame, this, FramedSource::handleClosure, this);
}

void GSMAudioStreamFramer::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval, unsigned)
{
	static_cast<GSMAudioStreamFramer*>(clientData)->afterGettingFrame(frameSize, numTruncatedBytes);
}

void GSMAudioStreamFramer::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes)
{
	fTo += frameSize;
	fFrameSize += frameSize;
	if (fFrameSize == cGSMFrameSize)
	{
		unsigned char* const frameStart = fTo-cGSMFrameSize;
		if (fFormat == 1)
		{
			if (fEvenFrame)
			{
				*frameStart = 0xD0 | fSavedFrameBeginning;
				fEvenFrame = false;
			}
			else
			{
				fSavedFrameBeginning = frameStart[cGSMFrameSize-1] >> 4;
				shr_buffer(frameStart, cGSMFrameSize, 4);
				*frameStart |= 0xD0;
				fEvenFrame = true;
			}
		}
		else if (fFormat == 2)
		{
			shr_buffer(frameStart, cGSMFrameSize, 4);
			*frameStart |= 0xD0;
		}

		fPresentationTime = fNextFramePresentationTime;
		fNextFramePresentationTime += std::chrono::milliseconds(20);
		fDurationInMicroseconds = 20000;
		afterGetting(this);
	}
	else
		continueReading();
}
