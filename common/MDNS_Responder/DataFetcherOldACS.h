#pragma once
#include "DataFetcherBase.h"
#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"

namespace mdns
{

class DataFetcherOldACS: public DataFetcherBase
{
public:
	template<typename ...Args>
	DataFetcherOldACS(VS_AccessConnectionSystem* connectionSystem, Args&& ...args) :
		ios_(),
		resolver_(ios_),
		connectionSystem_(connectionSystem),
		DataFetcherBase(std::forward<Args>(args)...)
		{}
	virtual void getListenerList(std::vector<boost::asio::ip::tcp::endpoint>& result) override;

private:
	void addHost(const std::string& host, const std::string& port,
		std::vector<boost::asio::ip::tcp::endpoint>& result);
	boost::asio::io_service ios_;
	boost::asio::ip::tcp::resolver resolver_;
	VS_AccessConnectionSystem* connectionSystem_;
};

}
