#pragma once

#include <cstddef>

bool ParseMPEG4AudioConfig(const void* p, size_t size
	, unsigned* object_type = nullptr
	, unsigned* frequency_index = nullptr
	, unsigned* channel_configuration = nullptr
	, unsigned* frequency = nullptr
);
bool MakeMPEG4AudioConfig(void* p, size_t& size
	, unsigned object_type
	, unsigned frequency_index
	, unsigned channel_configuration
	, unsigned frequency = 0
);

bool ParseADTS(const void* p, size_t size
	, unsigned* object_type = nullptr
	, unsigned* frequency_index = nullptr
	, unsigned* channel_configuration = nullptr
	, unsigned* payload_length = nullptr
);
bool MakeADTS(void* p, size_t& size
	, unsigned object_type
	, unsigned frequency_index
	, unsigned channel_configuration
	, unsigned payload_length
);

unsigned SamplingRateFromMPEG4FrequencyIndex(unsigned frequency_index);
unsigned MPEG4FrequencyIndexFromSamplingRate(unsigned sampling_rate);
