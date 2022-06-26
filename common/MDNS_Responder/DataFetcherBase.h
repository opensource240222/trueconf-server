#pragma once
#include <string>
#include <vector>
#include <cstdlib>
#include <boost/asio/ip/tcp.hpp>

extern std::string g_tr_endpoint_name;

namespace mdns
{

class DataFetcherBase
{
public:
	std::string& getCompanyName();
	std::string& getServerName();
	std::string& getWebURL();

	DataFetcherBase(std::string companyName, std::string webURL);

	virtual void getListenerList(std::vector<boost::asio::ip::tcp::endpoint>& result) = 0;

	virtual unsigned int getConnectTcpCount();
	virtual std::pair<std::string, uint16_t> getConnectTcp(unsigned int index);

	virtual ~DataFetcherBase() {};
private:
	std::string organizationName_;
	std::string serverName_;
	std::string webURL_;
};

}
