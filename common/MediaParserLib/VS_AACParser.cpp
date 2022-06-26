#include "VS_AACParser.h"
#include "../std/cpplib/VS_BitField.h"

#include <cstdint>

bool ParseMPEG4AudioConfig(const void* p, size_t size
	, unsigned* object_type
	, unsigned* frequency_index
	, unsigned* channel_configuration
	, unsigned* frequency
)
{
	// http://wiki.multimedia.cx/index.php?title=MPEG-4_Audio
	if (size < 2)
		return false;

	unsigned int offset = 0;

	if (object_type)
		*object_type = GetBitFieldBE(p, offset, 5);
	offset += 5;

	unsigned fi = GetBitFieldBE(p, offset, 4);
	if (frequency_index)
		*frequency_index = fi;
	offset += 4;

	if (fi == 15)
	{
		if (size < 5)
			return false;
		if (frequency)
			*frequency = GetBitFieldBE(p, offset, 24);
		offset += 24;
	}

	if (channel_configuration)
		*channel_configuration = GetBitFieldBE(p, offset, 4);
	offset += 4;

	return true;
}

bool MakeMPEG4AudioConfig(void* p, size_t& size
	, unsigned object_type
	, unsigned frequency_index
	, unsigned channel_configuration
	, unsigned frequency
)
{
	if (size < 2)
	{
		size = 2;
		return false;
	}
	else if (frequency_index == 15 && size < 5)
	{
		size = 5;
		return false;
	}

	unsigned int offset = 0;
	SetBitFieldBE(p, offset, 5, object_type);
	offset += 5;
	SetBitFieldBE(p, offset, 4, frequency_index);
	offset += 4;
	if (frequency_index == 15)
	{
		SetBitFieldBE(p, offset, 24, frequency);
		offset += 24;
	}
	SetBitFieldBE(p, offset, 4, channel_configuration);
	offset += 4;

	size = (offset + 7) / 8;
	return true;
}

bool ParseADTS(const void* p, size_t size
	, unsigned* object_type
	, unsigned* frequency_index
	, unsigned* channel_configuration
	, unsigned* payload_length
)
{
	// http://wiki.multimedia.cx/index.php?title=ADTS
	if (size < 7)
		return false;

	unsigned int offset = 0;

	// syncword
	if (GetBitFieldBE(p, 0, 12) != 0xfff)
		return false;
	offset += 12;

	// MPEG version, layer, protection absent
	offset += 1 + 2 + 1;

	// profile
	if (object_type)
		*object_type = GetBitFieldBE(p, offset, 2) + 1;
	offset += 2;

	// frequency index
	if (frequency_index)
		*frequency_index = GetBitFieldBE(p, offset, 4);
	offset += 4;

	// private bit
	offset += 1;

	// channel configuration
	if (channel_configuration)
		*channel_configuration = GetBitFieldBE(p, offset, 3);
	offset += 3;

	// originality, home, copyright id bit, copyright id start
	offset += 1 + 1 + 1 + 1;

	// frame_length
	if (payload_length)
		*payload_length = GetBitFieldBE(p, offset, 13) - 7;
	offset += 13;

	// buffer fullness, number of frames
	offset += 11 + 2;

	return true;
}

bool MakeADTS(void* p, size_t& size
	, unsigned object_type
	, unsigned frequency_index
	, unsigned channel_configuration
	, unsigned payload_length
)
{
	if (size < 7)
	{
		size = 7;
		return false;
	}

	unsigned int offset = 0;
	SetBitFieldBE(p, offset, 12, 0xfff); // syncword=0xfff
	offset += 12;
	SetBitFieldBE(p, offset, 4, 0x1); // MPEG version=0 (MPEG-4), layer=0, protection absent=1 (No CRC)
	offset += 4;
	SetBitFieldBE(p, offset, 2, object_type-1);
	offset += 2;
	SetBitFieldBE(p, offset, 4, frequency_index);
	offset += 4;
	SetBitFieldBE(p, offset, 1, 0x0); // private bit=0 (ignored)
	offset += 1;
	SetBitFieldBE(p, offset, 3, channel_configuration);
	offset += 3;
	SetBitFieldBE(p, offset, 4, 0x0); // originality=0 (ignored), home=0 (ignored), copyright id bit=0 (ignored), copyright id start=0 (ignored)
	offset += 4;
	SetBitFieldBE(p, offset, 13, 7 + payload_length);
	offset += 13;
	SetBitFieldBE(p, offset, 11, 0x7ff); // buffer fullness=0x7ff (VBR)
	offset += 11;
	SetBitFieldBE(p, offset, 2, 0x0); // number of frames=0 (1 AAC frame per ADTS frame)
	offset += 2;

	assert(offset == 56);
	size = (offset + 7) / 8;
	return true;
}

unsigned SamplingRateFromMPEG4FrequencyIndex(unsigned frequency_index)
{
	static const unsigned tbl[16] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, 0, 0, 0};
	return tbl[frequency_index%16];
}

unsigned MPEG4FrequencyIndexFromSamplingRate(unsigned sampling_rate)
{
	switch (sampling_rate)
	{
	case 96000: return 0;
	case 88200: return 1;
	case 64000: return 2;
	case 48000: return 3;
	case 44100: return 4;
	case 32000: return 5;
	case 24000: return 6;
	case 22050: return 7;
	case 16000: return 8;
	case 12000: return 9;
	case 11025: return 10;
	case  8000: return 11;
	case  7350: return 12;
	}
	return 15;
}
