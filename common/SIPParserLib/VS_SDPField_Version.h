#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

class VS_SDPField_Version: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	VS_SDPField_Version();
	~VS_SDPField_Version();

	int order() const override
	{
		return 10;
	}
private:
	unsigned int iValue;
};

std::unique_ptr<VS_BaseField> VS_SDPField_Version_Instance();