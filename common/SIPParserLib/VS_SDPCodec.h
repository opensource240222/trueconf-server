#pragma once

#include <boost/regex.hpp>

#include <string>
#include "../SIPParserBase/VS_BaseField.h"

struct VS_GatewayAudioMode;
struct VS_GatewayVideoMode;

enum class SDPMediaType : int;

class VS_SDPCodec: public VS_BaseField
{
public:
	const static boost::regex e;
	const static boost::regex e2;

	VS_SDPCodec();
	VS_SDPCodec(const VS_SDPCodec&);
	virtual ~VS_SDPCodec();

	VS_SDPCodec& operator=(const VS_SDPCodec& codec);
	virtual std::unique_ptr<VS_SDPCodec> Clone() const = 0;

	TSIPErrorCodes Decode(VS_SIPBuffer &_buffer) override = 0;		// Знает как работать с "rtpmap: .... "
	TSIPErrorCodes Encode(VS_SIPBuffer &_buffer) const override = 0;


	virtual void FillRcvAudioMode(VS_GatewayAudioMode& /*mode*/) const {}
	virtual void FillRcvVideoMode(VS_GatewayVideoMode& /*mode*/) const {}
	virtual void FillSndVideoMode(VS_GatewayVideoMode& /*mode*/) const {}

	void SetPT(int _pt);
	int GetPT() const;

	void SetClockRate(std::uint32_t rate);
	std::uint32_t GetClockRate() const;

	void SetNumChannels(unsigned int _n);
	unsigned int GetNumChannels() const;

	void SetCodecType(int _c);
	int GetCodecType() const;

	void SetEncodingName(std::string _name);
	const std::string &GetEncodingName() const;

	void SetMediaType(SDPMediaType mt);
	SDPMediaType GetMediaType() const;

protected:
	int				m_pt;					// Копия из VS_SDPField_MediaStream

	bool FindParam_bitrate(string_view _in, unsigned long &bitrate) const;
	bool operator!=(const VS_SDPCodec& codec) const;

private:
	int				m_codec_type;			// VS_H323AudioCodec или VS_H323VideoCodec (смотреть CommonTypes.h)
											// зависит от m_media_type
	SDPMediaType	m_media_type;

	unsigned int	m_clock_rate;
	unsigned int	m_num_channels;
	std::string		m_encoding_name;
};

class VS_SDPRtpmapDecoder final : public VS_SDPCodec
{
	TSIPErrorCodes Encode(VS_SIPBuffer&) const override
	{ return TSIPErrorCodes::e_ok; }

public:
	TSIPErrorCodes Decode(VS_SIPBuffer &_buffer) override;
	std::unique_ptr<VS_SDPCodec> Clone() const override;
};