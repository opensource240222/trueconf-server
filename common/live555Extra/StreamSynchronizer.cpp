#include "StreamSynchronizer.hh"
#include "FrameDropper.hh"

class SynchronizedSource : public FrameDropper
{
public:
	static SynchronizedSource* createNew(UsageEnvironment& env, FramedSource* inputSource, StreamSynchronizer* synchronizer)
	{
		return new SynchronizedSource(env, inputSource, synchronizer);
	}

private:
	SynchronizedSource(UsageEnvironment& env, FramedSource* inputSource, StreamSynchronizer* synchronizer)
		: FrameDropper(env, inputSource)
		, fSynchronizer(synchronizer)
		, fSourceReady(False)
		, fSynchronized(False)
	{
		++fSynchronizer->fNumSources;
	}

	~SynchronizedSource()
	{
		if (fSourceReady)
			--fSynchronizer->fNumReadySources;
		--fSynchronizer->fNumSources;
	}

	virtual Boolean shouldDrop()
	{
		if (!fSourceReady)
		{
			fSourceReady = True;
			++fSynchronizer->fNumReadySources;
		}
		fSynchronized = fSynchronized || fSynchronizer->isSynchronized();
		return !fSynchronized;
	}

private:
	StreamSynchronizer* fSynchronizer;
	Boolean fSourceReady;
	Boolean fSynchronized;
};

StreamSynchronizer* StreamSynchronizer::createNew(UsageEnvironment& env)
{
	return new StreamSynchronizer(env);
}

StreamSynchronizer::StreamSynchronizer(UsageEnvironment& env)
	: Medium(env)
	, fNumSources(0)
	, fNumReadySources(0)
{
}

FramedSource* StreamSynchronizer::synchronizeSource(FramedSource* source)
{
	return SynchronizedSource::createNew(envir(), source, this);
}
