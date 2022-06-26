#ifndef _TIMESTAMP_SETTER_HH
#define _TIMESTAMP_SETTER_HH

#include <FramedFilter.hh>

#include <memory>
#include <vector>

class TimestampSetter : public FramedFilter
{
public:
	class Action
	{
	public:
		virtual ~Action();

		virtual void apply(struct timeval& presentationTime, unsigned& durationInMicroseconds, struct timeval lastPresentationTime, unsigned lastDurationInMicroseconds) = 0;

	protected:
		TimestampSetter* setter() const { return fSetter; }

	private:
		friend class TimestampSetter;
		TimestampSetter* fSetter;
	};

	static TimestampSetter* createNew(UsageEnvironment& env, FramedSource* inputSource);

	void appendAction(Action* action);

private:
	TimestampSetter(UsageEnvironment& env, FramedSource* inputSource);

	virtual void doGetNextFrame();
	static void afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);
	void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);

private:
	struct timeval fLastPresentationTime;
	unsigned fLastDurationInMicroseconds;

	std::vector<std::unique_ptr<Action>> fActions;
};

class SetPresentationTimeFromDuration : public TimestampSetter::Action
{
public:
	SetPresentationTimeFromDuration(struct timeval initialPresentationTime);
	virtual void apply(struct timeval& presentationTime, unsigned& durationInMicroseconds, struct timeval lastPresentationTime, unsigned lastDurationInMicroseconds);

private:
	struct timeval fInitialPresentationTime;
};

class SetConstantDuration : public TimestampSetter::Action
{
public:
	SetConstantDuration(unsigned durationInMicroseconds);
	virtual void apply(struct timeval& presentationTime, unsigned& durationInMicroseconds, struct timeval lastPresentationTime, unsigned lastDurationInMicroseconds);

private:
	unsigned fValue;
};

class PredictDurationFromPresentationTime : public TimestampSetter::Action
{
public:
	PredictDurationFromPresentationTime(unsigned initialDurationInMicroseconds, unsigned alphaNumerator = 1, unsigned alphaDenominator = 32);
	virtual void apply(struct timeval& presentationTime, unsigned& durationInMicroseconds, struct timeval lastPresentationTime, unsigned lastDurationInMicroseconds);

private:
	unsigned fAlphaNumerator;
	unsigned fAlphaDenominator;
	unsigned fDurationInMicrosecondsEMA;
};

class SyncronizeDurationToPresentationTime : public TimestampSetter::Action
{
public:
	SyncronizeDurationToPresentationTime();
	virtual void apply(struct timeval& presentationTime, unsigned& durationInMicroseconds, struct timeval lastPresentationTime, unsigned lastDurationInMicroseconds);

private:
	long long fDurationCorrection;
};

class LimitPresentationTime : public TimestampSetter::Action
{
public:
	LimitPresentationTime(unsigned maxPresentationTimeDeviationMicroseconds);
	virtual void apply(struct timeval& presentationTime, unsigned& durationInMicroseconds, struct timeval lastPresentationTime, unsigned lastDurationInMicroseconds);

private:
	unsigned fMaxPresentationTimeDeviationMicroseconds;
};

class LimitDuration : public TimestampSetter::Action
{
public:
	LimitDuration(unsigned maxDurationInMicroseconds);
	virtual void apply(struct timeval& presentationTime, unsigned& durationInMicroseconds, struct timeval lastPresentationTime, unsigned lastDurationInMicroseconds);

private:
	unsigned fMaxDurationInMicroseconds;
};

#endif
