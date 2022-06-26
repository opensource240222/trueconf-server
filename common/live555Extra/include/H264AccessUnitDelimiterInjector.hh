#ifndef _H264_ACCESS_UNIT_DELIMITER_INJECTOR_HH
#define _H264_ACCESS_UNIT_DELIMITER_INJECTOR_HH

#include <FramedFilter.hh>

class H264AccessUnitDelimiterInjector : public FramedFilter
{
public:
	static H264AccessUnitDelimiterInjector* createNew(UsageEnvironment& env, FramedSource* inputSource);

private:
	H264AccessUnitDelimiterInjector(UsageEnvironment& env, FramedSource* inputSource);
	~H264AccessUnitDelimiterInjector();

	virtual void doGetNextFrame();
	static void afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
	void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);

private:
	struct timeval fLastPresentationTime;
	unsigned char* fHoldBuffer;
	unsigned fHoldBufferSize;

	unsigned fSavedFrameSize;
	unsigned fSavedNumTruncatedBytes;
	struct timeval fSavedPresentationTime;
	unsigned fSavedDurationInMicroseconds;
};

#endif
