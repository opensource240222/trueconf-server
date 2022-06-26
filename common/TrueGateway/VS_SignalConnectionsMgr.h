#pragma once

#include "TrueGateway/VS_GatewayService.h"
#include "acs_v2/Responses.h"
#include "net/Address.h"
#include "net/Port.h"
#include "net/UDPRouter.h"
#include "statuslib/status_types.h"
#include "statuslib/VS_ExternalPresenceInterface.h"
#include "std/cpplib/event.h"

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <chrono>
#include <functional>
#include <map>
#include <set>
#include <vector>

class VS_TransportConnection;
class VS_CallConfigStorage;
class VS_Policy;
struct VS_ExternalAccount;
class VS_UserData;
namespace net {
	class LoggerInterface;
} //namespace net
namespace ts { struct IPool; }
namespace acs { class Service; }
class VS_SignalConnectionHandler;

class VS_SignalConnectionsMgr
	: public VS_ExternalPresenceInterface
	, public VS_GatewayService::AbstractGatewayEventListener
{
private:
	struct Address final
	{
		net::address addr;
		net::port port;
		friend bool operator<(const Address& lhs, const Address& rhs)
		{
			return std::tie(lhs.addr, lhs.port) < std::tie(rhs.addr, rhs.port);
		}
	};

public:
	using post_mes_t = std::function<bool(VS_RouterMessage*)>;
	struct InitInfo
	{
		explicit InitInfo(boost::asio::io_service::strand& strand)
			: strand(strand)
			, ourEndpoint(nullptr)
			, ourService(nullptr)
		{
		}
		boost::asio::io_service::strand& strand;
		std::function<bool(const std::string&, const std::string&)> checkDigest;
		UserStatusFunction getUserStatus;
		const char *ourEndpoint;
		const char *ourService;
		post_mes_t postMes;
		std::shared_ptr<VS_CallConfigStorage> peerConfig;
		std::weak_ptr<ts::IPool>	transcPool;
		std::shared_ptr<net::LoggerInterface> logger;
	};

	~VS_SignalConnectionsMgr();

	bool Start(acs::Service* acs);

	void Close();
	void WaitForShutdown();

	// Called by VS_SignalConnectionHandler
	acs::Response Protocol(const void* data, size_t size);
	void Accept(boost::asio::ip::tcp::socket&& socket, const void* data, size_t size);
	void Accept(net::UDPConnection&& connection, const void* data, size_t size);

	void NewPeerCfg(string_view call_id, const std::vector<VS_ExternalAccount>& v);

protected:
	explicit VS_SignalConnectionsMgr(InitInfo&& info);
	static void PostConstruct(std::shared_ptr<VS_SignalConnectionsMgr>&) { /*stub*/ }

protected:
	// VS_GatewayService::AbstractGatewayEventListener
	using AbstractGatewayEventListener::Start;
	void HandleEvent(const VS_GatewayService::ReloadConfiguration& reloadCfg) override;
	void SetRegistrationConfiguration(VS_CallConfig config) override;
	void UpdateStatusRegistration(const net::address& address, net::port, std::function<void(const std::shared_ptr<VS_ParserInterface>&)>&& exec) override;
	void ResetAllConfigsStatus() override;
	const char* GetPeerName() override;

	// VS_ExternalPresenceInterface
	bool Resolve(std::string& call_id, VS_CallIDInfo& ci, VS_UserData* from_user) override;
	void Subscribe(const char *call_id) override;
	void Unsubscribe(const char *call_id) override;
	bool IsMyCallId(const char *call_id) const override;
	bool CanICall(VS_UserData* from_ude, const char* to_call_id, bool IsVCS) override;

	std::shared_ptr<VS_TransportConnection> GetTransportConnectionByAddress(const Address& a);

private:
	template <class Socket>
	void Accept(Socket&& socket, const void* data, size_t size, const char* proto_name);
	void ScheduleTimer();
	void PushAllSubscribtionsInvalid();
	std::shared_ptr<VS_TransportConnection> CreateTransportConnection(std::shared_ptr<VS_ParserInterface> parser);
	std::string ReserveOut(const std::string& to_sip_id, VS_UserData* from_user); // return TranscoderID

private:
	boost::asio::io_service::strand m_strand;
	boost::asio::steady_timer m_timer;
	std::atomic<bool> m_should_run;
	vs::event m_stopped;

	std::multimap<Address, std::shared_ptr<VS_TransportConnection>> m_connections;
	std::set<std::string> m_subscribes;
	std::shared_ptr<VS_SignalConnectionHandler> m_handler;
	boost::shared_ptr<VS_Policy> m_policy;

	std::function<bool(const std::string&, const std::string&)> m_checkDigest;
	UserStatusFunction m_getUserStatus;
	const char* m_ourEndpoint;
	const char* m_ourService;
	post_mes_t m_postMes;
	std::weak_ptr<ts::IPool> m_transcPool;
	std::shared_ptr<net::LoggerInterface> m_logger;
};
