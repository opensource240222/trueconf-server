#pragma once

#include <queue>

#include "VSVideoFile.h"

class VSVideoFileReader : public VSVideoFile
{
public:
	VSVideoFileReader();
	~VSVideoFileReader();

	bool Init(std::string fileName);
	void Release();

	int ReadVideo(void* data, bool* isKey);
	int ReadAudio(void* data);

	unsigned int GetVideoDuration() const;
	unsigned int GetAudioDuration() const;

	bool SeekToTime(int ms);

private:
	bool ReadNextFrameToQueue();

	struct SPacket
	{
		uint8_t* Data;
		int Size;
		bool IsKey;
		unsigned int timeStamp;
	};

	std::queue<SPacket> m_AudioQueue;
	std::queue<SPacket> m_VideoQueue;
};
