#pragma once

#include <string>

#include "DataFetcherBase.h"

#include "acs_v2/Service.h"

namespace mdns
{

class DataFetcherNewACS: public DataFetcherBase
{
public:
	template <typename ...Args>
	DataFetcherNewACS(std::shared_ptr<acs::Service> service, Args&& ...args) :
		service_(std::move(service)),
		DataFetcherBase(std::forward<Args>(args)...)
		{}
	virtual void getListenerList(std::vector<boost::asio::ip::tcp::endpoint>& result) override;

private:
	std::shared_ptr<acs::Service> service_;
};

}
