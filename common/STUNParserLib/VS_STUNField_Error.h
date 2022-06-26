#pragma once

#include "VS_STUNField.h"

class VS_STUNField_Error: public VS_STUNField
{
public:
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	VS_STUNField_Error();
	~VS_STUNField_Error();

protected:

private:
	unsigned iClass : 3;
	char iNumber;
	char* iPhrase;
};

std::unique_ptr<VS_BaseField> VS_STUNField_Error_Instance();