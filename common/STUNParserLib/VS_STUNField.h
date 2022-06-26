#pragma once

#include "../SIPParserBase/VS_BaseField.h"
#include <memory>

class VS_STUNField: public VS_BaseField
{
public:
	VS_STUNField();
	virtual ~VS_STUNField();

	short int iType;
    unsigned short int iLength;

protected:

private:
//	friend class VS_STUNClient;
};