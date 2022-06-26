#include "LateFrameDropper.hh"
#include "timeval_helper.hh"
#include "LogHelper.hh"

#include <GroupsockHelper.hh>

#include <utility>

#define LOG FRAMED_FILTER_LOG(LateFrameDropper)

LateFrameDropper* LateFrameDropper::createNew(UsageEnvironment& env, FramedSource* inputSource, unsigned timeoutMaxMicroseconds, unsigned timeoutMinMicroseconds)
{
	return new LateFrameDropper(env, inputSource, timeoutMaxMicroseconds, timeoutMinMicroseconds);
}

LateFrameDropper::LateFrameDropper(UsageEnvironment& env, FramedSource* inputSource, unsigned timeoutMaxMicroseconds, unsigned timeoutMinMicroseconds)
	: FrameDropper(env, inputSource)
	, fTimeoutMaxMicroseconds(timeoutMaxMicroseconds)
	, fTimeoutMinMicroseconds(timeoutMinMicroseconds)
{
	using std::swap;
	if (fTimeoutMinMicroseconds > fTimeoutMaxMicroseconds)
		swap(fTimeoutMinMicroseconds, fTimeoutMaxMicroseconds);
}

Boolean LateFrameDropper::shouldDrop()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval lag(now-fPresentationTime);

	if (lag > std::chrono::microseconds(fTimeoutMaxMicroseconds)
		|| (numDropped() > 0 && lag > std::chrono::microseconds(fTimeoutMinMicroseconds))
	)
	{
		if (numDropped() == 0)
			LOG << "dropping frames: " << lag << "s late\n";
		else if (numDropped()%1500 == 0)
			LOG << "dropping frames: " << numDropped() << " dropped so far, still " << lag << "s late\n";
		return True;
	}
	else
	{
		if (numDropped() > 0)
			LOG << "done dropping frames: " << numDropped() << " dropped total\n";
		return False;
	}
}
