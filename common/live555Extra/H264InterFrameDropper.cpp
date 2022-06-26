#include "H264InterFrameDropper.hh"
#include "timeval_helper.hh"
#include "LogHelper.hh"

#define LOG FRAMED_FILTER_LOG(H264InterFrameDropper)

H264InterFrameDropper* H264InterFrameDropper::createNew(UsageEnvironment& env, FramedSource* inputSource, Boolean waitForIntraFrame)
{
	return new H264InterFrameDropper(env, inputSource, waitForIntraFrame);
}

H264InterFrameDropper::H264InterFrameDropper(UsageEnvironment& env, FramedSource* inputSource, Boolean waitForIntraFrame)
	: FrameDropper(env, inputSource)
	, fWaitForIntraFrame(waitForIntraFrame)
{
}

static Boolean isSPSorPSPFrame(unsigned char* data, unsigned size)
{
	// Skip NALU start codes
	if (size >= 3 && data[0] == 0 && data[1] == 0 && data[2] == 1)
	{
		data += 3;
		size -= 3;
	}
	else if (size >= 4 && data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 1)
	{
		data += 4;
		size -= 4;
	}

	if (size < 1)
		return False; // too small
	if ((data[0] & 0x80) != 0)
		return False; // forbidden_zero_bit is set which is wrong
	unsigned char nal_unit_type = data[0] & 0x1F;
	return nal_unit_type == 7/*SPS*/ || nal_unit_type == 8/*PPS*/;
}

Boolean H264InterFrameDropper::shouldDrop()
{
	if (!fWaitForIntraFrame)
		return False;

	if (isSPSorPSPFrame(fTo, fFrameSize))
	{
		fWaitForIntraFrame = False;
		if (numDropped() > 0)
			LOG << numDropped() << " frames/NALUs (" << (fPresentationTime - fDropStartPT) << "s) were dropped before intra frame arrived\n";
		return False;
	}

	if (numDropped() == 0)
		fDropStartPT = fPresentationTime;
	return True;
}
