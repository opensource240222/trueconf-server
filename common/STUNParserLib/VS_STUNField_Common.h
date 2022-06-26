#pragma once

#include "VS_STUNField.h"

class VS_STUNField_Common: public VS_STUNField
{
public:
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	VS_STUNField_Common();
	~VS_STUNField_Common();

protected:

private:
	char* iValue;
};

std::unique_ptr<VS_BaseField> VS_STUNField_Common_Instance();