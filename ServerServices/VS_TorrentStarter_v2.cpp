#include "VS_TorrentStarter_v2.h"

#include <cassert>

#include "acs_v2/Service.h"
#include "acs_v2/Handler.h"

#include "VS_TorrentService.h"

VS_TorrentStarter_v2::~VS_TorrentStarter_v2()
{
}

void VS_TorrentStarter_v2::AddHandler_ACS(string_view name)
{
	assert(!name.empty());

	auto tmp_acs = m_acsSrv.lock();
	if(tmp_acs)
		tmp_acs->AddHandler(name, std::static_pointer_cast<acs::Handler>(Service()));
}

void VS_TorrentStarter_v2::RemoveHandler_ACS(string_view name)
{
	assert(!name.empty());

	assert(Service() == nullptr);

	auto tmp_acs = m_acsSrv.lock();
	if (tmp_acs)
		tmp_acs->RemoveHandler(name);
}

std::set<std::pair<std::string, net::port>> VS_TorrentStarter_v2::Listeners_ACS()
{
	std::vector<std::pair<net::address, net::port>> addrs;

	auto tmp_acs = m_acsSrv.lock();
	if (tmp_acs)
		tmp_acs->GetListenerList(addrs, net::protocol::TCP | net::protocol::UDP);

	vs::set<std::pair<std::string, net::port>> res;
	for(auto &&item : addrs)
	{
		res.emplace(item.first.to_string(), item.second);
	}

	return res;
}
