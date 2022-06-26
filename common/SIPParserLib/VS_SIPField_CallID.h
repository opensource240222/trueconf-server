#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

class VS_SIPField_CallID: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	void Value(std::string);
	const std::string &Value() const;

	VS_SIPField_CallID();
	~VS_SIPField_CallID();

	int order() const override
	{
		return 60;
	}

private:
	std::string	iValue;
	bool	compact;
};

std::unique_ptr<VS_BaseField> VS_SIPField_CallID_Instance();