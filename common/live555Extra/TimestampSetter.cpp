#include "TimestampSetter.hh"
#include <GroupsockHelper.hh>
#include "timeval_helper.hh"
#include "LogHelper.hh"

#include "../std/cpplib/numerical.h"

#define LOG FRAMED_FILTER_LOG(TimestampSetter)

TimestampSetter* TimestampSetter::createNew(UsageEnvironment& env, FramedSource* inputSource)
{
	return new TimestampSetter(env, inputSource);
}

TimestampSetter::TimestampSetter(UsageEnvironment& env, FramedSource* inputSource)
	: FramedFilter(env, inputSource)
	, fLastPresentationTime({0, 0})
	, fLastDurationInMicroseconds(0)
{
}

void TimestampSetter::appendAction(Action* action)
{
	fActions.emplace_back(action);
}

void TimestampSetter::doGetNextFrame()
{
	fInputSource->getNextFrame(fTo, fMaxSize, TimestampSetter::afterGettingFrame, this, FramedSource::handleClosure, this);
}

void TimestampSetter::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	static_cast<TimestampSetter*>(clientData)->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

void TimestampSetter::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	fFrameSize = frameSize;
	fNumTruncatedBytes = numTruncatedBytes;
	fPresentationTime = presentationTime;
	fDurationInMicroseconds = durationInMicroseconds;

	for (const auto& action: fActions)
		action->apply(fPresentationTime, fDurationInMicroseconds, fLastPresentationTime, fLastDurationInMicroseconds);
#ifdef DEBUG
	if (durationInMicroseconds > 0 || fDurationInMicroseconds > 0)
		LOG << "pT: " << presentationTime << " -> " << fPresentationTime << ", duration: " << durationInMicroseconds << " -> " << fDurationInMicroseconds << "\n";
#endif

	fLastPresentationTime = fPresentationTime;
	fLastDurationInMicroseconds = fDurationInMicroseconds;
	afterGetting(this);
}

TimestampSetter::Action::~Action()
{
}

SetPresentationTimeFromDuration::SetPresentationTimeFromDuration(struct timeval initialPresentationTime)
	: fInitialPresentationTime(initialPresentationTime)
{
}

void SetPresentationTimeFromDuration::apply(struct timeval& presentationTime, unsigned& durationInMicroseconds, struct timeval lastPresentationTime, unsigned lastDurationInMicroseconds)
{
	if (lastPresentationTime == timeval{ 0, 0 })
	{
		presentationTime = fInitialPresentationTime;
		return;
	}
	presentationTime = lastPresentationTime + std::chrono::microseconds(lastDurationInMicroseconds);
}

SetConstantDuration::SetConstantDuration(unsigned durationInMicroseconds)
	: fValue(durationInMicroseconds)
{
}

void SetConstantDuration::apply(struct timeval& presentationTime, unsigned& durationInMicroseconds, struct timeval lastPresentationTime, unsigned lastDurationInMicroseconds)
{
	durationInMicroseconds = fValue;
}

PredictDurationFromPresentationTime::PredictDurationFromPresentationTime(unsigned initialDurationInMicroseconds, unsigned alphaNumerator, unsigned alphaDenominator)
	: fDurationInMicrosecondsEMA(initialDurationInMicroseconds)
	, fAlphaNumerator(alphaNumerator)
	, fAlphaDenominator(alphaDenominator)
{
}

void PredictDurationFromPresentationTime::apply(struct timeval& presentationTime, unsigned& durationInMicroseconds, struct timeval lastPresentationTime, unsigned lastDurationInMicroseconds)
{
	if (lastPresentationTime == timeval{0, 0})
	{
		durationInMicroseconds = fDurationInMicrosecondsEMA;
		return;
	}
	const struct timeval diff(presentationTime - lastPresentationTime);
	const long long realLastDurationInMicroseconds(diff.tv_sec*1000000ll + diff.tv_usec);
	durationInMicroseconds = fDurationInMicrosecondsEMA = clamp_cast<unsigned>(
		(fAlphaNumerator*realLastDurationInMicroseconds + (fAlphaDenominator-fAlphaNumerator)*fDurationInMicrosecondsEMA)/fAlphaDenominator
	);
}

SyncronizeDurationToPresentationTime::SyncronizeDurationToPresentationTime()
	: fDurationCorrection(0)
{
}

void SyncronizeDurationToPresentationTime::apply(struct timeval& presentationTime, unsigned& durationInMicroseconds, struct timeval lastPresentationTime, unsigned lastDurationInMicroseconds)
{
	if (lastPresentationTime == timeval{0, 0})
		return;
	const struct timeval diff(presentationTime - lastPresentationTime);
	const long long realLastDurationInMicroseconds(diff.tv_sec*1000000ll + diff.tv_usec);
	fDurationCorrection += realLastDurationInMicroseconds - lastDurationInMicroseconds;
	durationInMicroseconds = clamp_cast<unsigned>(durationInMicroseconds + fDurationCorrection);
}

LimitPresentationTime::LimitPresentationTime(unsigned maxPresentationTimeDeviationMicroseconds)
	: fMaxPresentationTimeDeviationMicroseconds(maxPresentationTimeDeviationMicroseconds)
{
}

void LimitPresentationTime::apply(struct timeval& presentationTime, unsigned& durationInMicroseconds, struct timeval lastPresentationTime, unsigned lastDurationInMicroseconds)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval diff(presentationTime - now);
	if (abs(diff) > std::chrono::microseconds(fMaxPresentationTimeDeviationMicroseconds))
		presentationTime = now;
}

LimitDuration::LimitDuration(unsigned maxDurationInMicroseconds)
	: fMaxDurationInMicroseconds(maxDurationInMicroseconds)
{
}

void LimitDuration::apply(struct timeval& presentationTime, unsigned& durationInMicroseconds, struct timeval lastPresentationTime, unsigned lastDurationInMicroseconds)
{
	if (durationInMicroseconds > fMaxDurationInMicroseconds)
		durationInMicroseconds = fMaxDurationInMicroseconds;
}
