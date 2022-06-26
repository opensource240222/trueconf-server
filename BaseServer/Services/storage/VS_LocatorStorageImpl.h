#pragma once
#include <string>
#include "std-generic/cpplib/string_view.h"
class  VS_LocatorStorageImpl
{
public:
	 VS_LocatorStorageImpl();
	virtual ~VS_LocatorStorageImpl();

	/* password can be used to find language i.e $5~#en or $5~#ru it's helpfull for database*/
	virtual bool GetServerByLogin(const std::string& login, const string_view passwd, std::string& server) = 0;
	virtual bool GetServerByCallID(const std::string& user, std::string & server) = 0;
	virtual bool IsValid() const = 0;
private:

};
