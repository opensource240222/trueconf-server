#pragma once

#include "http/handlers/Interface.h"
#include "net/Address.h"
#include "net/Port.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/weak_ptr.hpp>

namespace http {
namespace handlers{

struct GetTorrentServiceInterface
{
	virtual ~GetTorrentServiceInterface() {}
	virtual std::shared_ptr<http::handlers::Interface> AsHandler() = 0;
};

class TorrentAnnounce : public http::handlers::Interface
{
	std::weak_ptr<GetTorrentServiceInterface> m_torrent_starter;
public:
	TorrentAnnounce(const std::weak_ptr<GetTorrentServiceInterface>& starter) : m_torrent_starter(starter)
	{}
	boost::optional<std::string> HandleRequest(string_view in, const net::address& from_ip, const net::port from_port) override
	{
		auto strong = m_torrent_starter.lock();
		if (!strong)
			return boost::none;
		auto torrent_service = strong->AsHandler();
		return (torrent_service) ? torrent_service->HandleRequest(in, from_ip, from_port) : boost::none;
	}
};

} // handlers
} // http

