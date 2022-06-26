#include "FrameMetadataPrinter.hh"

FrameMetadataPrinter* FrameMetadataPrinter::createNew(UsageEnvironment& env, FramedSource* inputSource, const char* prefix)
{
	return new FrameMetadataPrinter(env, inputSource, prefix);
}

FrameMetadataPrinter::FrameMetadataPrinter(UsageEnvironment& env, FramedSource* inputSource, const char* prefix)
	: FramedFilter(env, inputSource)
	, fPrefix(prefix)
{
}

void FrameMetadataPrinter::doGetNextFrame()
{
	fInputSource->getNextFrame(fTo, fMaxSize, FrameMetadataPrinter::afterGettingFrame, this, FramedSource::handleClosure, this);
}

void FrameMetadataPrinter::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	static_cast<FrameMetadataPrinter*>(clientData)->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

void FrameMetadataPrinter::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	fFrameSize = frameSize;
	fNumTruncatedBytes = numTruncatedBytes;
	fPresentationTime = presentationTime;
	fDurationInMicroseconds = durationInMicroseconds;

	if (fPrefix)
		envir() << fPrefix;
	printMetadata();
	envir() << "\n";
	afterGetting(this);
}

void FrameMetadataPrinter::printMetadata()
{
	envir() << "pT: " << fPresentationTime << ", duration: " << fDurationInMicroseconds << ", size: " << fFrameSize;
}
