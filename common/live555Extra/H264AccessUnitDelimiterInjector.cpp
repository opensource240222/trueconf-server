#include "H264AccessUnitDelimiterInjector.hh"

static const unsigned char H264AccessUnitDelimiterNALU[] = {
	0x09, // forbidden_zero_bit=0, nal_ref_idc=0, nal_unit_type=9
	0xF0, // primary_pic_type=7, rbsp_trailing_bits=0x10
};

H264AccessUnitDelimiterInjector* H264AccessUnitDelimiterInjector::createNew(UsageEnvironment& env, FramedSource* inputSource)
{
	return new H264AccessUnitDelimiterInjector(env, inputSource);
}

H264AccessUnitDelimiterInjector::H264AccessUnitDelimiterInjector(UsageEnvironment& env, FramedSource* inputSource)
	: FramedFilter(env, inputSource)
	, fLastPresentationTime({0, 0})
	, fHoldBuffer(NULL)
	, fHoldBufferSize(0)
	, fSavedFrameSize(0)
{
}

H264AccessUnitDelimiterInjector::~H264AccessUnitDelimiterInjector()
{
	delete[] fHoldBuffer;
}

void H264AccessUnitDelimiterInjector::doGetNextFrame()
{
	if (fSavedFrameSize > 0)
	{
		fFrameSize = fSavedFrameSize;
		fNumTruncatedBytes = fSavedNumTruncatedBytes;
		if (fFrameSize > fMaxSize)
		{
			fNumTruncatedBytes += fFrameSize - fMaxSize;
			fFrameSize = fMaxSize;
		}
		memcpy(fTo, fHoldBuffer, fFrameSize);
		fPresentationTime = fSavedPresentationTime;
		fDurationInMicroseconds = fSavedDurationInMicroseconds;
		fSavedFrameSize = 0;
		afterGetting(this);
	}
	else
		fInputSource->getNextFrame(fTo, fMaxSize, H264AccessUnitDelimiterInjector::afterGettingFrame, this, FramedSource::handleClosure, this);
}

void H264AccessUnitDelimiterInjector::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	static_cast<H264AccessUnitDelimiterInjector*>(clientData)->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

void H264AccessUnitDelimiterInjector::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	if (presentationTime.tv_sec  > fLastPresentationTime.tv_sec
	 || (presentationTime.tv_sec == fLastPresentationTime.tv_sec && presentationTime.tv_usec > fLastPresentationTime.tv_usec))
	{
		if (fHoldBufferSize < frameSize)
		{
			delete[] fHoldBuffer;
			fHoldBufferSize = frameSize;
			fHoldBuffer = new unsigned char[fHoldBufferSize];
		}
		memcpy(fHoldBuffer, fTo, frameSize);
		fSavedFrameSize = frameSize;
		fSavedNumTruncatedBytes = numTruncatedBytes;
		fSavedPresentationTime = presentationTime;
		fSavedDurationInMicroseconds = durationInMicroseconds;

		fFrameSize = sizeof(H264AccessUnitDelimiterNALU);
		fNumTruncatedBytes = 0;
		if (fFrameSize > fMaxSize)
		{
			fNumTruncatedBytes += fFrameSize - fMaxSize;
			fFrameSize = fMaxSize;
		}
		memcpy(fTo, H264AccessUnitDelimiterNALU, fFrameSize);
		fPresentationTime = presentationTime;
		fDurationInMicroseconds = 0;
	}
	else
	{
		fFrameSize = frameSize;
		fNumTruncatedBytes = numTruncatedBytes;
		fPresentationTime = presentationTime;
		fDurationInMicroseconds = durationInMicroseconds;
	}
	fLastPresentationTime = presentationTime;
	afterGetting(this);
}
