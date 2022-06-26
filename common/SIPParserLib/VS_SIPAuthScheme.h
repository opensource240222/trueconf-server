#pragma once

#include "../SIPParserBase/VS_BaseField.h"
#include "VS_SIPAuthInfo.h"

enum eSIP_AUTH_SCHEME;

class VS_SIPAuthScheme: public VS_BaseField,
						public VS_SIPAuthInfo
{
public:
	VS_SIPAuthScheme()
	{	}

	virtual ~VS_SIPAuthScheme()
	{	}

	virtual eSIP_AUTH_SCHEME scheme() const = 0;
};