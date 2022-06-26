#include "H264FrameMetadataPrinter.hh"

H264FrameMetadataPrinter* H264FrameMetadataPrinter::createNew(UsageEnvironment& env, FramedSource* inputSource, const char* prefix)
{
	return new H264FrameMetadataPrinter(env, inputSource, prefix);
}

H264FrameMetadataPrinter::H264FrameMetadataPrinter(UsageEnvironment& env, FramedSource* inputSource, const char* prefix)
	: FrameMetadataPrinter(env, inputSource, prefix)
{
}

void H264FrameMetadataPrinter::printMetadata()
{
	FrameMetadataPrinter::printMetadata();
	const u_int8_t nal_unit_type = fFrameSize > 0 ? fTo[0]&0x1F : 0xFF;
	envir() << ", nal_unit_type: " << (unsigned)nal_unit_type;
}
