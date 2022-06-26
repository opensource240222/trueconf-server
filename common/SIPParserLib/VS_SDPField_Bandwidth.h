#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

enum eSDP_Bandwidth;

class VS_SDPField_Bandwidth: public VS_BaseField
{
public:
	const static boost::regex e;

	VS_SDPField_Bandwidth();
	VS_SDPField_Bandwidth(const VS_SDPField_Bandwidth&) = delete;
	~VS_SDPField_Bandwidth();

	VS_SDPField_Bandwidth& operator=(const VS_SDPField_Bandwidth &_bandwidth) = delete;

	void Clean() noexcept override;

	TSIPErrorCodes Decode(VS_SIPBuffer &_buffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &_buffer) const override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	void SetBandwidth(const unsigned int _bandwidth);
	unsigned int GetBandwidth() const;

	void SetType(const eSDP_Bandwidth _type);
	eSDP_Bandwidth GetType() const;

	int order() const override
	{
		return 50;
	}

private:
	eSDP_Bandwidth			m_type;							// CT | AS | TIAS
	unsigned int			m_bandwidth;					// —читаетс€ в бит/с
};

std::unique_ptr<VS_BaseField> VS_SDPField_Bandwidth_Instance();