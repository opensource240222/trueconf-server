#pragma once
#include "../../std/cpplib/VS_SimpleStr.h"
struct VS_DNSServerParam
{
	VS_SimpleStr	iHost;
	unsigned int	iPort;

	VS_DNSServerParam() : iPort(0) {	}
	VS_DNSServerParam& operator =(const VS_DNSServerParam&other)
	{
		iHost = other.iHost;
		iPort = other.iPort;
		return *this;
	}

};