#pragma once

#if defined(_WIN32) // not ported

#include "VS_TorrentStarterBase.h"

class VS_TorrentProxy;

class VS_TorrentStarter : public VS_TorrentStarterBase
{
public:
	VS_TorrentStarter(boost::asio::io_service& ios);
	~VS_TorrentStarter();

protected:
	void AddHandler_ACS(string_view name) override;
	void RemoveHandler_ACS(string_view name) override;
	vs::set<std::pair<std::string, net::port>> Listeners_ACS() override;
private:
	std::unique_ptr<VS_TorrentProxy> m_torrentProxy;
};

#endif //not ported