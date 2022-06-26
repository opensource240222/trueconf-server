#pragma once

#include <string>
#include <functional>

#include "std-generic/cpplib/string_view.h"
#include "std-generic/compat/set.h"
#include "std-generic/asio_fwd.h"

#include "http/handlers/TorrentAnnounce.h"

namespace transport { struct IRouter; }

class VS_TransportRouter;
class VS_TRStorageInterface;
class VS_TorrentService;

class VS_TorrentStarterBase : public http::handlers::GetTorrentServiceInterface
{
public:
	virtual ~VS_TorrentStarterBase() {}

	bool IsStarted() const noexcept
	{
		return bool(m_service);
	}

	template<class Callback>
	void SetGetServerID(Callback&& getServerCb) noexcept { m_getServerId = std::forward<Callback>(getServerCb); }

	bool Start(transport::IRouter *tr, const std::shared_ptr<VS_TRStorageInterface> &dbStorage);
	void Stop(transport::IRouter *tr);

	std::shared_ptr<http::handlers::Interface> AsHandler() override;

	static std::string FileStorageDir(bool create = false);

protected:
	explicit VS_TorrentStarterBase(boost::asio::io_service &ios) : m_ios(ios) {}

	const std::shared_ptr<VS_TorrentService> &Service() const noexcept { return m_service; }
	virtual void AddHandler_ACS(string_view name) = 0;
	virtual void RemoveHandler_ACS(string_view name) = 0;
	//first - ip addr or dns name:
	//		  dns
	//		  ipv4
	//		  [ipv6]
	//second - port
	virtual vs::set<std::pair<std::string, net::port>> Listeners_ACS() = 0;
private:
	static void STARTMESS(const char *service);

private:
	boost::asio::io_service &m_ios;
	std::shared_ptr<VS_TorrentService> m_service;
	std::function<std::string(const char *)> m_getServerId;
};