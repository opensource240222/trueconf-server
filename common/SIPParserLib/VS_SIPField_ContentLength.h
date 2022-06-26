#pragma once

#include <boost/regex.hpp>
#include "../SIPParserBase/VS_BaseField.h"

class VS_SIPField_ContentLength: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	VS_SIPField_ContentLength();
	~VS_SIPField_ContentLength();

	unsigned int Value() const;
	void Value(unsigned int value);

private:
	unsigned int iValue;
	bool compact;
};

std::unique_ptr<VS_BaseField> VS_SIPField_ContentLength_Instance();