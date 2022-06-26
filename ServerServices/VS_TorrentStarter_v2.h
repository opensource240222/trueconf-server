#pragma once

#include "VS_TorrentStarterBase.h"

namespace acs { class Service; }

class VS_TorrentStarter_v2 : public VS_TorrentStarterBase
{
public:
	explicit VS_TorrentStarter_v2(boost::asio::io_service &ios, const std::shared_ptr<acs::Service> &acsSrv) : VS_TorrentStarterBase(ios), m_acsSrv(acsSrv) {}
	~VS_TorrentStarter_v2();
protected:
	void AddHandler_ACS(string_view name) override;
	void RemoveHandler_ACS(string_view name) override;
	vs::set<std::pair<std::string, net::port>> Listeners_ACS() override;
private:
	std::weak_ptr<acs::Service> m_acsSrv;
};
