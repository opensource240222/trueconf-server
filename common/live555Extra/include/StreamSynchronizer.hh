#ifndef _STREAM_SYNCHRONIZER_HH
#define _STREAM_SYNCHRONIZER_HH

#include <Media.hh>
#include <FramedSource.hh>

class StreamSynchronizer : public Medium
{
public:
	static StreamSynchronizer* createNew(UsageEnvironment& env);

	FramedSource* synchronizeSource(FramedSource* source);
	Boolean isSynchronized() const
	{
		return fNumSources == fNumReadySources;
	}

private:
	StreamSynchronizer(UsageEnvironment& env);

private:
	friend class SynchronizedSource;
	unsigned fNumSources;
	unsigned fNumReadySources;
};

#endif
