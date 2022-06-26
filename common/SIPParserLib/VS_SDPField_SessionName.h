#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

class VS_SDPField_SessionName: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	VS_SDPField_SessionName();
	~VS_SDPField_SessionName();

	int order() const override
	{
		return 30;
	}
private:
	std::string					m_name;
};

std::unique_ptr<VS_BaseField> VS_SDPField_SessionName_Instance();