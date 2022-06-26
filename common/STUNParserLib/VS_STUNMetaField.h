#pragma once

#include <vector>
//#include "VS_SIPMetaContainer.h"
#include "VS_STUNObjectFactory.h"

// STUN Fields
#include "VS_STUNField_Common.h"

class VS_STUNMetaField: public VS_STUNField
{
public:
	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	VS_STUNMetaField();
	~VS_STUNMetaField();

protected:
	VS_STUNObjectFactory* iFactory;
	//VS_SIPMetaContainer* iContainer;
	std::vector<VS_BaseField*> iContainer;

private:
	friend class VS_STUNClient;

	char* iTransactionID;
};