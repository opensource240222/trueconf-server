#pragma once

#include "VS_STUNField.h"
#include "../STUNClientLib/VS_STUNClient.h"

class VS_STUNField_Change: public VS_STUNField
{
public:
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	VS_STUNField_Change();
	~VS_STUNField_Change();

protected:

private:
	friend class VS_STUNClient;

	bool iChangeIP;
	bool iChangePort;
};

std::unique_ptr<VS_BaseField> VS_STUNField_Change_Instance();