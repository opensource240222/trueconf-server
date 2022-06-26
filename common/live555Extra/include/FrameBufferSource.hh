#ifndef _FRAME_BUFFER_SOURCE_HH
#define _FRAME_BUFFER_SOURCE_HH

#include <FramedSource.hh>

#include <boost/circular_buffer.hpp>

#include <deque>
#include <functional>
#include <mutex>

class FrameBufferSource : public FramedSource
{
private:
	struct frame_info
	{
		size_t size;
		struct timeval presentationTime;
		unsigned durationInMicroseconds;

		frame_info(size_t size_, struct timeval presentationTime_, unsigned durationInMicroseconds_)
			: size(size_)
			, presentationTime(presentationTime_)
			, durationInMicroseconds(durationInMicroseconds_)
		{}
	};

public:
	static FrameBufferSource* createNew(UsageEnvironment& env, size_t maxSizeBytes, unsigned maxSizeMicroseconds, bool allowPartialDelivery = false);

	bool insertFrame(unsigned char* data, size_t size, struct timeval presentationTime, unsigned durationInMicroseconds);
	void insertEnd();
	void reset();

	typedef std::function<void(void)> underflow_cb_t;
	void setUnderflowCallback(const underflow_cb_t& cb);
	void setUnderflowCallback(underflow_cb_t&& cb);

	bool allowPartialDelivery() const
	{
		return fAllowPartialDelivery;
	}

	void allowPartialDelivery(bool allowPartialDelivery);

private:
	FrameBufferSource(UsageEnvironment& env, size_t maxSizeBytes, unsigned maxSizeMicroseconds, bool allowPartialDelivery);
	~FrameBufferSource();

	virtual void doGetNextFrame();
	static void frameReady(void* clientData);
	void frameReady();

	void deliverFrame();

private:
	std::mutex fMutex;
	std::deque<frame_info> fFrames;
	boost::circular_buffer<unsigned char> fBuffer;
	bool fClosed;
	bool fPartialDelivery;
	underflow_cb_t fUnderflowCB;
	unsigned fNumDroppedFrames;

	size_t fMaxSizeBytes;
	unsigned fMaxSizeMicroseconds;
	bool fAllowPartialDelivery;
	EventTriggerId fFrameReadyEventId;
};

#endif
