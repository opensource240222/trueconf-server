#ifdef _WIN32
#include "DataFetcherOldACS.h"

#include <boost/asio/ip/tcp.hpp>

namespace mdns
{

namespace
{

inline uint16_t getPort(const std::string& port)
{
	return static_cast<uint16_t>(std::atoi(port.c_str()));
}

}

void DataFetcherOldACS::addHost(const std::string& host, const std::string& port,
	std::vector<boost::asio::ip::tcp::endpoint>& result)
{
	boost::system::error_code ec;
	boost::asio::ip::address possibleIp = boost::asio::ip::address::from_string(host, ec);
	if (!ec)
	{
		result.emplace_back(possibleIp, getPort(port));
		return;
	}
//	Did we meet a hostname?
	auto possibleIpIterator = resolver_.resolve({host, port}, ec);
	if (!ec)
	{
		boost::asio::ip::tcp::resolver::iterator end;
		while (possibleIpIterator != end)
		{
			result.emplace_back(possibleIpIterator->endpoint());
			++possibleIpIterator;
		}
		return;
	}
}

void DataFetcherOldACS::getListenerList(std::vector<boost::asio::ip::tcp::endpoint>& result)
{
	std::string listenerString;
	unsigned int listenerCount = connectionSystem_->GetListeners(listenerString);
	size_t pointer = 0;
	for (unsigned int i = 0; i < listenerCount; ++i)
	{
		if (listenerString[pointer] == '[')// IPV6
		{
			size_t addressEnd = listenerString.find(']', pointer);
			size_t portEnd = listenerString.find(',', addressEnd);
			if (portEnd == std::string::npos)
				portEnd = listenerString.size();
			addHost(
				listenerString.substr(pointer + 1, addressEnd - pointer - 1),
				listenerString.substr(addressEnd + 2, portEnd - addressEnd - 2),
				result);
			pointer = portEnd + 1;
		} else //IPV4
		{
			size_t addressEnd = listenerString.find(':', pointer);
			size_t portEnd = listenerString.find(',', addressEnd);
			if (portEnd == std::string::npos)
				portEnd = listenerString.size();
			addHost(
				listenerString.substr(pointer, addressEnd - pointer),
				listenerString.substr(addressEnd + 1, portEnd - addressEnd - 1),
				result);
			pointer = portEnd + 1;
		}
	}
}

}

#endif
