#pragma once

#include "VS_STUNField.h"

class VS_STUNField_Address: public VS_STUNField
{
public:
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	VS_STUNField_Address();
	~VS_STUNField_Address();

protected:

private:
	friend class VS_STUNClient;

	unsigned long iAddress;
	char iFamily;
	unsigned short iPort;
};

std::unique_ptr<VS_BaseField> VS_STUNField_Address_Instance();