#pragma once

#include "std/cpplib/event.h"
#include "std/cpplib/VS_MediaFormat.h"

#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

class VideoCodec;

class VS_VideoCodecState
{
public:
	enum eStateLayerSVC
	{
		STATE_IDLE = 0,
		STATE_WAIT_IDLE = 1,
		STATE_WAIT_SKIP,
		STATE_SKIP
	};


	std::unique_ptr<VideoCodec> cdc;
	std::vector<std::unique_ptr<uint8_t[]>> encodeFrame;
	std::vector<int> encodeSize;
	uint32_t shift = 0;
	uint32_t svc = 0;
	uint8_t	*srcFrame = nullptr;
	bool keyFrame = false;
	bool shutdown = false;
	vs::event eventStartEncode{ false };
	vs::event eventEndEncode{ false };
	std::thread thread;
	eStateLayerSVC state;
	VS_MediaFormat format;

	VS_VideoCodecState(const VS_MediaFormat &mf);
	~VS_VideoCodecState();

	void PushEncode(uint8_t *frame, bool key);
	void EncodeRoutine(uint8_t *src, int32_t idx);
};
