#pragma once

#include "VS_SDPCodec.h"

class VS_SDPCodecMPEG4ES : public VS_SDPCodec
{
public:
	explicit VS_SDPCodecMPEG4ES(const char* mode = "");
	~VS_SDPCodecMPEG4ES();

	TSIPErrorCodes Decode(VS_SIPBuffer &_buffer)override;
	TSIPErrorCodes Encode(VS_SIPBuffer &_buffer) const override;
	std::unique_ptr<VS_SDPCodec> Clone() const override;

	void FillRcvAudioMode(VS_GatewayAudioMode &mode) const override;

private:
	std::string m_mode;
	uint8_t m_size_length;
	uint8_t m_index_length;
	uint8_t m_index_delta_length;
	uint16_t m_constant_size;
	std::vector<uint8_t> m_config;
};