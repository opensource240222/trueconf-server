#include "DataFetcherBase.h"
#include "net/EndpointRegistry.h"

namespace mdns
{

namespace
{
	const size_t NAME_BUFFER_SIZE = 255;
}


DataFetcherBase::DataFetcherBase(std::string companyName, std::string webURL):
	organizationName_(std::move(companyName)),
	webURL_(std::move(webURL)),
	serverName_(g_tr_endpoint_name)
	{}

std::string& DataFetcherBase::getCompanyName()
{
	return organizationName_;
}

std::string& DataFetcherBase::getServerName()
{
	return serverName_;
}

std::string& DataFetcherBase::getWebURL()
{
	return webURL_;
}

unsigned int DataFetcherBase::getConnectTcpCount()
{
	return net::endpoint::GetCountConnectTCP(serverName_, false);
}

std::pair<std::string, uint16_t> DataFetcherBase::getConnectTcp(unsigned int index)
{
	auto endpoint = net::endpoint::ReadConnectTCP(index, serverName_, false);
	return {endpoint->host, endpoint->port};
}

}
