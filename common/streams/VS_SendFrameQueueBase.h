#pragma once

#include <cstdint>

const int VIDEODATASIZE_DEFAULT = 1400;
const int VIDEODATASIZE_MIN = 500;
const int VIDEODATASIZE_MAX = 4000;

namespace stream
{
	struct SVCHeader;
}

enum VS_FramePriority {
	FRAME_PRIORITY_COMMAND = 1,
	FRAME_PRIORITY_AUDIO = 2,
	FRAME_PRIORITY_VIDEO = 3,
	FRAME_PRIORITY_DATA = 4,
	FRAME_PRIORITY_SHARE = 5
};

struct FrameQueueInfo
{
	uint8_t counter = 0;
	uint8_t type = 0;
	uint32_t timestamp = 0;
	uint32_t interval = 0;
};

class VS_SendFrameQueueBase
{
protected:
	int m_LimitSize;

public:
	/// Retrive TCP or NHP interfase
	static VS_SendFrameQueueBase* Factory(bool useNhp, bool useSVC = false);
	/// do nothing
	virtual ~VS_SendFrameQueueBase() {}
	/// Coupling frame and add packets in queue with priority, return 'true' if successfully
	virtual bool AddFrame(int track, int size, unsigned char *pBuffer, int iPriority) = 0;
	/// Coupling frame and add packets in queue with priority, return 'true' if successfully
	virtual bool AddFrame(int track, int size, unsigned char *pBuffer, int iPriority, const FrameQueueInfo &info, const stream::SVCHeader *h) = 0;
	/// Copy reference to first packet from send queue, return queue size
	virtual int GetFrame(unsigned char* &pBuffer, int &size, unsigned char &track, unsigned char &slayer) = 0;
	/// Return queue size
	virtual int GetSize() = 0;
	/// Return queue size in bytes
	virtual int GetBytes() = 0;
	/// Erase first packet from send queue, put it to resend queue
	virtual bool MarkFirstAsSend() = 0;
	/// Erase all audio and video packets
	virtual void EraseMedia() = 0;
	/// Erase all video packets
	virtual void EraseVideo() = 0;
	/// Erase all audio packets
	virtual void EraseAudio() = 0;

	void SetLimitSize(int limitSize);
};
