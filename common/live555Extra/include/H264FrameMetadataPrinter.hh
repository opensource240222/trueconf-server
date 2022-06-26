#ifndef _H264_FRAME_METADATA_PRINTER_HH
#define _H264_FRAME_METADATA_PRINTER_HH

#include <FrameMetadataPrinter.hh>

class H264FrameMetadataPrinter : public FrameMetadataPrinter
{
public:
	static H264FrameMetadataPrinter* createNew(UsageEnvironment& env, FramedSource* inputSource, const char* prefix = NULL);

private:
	H264FrameMetadataPrinter(UsageEnvironment& env, FramedSource* inputSource, const char* prefix);

protected:
	virtual void printMetadata();
};

#endif
