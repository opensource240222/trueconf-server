#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>
#include <chrono>

class VS_SIPField_Expires: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	std::chrono::seconds Value() const;
	void Value(std::chrono::seconds value);

	VS_SIPField_Expires();
	~VS_SIPField_Expires();

private:
	std::chrono::seconds iValue;
};

std::unique_ptr<VS_BaseField> VS_SIPField_Expires_Instance();