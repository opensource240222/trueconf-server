#pragma once

#include "../SIPParserBase/VS_BaseField.h"
#include <boost/regex.hpp>
#include <string>

std::unique_ptr<VS_BaseField> VS_SIPField_Accept_Instance();

class VS_SIPField_Accept : public VS_BaseField
{
public:
	const static boost::regex e;

	VS_SIPField_Accept();

	TSIPErrorCodes Decode(VS_SIPBuffer &buf) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &buf)const override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
private:

	std::string value_;
};
