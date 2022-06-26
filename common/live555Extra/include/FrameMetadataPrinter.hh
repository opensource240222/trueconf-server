#ifndef _FRAME_METADATA_PRINTER_HH
#define _FRAME_METADATA_PRINTER_HH

#include <FramedFilter.hh>

class FrameMetadataPrinter : public FramedFilter
{
public:
	static FrameMetadataPrinter* createNew(UsageEnvironment& env, FramedSource* inputSource, const char* prefix = NULL);

protected:
	FrameMetadataPrinter(UsageEnvironment& env, FramedSource* inputSource, const char* prefix);
	virtual void printMetadata();

private:
	virtual void doGetNextFrame();
	static void afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
	void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);

private:
	const char* fPrefix;
};

#endif
