#pragma once

#include <string>

class VS_AuthConnectionInterface
{
	std::string m_authenticatedName;
public:
	virtual ~VS_AuthConnectionInterface(){}
	virtual bool AuthConnection(const unsigned char *data,const unsigned long data_sz)	= 0;
	virtual const std::string& GetAuthenticatedName() const = 0;
};