#include "DataFetcherNewACS.h"

namespace mdns
{


void DataFetcherNewACS::getListenerList(std::vector<boost::asio::ip::tcp::endpoint>& result)
{
	acs::Service::address_list listenerList;
	service_->GetListenerList(listenerList, net::protocol::TCP);
	for (const auto& address: listenerList)
		result.emplace_back(address.first, address.second);
}

}
