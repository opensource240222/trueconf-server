#pragma once

#include <cinttypes>

class VS_AudioResamplerBase
{
private:

	virtual bool Init(uint32_t oldFreq, uint32_t newFreq) = 0;
	virtual bool SetRates(uint32_t oldFreq, uint32_t newFreq) = 0;
	virtual int /* bytes */ InternalProcess(const void *inbuffer, void *outbuffer, uint32_t insize /* bytes */) = 0;

public:

	VS_AudioResamplerBase();
	virtual ~VS_AudioResamplerBase();
	int /* bytes */ Process(const void *inbuffer, void *outbuffer, uint32_t insize /* bytes */, uint32_t inFreq, uint32_t outFreq);

protected:

	bool m_valid;
	uint32_t m_inFrequency;
	uint32_t m_outFrequency;
};
