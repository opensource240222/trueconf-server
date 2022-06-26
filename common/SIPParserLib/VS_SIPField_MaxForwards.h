#pragma once

#include "../SIPParserBase/VS_BaseField.h"

#include <boost/regex.hpp>

class VS_SIPField_MaxForwards: public VS_BaseField
{
public:
	const static boost::regex e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;


	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	VS_SIPField_MaxForwards();
	~VS_SIPField_MaxForwards();

	int order() const override
	{
		return 35;
	}

protected:
	unsigned int GetValue() const { return iValue; };
	void SetValue(unsigned int aValue) { this->iValue = aValue; }

private:
	unsigned int iValue;

};

std::unique_ptr<VS_BaseField> VS_SIPField_MaxForwards_Instance();