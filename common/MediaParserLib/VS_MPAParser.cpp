#include "VS_MPAParser.h"
#include "../std/cpplib/VS_BitField.h"

#include <cassert>

static unsigned MPA_bitrate_tbl[][16] = {
	{ 0/*free*/, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0/*bad*/ }, // MPEG 1 layer 1
	{ 0/*free*/, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 0/*bad*/ }, // MPEG 1 layer 2
	{ 0/*free*/, 32, 40, 48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 0/*bad*/ }, // MPEG 1 layer 3
	{ 0/*free*/, 32, 48, 56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, 0/*bad*/ }, // MPEG 2,2.5 layer 1
	{ 0/*free*/,  8, 16, 24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0/*bad*/ }, // MPEG 2,2.5 layer 2,3
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
static unsigned MPA_bitrate_tbl_index[4/*version*/][4/*layer*/] = {
	{ 5, 4, 4, 3 }, // MPEG 2.5
	{ 5, 5, 5, 5 }, // reserved
	{ 5, 4, 4, 3 }, // MPEG 2
	{ 5, 2, 1, 0 }, // MPEG 1
};
static unsigned MPA_frequency_tbl[4/*version*/][4] = {
	{ 11025, 12000,  8000, 0/*reserved*/ }, // MPEG 2.5
	{     0,     0,     0, 0             }, // reserved
	{ 22050, 24000, 16000, 0/*reserved*/ }, // MPEG 2
	{ 44100, 48000, 32000, 0/*reserved*/ }, // MPEG 1
};

bool ParseMPAFrameHeader(const void* p, size_t size
	, MPAVersion* version
	, MPALayer* layer
	, unsigned* bitrate
	, unsigned* frequency
	, MPAChannelMode* channel_mode
	)
{
	// http://www.mp3-tech.org/programmer/frame_header.html
	if (size < 4)
		return false;

	unsigned int offset = 0;

	// Frame sync
	if (GetBitFieldBE(p, offset, 11) != 0x7ff)
		return false;
	offset += 11;

	// MPEG Audio version ID
	MPAVersion v = static_cast<MPAVersion>(GetBitFieldBE(p, offset, 2));
	offset += 2;
	if (version)
		*version = v;

	// Layer description
	MPALayer l = static_cast<MPALayer>(GetBitFieldBE(p, offset, 2));
	offset += 2;
	if (layer)
		*layer = l;

	// Protection bit
	offset += 1;

	// Bitrate index
	if (bitrate)
		*bitrate = MPA_bitrate_tbl[MPA_bitrate_tbl_index[v][l]][GetBitFieldBE(p, offset, 4)];
	offset += 4;

	// Sampling rate frequency index
	if (frequency)
		*frequency = MPA_frequency_tbl[v][GetBitFieldBE(p, offset, 2)];
	offset += 2;

	// Padding bit, Private bit
	offset += 1 + 1;

	// Channel Mode
	if (channel_mode)
		*channel_mode = static_cast<MPAChannelMode>(GetBitFieldBE(p, offset, 2));
	offset += 2;

	// Mode extension, Copyright, Original, Emphasis
	offset += 2 + 1 + 1 + 2;

	assert(offset == 32);
	return true;
}

bool MakeMPAFrameHeader(void* p, size_t& size
	, MPAVersion version
	, MPALayer layer
	, unsigned bitrate
	, unsigned frequency
	, MPAChannelMode channel_mode
	)
{
	if (size < 4)
	{
		size = 4;
		return false;
	}

	unsigned int offset = 0;
	SetBitFieldBE(p, offset, 11, 0x7ff); // Frame sync = 0x7ff
	offset += 11;
	SetBitFieldBE(p, offset, 2, static_cast<unsigned>(version)); // MPEG Audio version ID
	offset += 2;
	SetBitFieldBE(p, offset, 2, static_cast<unsigned>(layer)); // Layer description
	offset += 2;
	SetBitFieldBE(p, offset, 1, 0); // Protection bit = 0
	offset += 1;

	unsigned bi;
	for (bi = 0; bi < 16; ++bi)
		if (bitrate == MPA_bitrate_tbl[MPA_bitrate_tbl_index[version][layer]][bi])
			break;
	if (bi >= 16)
		return false;
	SetBitFieldBE(p, offset, 4, bi); // Bitrate index
	offset += 4;

	unsigned fi;
	for (fi = 0; fi < 4; ++fi)
		if (frequency == MPA_frequency_tbl[version][fi])
			break;
	if (fi >= 4)
		return false;
	SetBitFieldBE(p, offset, 2, fi); // Sampling rate frequency index
	offset += 2;

	SetBitFieldBE(p, offset, 2, 0); // Padding bit = 0, Private bit = 0
	offset += 1 + 1;
	SetBitFieldBE(p, offset, 2, static_cast<unsigned>(channel_mode)); // Channel Mode
	offset += 2;
	SetBitFieldBE(p, offset, 6, 0); // Mode extension = 0, Copyright = 0, Original = 0, Emphasis = 0
	offset += 6;

	assert(offset == 32);
	size = 4;
	return true;
}