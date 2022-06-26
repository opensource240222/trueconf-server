#pragma once

#include <cstddef>

enum MPAVersion : unsigned
{
	MPA_V1 = 3,
	MPA_V2 = 2,
	MPA_V2_5 = 0,
};

enum MPALayer : unsigned
{
	MPA_L1 = 3,
	MPA_L2 = 2,
	MPA_L3 = 1,
};

enum MPAChannelMode : unsigned
{
	Stereo = 0,
	JointStereo = 1,
	DualChannel = 2,
	SingleChannel = 3,
};

bool ParseMPAFrameHeader(const void* p, size_t size
	, MPAVersion* version = nullptr
	, MPALayer* layer = nullptr
	, unsigned* bitrate = nullptr
	, unsigned* frequency = nullptr
	, MPAChannelMode* channel_mode = nullptr
);
bool MakeMPAFrameHeader(void* p, size_t& size
	, MPAVersion version
	, MPALayer layer
	, unsigned bitrate
	, unsigned frequency
	, MPAChannelMode channel_mode
);
