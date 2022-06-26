#include "VS_AudioResamplerBase.h"

VS_AudioResamplerBase::VS_AudioResamplerBase() : m_valid(false), m_inFrequency(0), m_outFrequency(0)
{
}

VS_AudioResamplerBase::~VS_AudioResamplerBase()
{
}

int VS_AudioResamplerBase::Process(const void *inbuffer, void *outbuffer, uint32_t insize, uint32_t inFreq, uint32_t outFreq)
{
	if ((inbuffer == nullptr || outbuffer == nullptr) || ((!m_valid) && (!(m_valid = Init(inFreq, outFreq)))))
		return -1;

	if (inFreq != m_inFrequency || outFreq != m_outFrequency)
	{
		if (!SetRates(inFreq, outFreq))
			return -1;

		m_inFrequency = inFreq;
		m_outFrequency = outFreq;
	}

	return InternalProcess(inbuffer, outbuffer, insize);
}
