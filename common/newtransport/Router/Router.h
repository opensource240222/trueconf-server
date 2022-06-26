#pragma once

#include "Monitor.h"
#include "TransportHandler.h"
#include "transport/Message.h"
#include "acs_v2/Service.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std-generic/cpplib/macro_utils.h"
#include "transport/Router/TransportRouterInterface.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/variant/variant.hpp>

#include "std-generic/compat/functional.h"
#include "std-generic/compat/map.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <string>

#include "std-generic/undef_windows.h" // this should be last

class VS_ServCertInfoInterface;

namespace { namespace mi = boost::multi_index; }

namespace transport
{
	class IEndpoint;
	class IService;
	class Message;

	class Router
		:	public std::enable_shared_from_this<Router>
		,	public TransportRouter_SetConnection
		,	public transport::IRouter
	{
	public:
		using response_cb = std::function<void (Message&&)>;
		using response_promise = std::promise<Message>;
		using response_future = std::future<Message>;

	private:
		using response_handler_storage = boost::variant<response_cb, response_promise>;

	public:
		~Router();

		boost::asio::io_service& get_io_service()
		{
			return m_strand.get_io_service();
		}

		bool	AddService(const char *serviceName,
			VS_TransportRouterServiceBase *service, bool withOwnThread = true,
			const bool permittedForAll = false) override;
		bool	RemoveService(const char *serviceName) override;

		void Start(acs::Service* acs);
		void Stop();
		bool IsStarted();

		void ProcessMessage(Message&& message);
		void PostMessage(Message&& message);
		bool RequestResponse(Message&& message, response_cb&& cb, std::chrono::steady_clock::duration time_limit);
		response_future RequestResponse(Message&& message, std::chrono::steady_clock::duration time_limit);

		bool AddService(IService* service);
		bool RemoveService(string_view service_name);

		bool EndpointExists(string_view endpoint_name);
		void RemoveEndpoint(std::shared_ptr<IEndpoint> endpoint);
		bool DisconnectEndpoint(string_view endpoint_name);
		bool FullDisconnectEndpoint(string_view endpoint_name);
		void FullDisconnectAll();
		std::string RegisterEndpoint(const std::shared_ptr<IEndpoint>& endpoint);

		bool AuthorizeClient(string_view client_id, string_view user_id);
		bool UnauthorizeClient(string_view client_id);
		bool IsAuthorized(string_view client_id);
		std::string GetIpByClientId(string_view client_id);
		std::string GetClientIdByUserId(string_view user_id);
		uint32_t GetEndpointCount();
		void GetStatistics(uint64_t& total_read_bytes, uint64_t& total_write_bytes, float& read_byterate, float& write_byterate);
		const std::string& EndpointName() const override;

		void SetConnection(const char *cid,
			const uint32_t version,
			const char *sid,
			boost::asio::ip::tcp::socket&& sock, const bool is_accept,
			const uint16_t max_conn_silence_ms,
			const uint8_t fatal_silence_coef,
			const uint8_t hops,
			const void* rnd_data,
			const size_t rnd_data_ln,
			const void* sign,
			const size_t sign_ln,
			const bool hs_error = false,
			const bool tcp_keep_alive = false) override;


		void GetMonitorInfo(Monitor::TmReply& reply);
		void AddMonitorEndpoint(boost::asio::ip::tcp::socket&& socket, Monitor::TmRequest request);

		VS_PKey& SrvPrivateKey() { return m_srv_private_key; }
		VS_Container& SrvCertChain() { return m_srv_cert_chain; }
		void SetSrvCert(const std::string& srv_cert) { m_srv_cert = srv_cert; }

		// Used by endpoints to allow Router to calculate IO bitrate
		void NotifyRead(size_t bytes)
		{
			m_total_read_bytes += bytes;
		}
		void NotifyWrite(size_t bytes)
		{
			m_total_write_bytes += bytes;
		}

		void SetIsRoamingAllowedFunc(std::function<bool(const char*)> f);
		bool IsRoamingAllowed(const char* sid);

		void TEST_AddEndpoint(std::shared_ptr<IEndpoint> endpoint); // Used by unit tests
	
	protected:
		Router(boost::asio::io_service &io_service, string_view endpoint_name);

	private:
		std::shared_ptr<IEndpoint> GetEndpoint(string_view endpoint_name);
		std::shared_ptr<IEndpoint> GetEndpointByUserId(string_view user_id);
		std::string GetNewEndpointId();
		void ProcessMessage_impl(Message&& message);
		void RequestResponse_impl(Message&& message, response_handler_storage&& handler, std::chrono::steady_clock::duration time_limit);

		void ScheduleRequestsCleanup();

		boost::asio::io_service::strand m_strand;
		std::string m_endpoint_name;
		uint32_t m_hops;
		VS_PKey m_srv_private_key;
		VS_Container m_srv_cert_chain;
		std::string m_srv_cert;
		vs::map<std::string, std::weak_ptr<IEndpoint>, vs::str_less> m_endpoints;
		vs::map<std::string, IService*, vs::str_less> m_services;
		VS_ServCertInfoInterface* m_servers_cert_info;
		std::shared_ptr<Handler> m_handler;

		std::atomic<uint64_t> m_total_read_bytes;
		std::atomic<uint64_t> m_total_write_bytes;
		uint64_t m_last_report_read_bytes;
		uint64_t m_last_report_write_bytes;
		std::chrono::steady_clock::time_point m_last_report_time;
		std::function<bool(const char*)> m_funcIsRoamingAllowed;

		struct request_info
		{
			uint64_t id;
			std::chrono::steady_clock::time_point expire_time;
			response_handler_storage handler;
			VS_FORWARDING_CTOR3(request_info, id, expire_time, handler) {}
		};
		typedef mi::indexed_by<
			mi::ordered_unique<mi::tag<struct id_tag>, mi::member<request_info, decltype(request_info::id), &request_info::id>>,
			mi::ordered_non_unique<mi::tag<struct expiration_tag>, mi::member<request_info, decltype(request_info::expire_time), &request_info::expire_time>>
		> requests_indices;
		boost::multi_index_container<request_info, requests_indices> m_requests;
		boost::asio::steady_timer m_requests_cleanup_timer;
	};
}
