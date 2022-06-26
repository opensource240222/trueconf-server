#pragma once

#include <string>

class VS_TransportRouterServiceBase;

namespace transport {
struct IRouter{
	virtual ~IRouter() {}
	virtual bool	AddService(const char *serviceName,
		VS_TransportRouterServiceBase *service, bool withOwnThread = true,
		const bool permittedForAll = false) = 0;
	virtual bool	RemoveService(const char *serviceName) = 0;
	virtual const std::string& EndpointName() const = 0;
};
}