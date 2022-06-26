#include "FrameBufferSource.hh"
#include "LogHelper.hh"
#include "timeval_helper.hh"

#include <algorithm>
#include <cassert>
#include <numeric>

#define LOG MEDIUM_LOG(FrameBufferSource)

FrameBufferSource* FrameBufferSource::createNew(UsageEnvironment& env, size_t maxSizeBytes, unsigned maxSizeMicroseconds, bool allowPartialDelivery)
{
	return new FrameBufferSource(env, maxSizeBytes, maxSizeMicroseconds, allowPartialDelivery);
}

FrameBufferSource::FrameBufferSource(UsageEnvironment& env, size_t maxSizeBytes, unsigned maxSizeMicroseconds, bool allowPartialDelivery)
	: FramedSource(env)
	, fMaxSizeBytes(maxSizeBytes)
	, fMaxSizeMicroseconds(maxSizeMicroseconds)
	, fBuffer(4*1024)
	, fClosed(false)
	, fPartialDelivery(false)
	, fNumDroppedFrames(0)
	, fAllowPartialDelivery(allowPartialDelivery)
	, fFrameReadyEventId(envir().taskScheduler().createEventTrigger(&FrameBufferSource::frameReady))
{
}

FrameBufferSource::~FrameBufferSource()
{
	envir().taskScheduler().deleteEventTrigger(fFrameReadyEventId);
}

bool FrameBufferSource::insertFrame(unsigned char* data, size_t size, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	std::lock_guard<std::mutex> lock(fMutex);

	if (fClosed)
		return false;

	if (fMaxSizeBytes > 0 && size > fMaxSizeBytes)
		return false;

	while (size > fBuffer.reserve())
	{
		if ((fMaxSizeBytes == 0 || fBuffer.size()+size < fMaxSizeBytes)
		 && (fMaxSizeMicroseconds == 0 || fFrames.empty() || presentationTime-fFrames.front().presentationTime < std::chrono::microseconds(fMaxSizeMicroseconds))
		   )
		{
			size_t newCapacity = std::max(fBuffer.capacity(), static_cast<size_t>(2));
			while (newCapacity < fBuffer.size()+size)
				newCapacity = newCapacity+newCapacity/2;
			if (fMaxSizeBytes > 0 && newCapacity > fMaxSizeBytes)
				newCapacity = fMaxSizeBytes;
			fBuffer.set_capacity(newCapacity);
		}
		else
		{
			fBuffer.erase_begin(fFrames.front().size);
			fFrames.pop_front();
			++fNumDroppedFrames;
		}
	}
	assert(size <= fBuffer.reserve());
	assert(std::accumulate(fFrames.begin(), fFrames.end(), size_t(0), [](size_t s, const frame_info& x) { return s + x.size; }) == fBuffer.size());
	fBuffer.insert(fBuffer.end(), data, data+size);
	fFrames.emplace_back(size, presentationTime, durationInMicroseconds);

	// Can't check with isCurrentlyAwaitingData() if we realy need to be informed
	// because that check will cause a data race
	envir().taskScheduler().triggerEvent(fFrameReadyEventId, this);

	return true;
}

void FrameBufferSource::insertEnd()
{
	std::lock_guard<std::mutex> lock(fMutex);
	fClosed = true;

	// Can't check with isCurrentlyAwaitingData() if we realy need to be informed
	// because that check will cause a data race
	envir().taskScheduler().triggerEvent(fFrameReadyEventId, this);
}

void FrameBufferSource::reset()
{
	std::lock_guard<std::mutex> lock(fMutex);
	fClosed = false;
	fPartialDelivery = false;
	fBuffer.clear();
	fFrames.clear();
}

void FrameBufferSource::setUnderflowCallback(const underflow_cb_t& cb)
{
	std::lock_guard<std::mutex> lock(fMutex);
	fUnderflowCB = cb;
}

void FrameBufferSource::setUnderflowCallback(underflow_cb_t&& cb)
{
	std::lock_guard<std::mutex> lock(fMutex);
	fUnderflowCB = std::move(cb);
}

void FrameBufferSource::allowPartialDelivery(bool allowPartialDelivery)
{
	std::lock_guard<std::mutex> lock(fMutex);
	fAllowPartialDelivery = allowPartialDelivery;

	if (!fAllowPartialDelivery && fPartialDelivery)
	{
		assert(!fFrames.empty());
		fBuffer.erase_begin(fFrames.front().size);
		fFrames.pop_front();
		fPartialDelivery = false;
	}
}

void FrameBufferSource::doGetNextFrame()
{
	std::unique_lock<std::mutex> lock(fMutex);
	if (!fFrames.empty())
	{
		deliverFrame();
		lock.unlock();
		afterGetting(this);
	}
	else if (fClosed)
	{
		lock.unlock();
		handleClosure();
	}
	else
	{
		if (fUnderflowCB)
		{
			underflow_cb_t cb(fUnderflowCB);
			lock.unlock();
			cb();
		}
	}
}

void FrameBufferSource::frameReady(void* clientData)
{
	static_cast<FrameBufferSource*>(clientData)->frameReady();
}

void FrameBufferSource::frameReady()
{
	if (!isCurrentlyAwaitingData())
		return;
	std::unique_lock<std::mutex> lock(fMutex);
	if (!fFrames.empty())
	{
		deliverFrame();
		lock.unlock();
		afterGetting(this);
	}
	else if (fClosed)
	{
		lock.unlock();
		handleClosure();
	}
}

void FrameBufferSource::deliverFrame()
{
	assert(!fFrames.empty());
	assert(isCurrentlyAwaitingData());

	frame_info& fi = fFrames.front();
	const size_t toCopy = std::min(fi.size, static_cast<size_t>(fMaxSize));
	std::copy(fBuffer.begin(), fBuffer.begin()+toCopy, fTo);
	fFrameSize = toCopy;
	fPresentationTime = fi.presentationTime;
	fDurationInMicroseconds = fi.durationInMicroseconds;

	fPartialDelivery = fAllowPartialDelivery && toCopy < fi.size;
	if (fPartialDelivery)
	{
		fBuffer.erase_begin(toCopy);
		fi.size -= toCopy;
	}
	else
	{
		fNumTruncatedBytes = fi.size - toCopy;
		fBuffer.erase_begin(fi.size);
		fFrames.pop_front();
	}

	if (fNumDroppedFrames > 0)
		LOG << fNumDroppedFrames << " frames was dropped due to buffer overflow since last delivery\n";
	fNumDroppedFrames = 0;
}
