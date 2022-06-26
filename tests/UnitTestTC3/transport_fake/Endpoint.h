#pragma once
#include "std-generic/cpplib/VS_Container.h"
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>

class VS_RouterMessage;
class VS_TransportRouterServiceBase;
namespace tc3_test
{
class FakeRouter;
class TransportRouterService_fake;

class Endpoint
{
public:
	using MsgTraceType = std::tuple<std::string, std::string,VS_Container,std::string,std::string,std::string>; // dst_service, method, container, src server, src user, src service
private:
	std::shared_ptr<FakeRouter> router_;
	std::string endpoint_name_;
	std::map < std::string, std::tuple<boost::shared_ptr<VS_TransportRouterServiceBase>, TransportRouterService_fake*> > services_;
	std::vector<MsgTraceType>	msg_trace_;
	void Processing(VS_RouterMessage*);

public:


	Endpoint(const std::string &name, const std::shared_ptr<FakeRouter>&router);
	void OnPointConnected(const std::string&ep_name);
	void OnPointDisconnected(const std::string&ep_name);

	void RegisterService(const std::string &name, const boost::shared_ptr<VS_TransportRouterServiceBase> &, TransportRouterService_fake*);
	void UnregisterService(const std::string&name);
	void GetMsgTrace(std::vector<MsgTraceType> &out);
	const char * EndpointName() const
	{
		return endpoint_name_.c_str();
	}
	auto GetRouter() const -> const decltype(router_)&
	{return router_; }
};

class ClientEndpoint
{
	std::shared_ptr<FakeRouter> router_;
	std::string name_;

	void Processing(VS_RouterMessage*);
public:
	ClientEndpoint(const char *user_name, const std::shared_ptr<FakeRouter> &r);
	~ClientEndpoint();

};
}