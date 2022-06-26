#include "Router.h"
#include "Endpoint.h"
#include "IEndpoint.h"
#include "IService.h"
#include "../Const.h"
#include "../Handshake.h"
#include "transport/VS_ServCertInfoInterface.h"
#include "SecureLib/VS_Sign.h"
#include "../../std/cpplib/VS_IntConv.h"
#include "../../std/cpplib/VS_Protocol.h"
#include "../../std/cpplib/VS_Utils.h"
#include "../../std/cpplib/event.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std-generic/cpplib/VS_Container_io.h"
#include "std-generic/cpplib/scope_exit.h"
#include "net/EndpointRegistry.h"
#include "std/debuglog/VS_Debug.h"
#include "newtransport/Router/ServiceV1Adaptor.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/variant/get.hpp>

#include <cinttypes>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

#define TRANSPORT_VERBOSE_LOGS 0

namespace transport
{

static std::atomic<uint64_t> g_last_request_id(0);

static const char c_request_name_prefix[] = "REQRESP:";
static const auto c_requests_cleanup_period = std::chrono::seconds(1); // This is also a maximal extra delay before requester is notified about expired request.

Router::Router(boost::asio::io_service& io_service, string_view endpoint_name)
	: m_strand(io_service)
	, m_endpoint_name(endpoint_name)
	, m_servers_cert_info(nullptr)
	, m_total_read_bytes(0)
	, m_total_write_bytes(0)
	, m_last_report_read_bytes(0)
	, m_last_report_write_bytes(0)
	, m_last_report_time(std::chrono::steady_clock::now())
	, m_requests_cleanup_timer(io_service)
	, m_funcIsRoamingAllowed([](const char *){return true; })
{
}

Router::~Router()
{
}

bool Router::AddService(const char * serviceName, VS_TransportRouterServiceBase * service, bool withOwnThread, const bool /*permittedForAll*/)
{
	return InstallV1Service(service, serviceName, withOwnThread, shared_from_this());
}

bool Router::RemoveService(const char * serviceName)
{
	if (!serviceName)
		return false;
	return RemoveService(string_view(serviceName));
}
void Router::Start(acs::Service* acs)
{
	m_handler = std::make_shared<Handler>(this);
	acs->AddHandler("TransportRouter Handler", m_handler);
	ScheduleRequestsCleanup();
}

void Router::Stop()
{
	m_handler = nullptr;

	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		m_requests_cleanup_timer.cancel();
		for (const auto& kv : m_endpoints)
			if (auto endpoint = kv.second.lock())
				endpoint->Close();
		m_endpoints.clear();
	});
	ready.wait();
}

bool Router::IsStarted()
{
	return m_handler != nullptr;
}

void Router::ProcessMessage(Message&& message)
{
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		ProcessMessage_impl(std::move(message));
	});
	ready.wait();
}

void Router::PostMessage(Message&& message)
{
	Message message_copy = std::move(message);
	m_strand.dispatch([this, message_copy]() mutable
	{
		ProcessMessage_impl(std::move(message_copy));
	});
}

void Router::ProcessMessage_impl(Message&& message)
{
	if (boost::starts_with(message.DstService_sv(), c_request_name_prefix) && m_endpoint_name == message.DstServer_sv())
	{
		const uint64_t id = std::strtoull(message.DstService() + sizeof(c_request_name_prefix)-1, nullptr, 10);
		// If conversion has failed then id==0 and we won't find it in the storage, because we start assigning ids from 1.

		auto& idx = m_requests.get<id_tag>();
		auto it = idx.find(id);
		if (it != idx.end())
		{
#if TRANSPORT_VERBOSE_LOGS
			dstream4 << "TR: Request(" << it->id << "): response arrived";
#endif
			idx.modify(it, [&](request_info& request) {
				auto p_cb = boost::get<response_cb>(&request.handler);
				if (p_cb)
				{
					response_cb cb = std::move(*p_cb);
					get_io_service().post([cb, message]() mutable { cb(std::move(message)); });
					return;
				}
				auto p_promise = boost::get<response_promise>(&request.handler);
				if (p_promise)
				{
					p_promise->set_value(std::move(message));
					return;
				}
			});
		}

		return;
	}

#if TRANSPORT_VERBOSE_LOGS
	{
		auto ds = dstream4;
		ds << "TR: Message: type=" << static_cast<int>(message.Type())
			<< ", SrcCID="     << message.SrcCID_sv()
			<< ", SrcService=" << message.SrcService_sv()
			<< ", SrcUser="    << message.SrcUser_sv()
			<< ", SrcServer="  << message.SrcServer_sv()
			<< ", DstCID="     << message.DstCID_sv()
			<< ", DstService=" << message.DstService_sv()
			<< ", DstUser="    << message.DstUser_sv()
			<< ", DstServer="  << message.DstServer_sv()
			<< ", AddString="  << message.AddString_sv()
			;
		VS_Container body_cnt;
		if (body_cnt.Deserialize(message.Body(), message.BodySize()))
			ds << ", Body:\n" << body_cnt;
		else
			ds << ", body_size="  << message.BodySize();
	}
#endif

			if (!message.SrcCID_sv().empty())
			{
				if (!message.DstService_sv().empty())
				{
					auto iter = m_services.find(message.DstService_sv());
					if (iter != m_services.end())
					{
						iter->second->ProcessMessage(std::move(message));
					}
				}
				else
				{
					auto endpoint = GetEndpoint(message.SrcCID_sv());
					if (endpoint)
					{
						endpoint->ProcessMessage(std::move(message));
					}
				}
			}
			else if (message.DstCID_sv().empty() &&
				((!message.SrcUser_sv().empty() && !message.DstServer_sv().empty()) ||
				(!message.SrcUser_sv().empty() && !message.DstUser_sv().empty() && message.DstServer_sv().empty()) ||
				(message.SrcUser_sv().empty() && !message.SrcServer_sv().empty() && !message.DstServer_sv().empty()) ||
				(message.SrcUser_sv().empty() && !message.SrcServer_sv().empty() && !message.DstUser_sv().empty() && message.DstServer_sv().empty())
				))
			{
				if (m_endpoint_name == message.DstServer_sv())
				{
					if (!message.DstUser_sv().empty()) {		// to our user
						auto endpoint = GetEndpointByUserId(message.DstUser_sv());
						if (endpoint)
							endpoint->SendToPeer(message);
					} else if (!message.DstService_sv().empty()) {	// to our service
						auto iter = m_services.find(message.DstService_sv());
						if (iter != m_services.end())
							iter->second->ProcessMessage(std::move(message));
					} else {
						// todo(kt): notify to SrcServer
						//auto endpoint = GetEndpoint(message.SrcServer_sv());
						//if (endpoint)
						//{
						//	endpoint->ProcessMessage(std::move(message));
						//}
					}
				}
				else
				{
					if (!message.DstServer_sv().empty()) {	// to remote server (or user at remote server)
						auto endpoint = GetEndpoint(message.DstServer_sv());
						if (endpoint)		// if already connected to remote server
							endpoint->SendToPeer(message);
						else {
							// create endpoint and start connect
							auto new_endpoint_name = message.DstServer_sv();
							if (IsRoamingAllowed(std::string(new_endpoint_name).c_str()))
							{
								net::endpoint::CreateFromID(new_endpoint_name, false);
								auto endpoint = std::make_shared<Endpoint>(get_io_service(), shared_from_this(), new_endpoint_name);
								m_endpoints.emplace(new_endpoint_name, endpoint);
								endpoint->SendToPeer(message);
								endpoint->StartConnection();
							}
						}
					} else {	// user at unknown server; need resolve
						auto endpoint = GetEndpointByUserId(message.DstUser_sv());
						if (endpoint)		// it is our user, but mess posted as to unknown user; so no need to resolve
							endpoint->SendToPeer(message);
						else
						{
							auto iter = m_services.find(string_view("RESOLVE"));
							if (iter != m_services.end())
							{
								// todo(kt): check that at Service thread, not Router thread
								iter->second->ProcessMessage(std::move(message));
							}
						}
					}
				}
			}
			else if (!message.DstCID_sv().empty())
			{
				auto endpoint = GetEndpoint(message.DstCID_sv());
				if (endpoint)
				{
					endpoint->SendToPeer(message);
				}
			}
}

bool Router::RequestResponse(Message&& message, response_cb&& cb, std::chrono::steady_clock::duration time_limit)
{
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		RequestResponse_impl(std::move(message), std::move(cb), time_limit);
	});
	ready.wait();
	return true;
}

auto Router::RequestResponse(Message&& message, std::chrono::steady_clock::duration time_limit) -> response_future
{
	response_promise p;
	response_future result(p.get_future());
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		RequestResponse_impl(std::move(message), std::move(p), time_limit);
	});
	ready.wait();
	return result;
};

void Router::RequestResponse_impl(Message&& message, response_handler_storage&& handler, std::chrono::steady_clock::duration time_limit)
{
	const uint64_t id = ++g_last_request_id;
	char name[sizeof(c_request_name_prefix)-1 + std::numeric_limits<uint64_t>::digits10 + 1 + 1/*\0*/];
	std::sprintf(name, "%s%" PRIu64, c_request_name_prefix, id);
	message = Message::Make()
		.Copy(message)
		.SrcService(name)
		.SrcServer(m_endpoint_name)
		;
	time_limit = std::max<decltype(time_limit)>(time_limit, std::chrono::milliseconds(message.TimeLimit()));
#if TRANSPORT_VERBOSE_LOGS
	dstream4 << "TR: Request(" << id << "): registered, dst_service=" << message.DstService_sv();
#endif
	bool inserted = m_requests.emplace(id, std::chrono::steady_clock::now() + time_limit, std::move(handler)).second;
	assert(inserted); // Insertion should never fail because we use atomic counter to allocate unique ids.
	ProcessMessage_impl(std::move(message));
}

bool Router::AddService(IService* service)
{
	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		result = m_services.emplace(service->GetName(), service).second;
	});
	ready.wait();
	return result;
}

bool Router::RemoveService(string_view service_name)
{
	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto it = m_services.find(service_name);
		if (it == m_services.end())
			return;
		m_services.erase(it);
		result = true;
	});
	ready.wait();
	return result;
}

bool Router::EndpointExists(string_view endpoint_name)
{
	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		result = GetEndpoint(endpoint_name) != nullptr;
	});
	ready.wait();
	return result;
}

bool Router::DisconnectEndpoint(string_view endpoint_name)
{
	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto endpoint = GetEndpoint(endpoint_name);
		if (!endpoint)
			return;
		endpoint->Close();
		result = true;
	});
	ready.wait();
	return result;
}

bool Router::FullDisconnectEndpoint(string_view endpoint_name)
{
	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto endpoint = GetEndpoint(endpoint_name);
		if (!endpoint)
			return;
		endpoint->SendDisconnect();
		endpoint->Shutdown();
		result = true;
	});
	ready.wait();
	return result;
}

	void Router::FullDisconnectAll()
	{
		m_strand.dispatch([this]()
		{
			for (auto endpoint_iter = m_endpoints.begin(); endpoint_iter != m_endpoints.end(); ++endpoint_iter)
			{
				auto ptr = endpoint_iter->second.lock();
				if (ptr)
				{
					ptr->SendDisconnect();
					ptr->Shutdown();
				}
			}
		});
	}

std::string Router::RegisterEndpoint(const std::shared_ptr<IEndpoint>& endpoint)
{
	std::string result;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		result = GetNewEndpointId();
		m_endpoints[result] = endpoint;

		for (auto& service_kv : m_services)
		{
			service_kv.second->OnEndpointConnect(endpoint->GetHops() == 0, EndpointConnectReason::incoming, result, endpoint->GetUserId());
			service_kv.second->OnEndpointIP(endpoint->GetUserId(), endpoint->GetRemoteIp());
		}
	});
	ready.wait();
	return result;
}

bool Router::AuthorizeClient(string_view client_id, string_view user_id)
{
	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto endpoint = GetEndpoint(client_id);
		if (!endpoint)
			return;
		endpoint->Authorize(user_id);
		result = true;
	});
	ready.wait();
	return result;
}

bool Router::UnauthorizeClient(string_view client_id)
{
	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto endpoint = GetEndpoint(client_id);
		if (!endpoint && !(endpoint = GetEndpointByUserId(client_id)))
			return;
		endpoint->Unauthorize();
		result = true;
	});
	ready.wait();
	return result;
}

bool Router::IsAuthorized(string_view client_id)
{
	bool result = false;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto endpoint = GetEndpointByUserId(client_id);
		if (!endpoint && !(endpoint = GetEndpoint(client_id)))
			return;
		result = endpoint->IsAuthorized();
	});
	ready.wait();
	return result;
}

std::string Router::GetIpByClientId(string_view client_id)
{
	std::string result;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto endpoint = GetEndpoint(client_id);
		if (!endpoint)
			return;
		result = endpoint->GetRemoteIp();
	});
	ready.wait();
	return result;
}

std::string Router::GetClientIdByUserId(string_view user_id)
{
	std::string result;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		auto endpoint = GetEndpointByUserId(user_id);
		if (!endpoint)
			return;
		result = std::string(endpoint->GetId());
	});
	ready.wait();
	return result;
}

	std::string Router::GetNewEndpointId()
	{
		char cid[512] = { 0 };
		std::string id;
		while (id.empty() || GetEndpoint(id))
		{
			VS_GenKeyByMD5(cid);
			id = cid;
		}
		return id;
	}

	void Router::SetConnection(const char* cid, const uint32_t version, const char* sid, boost::asio::ip::tcp::socket&& sock, const bool is_accept, const uint16_t /*max_conn_silence_ms*/, const uint8_t /*fatal_silence_coef*/, const uint8_t hops, const void* rnd_data, const size_t rnd_data_ln, const void* sign, const size_t sign_ln, const bool /*hs_error = false*/, const bool tcp_keep_alive /*= false*/)
	{
		auto result = HandshakeResult::ok;
		vs::event ready(true);
		m_strand.dispatch([&]() {
			VS_SCOPE_EXIT { ready.set(); };
			using pk_res = VS_ServCertInfoInterface::get_info_res;
			bool verify = true;
			auto get_pk_res = pk_res::undef;
			unsigned secure_hs_version = 1;
			uint32_t min_ver = 361;
			uint32_t cur_ver = atou_s(VCS_SERVER_VERSION);
			if (hops && m_servers_cert_info)
			{
				verify = false;
				VS_SimpleStr	pubKey;
				VS_Sign			verifier;
				VS_SignArg		signarg = { alg_pk_RSA, alg_hsh_SHA1 };
				uint32_t		srv_ver(0);

				if (verifier.Init(signarg)
					&& pk_res::ok
					== (get_pk_res = m_servers_cert_info->GetPublicKey(
						cid, pubKey, srv_ver))
					&& verifier.SetPublicKey(pubKey, pubKey.Length(),
						store_PEM_BUF)
					&& verifier.VerifySign(
						static_cast<const unsigned char*>(rnd_data),
						rnd_data_ln,
						static_cast<const unsigned char*>(sign), sign_ln))
				{
					verify = true;
				}
				else if (pk_res::auto_verify == get_pk_res)
					verify = true;
				else
					dstream4 << "got PubKey = " << pubKey.m_str
					<< "; cid:" << cid << "; res=" << static_cast<int32_t>(get_pk_res)
					<< "; CERT OR SIGN IS NOT CORRECT!!\n";
				dprint1("srv_ver=%d, min_ver=%d\n", srv_ver, min_ver);
				secure_hs_version = (srv_ver > min_ver) ? 2 : 1;
			}
			else if (hops&&min_ver < cur_ver)
				secure_hs_version = 2;
			if ((version & ~c_ssl_support_mask) <= c_version_old)
				result = HandshakeResult::oldarch;
			else if ((version & ~c_ssl_support_mask) < c_version_min)
				result = HandshakeResult::antique_you;
			else if (pk_res::auto_deny == get_pk_res
				|| (!verify
					&& ((pk_res::ok == get_pk_res)
						|| ((pk_res::key_is_absent == get_pk_res)
							&& (rnd_data_ln != 0 || sign_ln != 0)))))
			{
				result = HandshakeResult::verification_failed;
			}
			else if (!verify
				&& (pk_res::db_error == get_pk_res))
			{
				result = HandshakeResult::busy;
			}
			else if (!hops && m_srv_cert.empty())
			{
				result = HandshakeResult::busy;
			}
			else if (hops && sid && *sid && m_endpoint_name != sid)
			{
				result = HandshakeResult::alien_server;
			}
			else
			{
				if (auto endpoint = GetEndpoint(cid))
					endpoint->Close();

				if ((hops == 0 && verify && (!cid || *cid == '\0')) // client
					|| IsRoamingAllowed(cid))
				{
					const std::string endpoint_id = (hops && cid && *cid) ? cid : GetNewEndpointId();
					auto endpoint = std::make_shared<Endpoint>(get_io_service(), shared_from_this(), endpoint_id);
					m_endpoints[endpoint_id] = endpoint;
					endpoint->SetConnection(std::move(sock), version, secure_hs_version, hops, tcp_keep_alive);
					if (hops && verify)	// authorize server only, not client
					{
						endpoint->Authorize(cid);
					}
					else
					{
						endpoint->Unauthorize();
					}

					for (auto& service_kv : m_services)
					{
						service_kv.second->OnEndpointConnect(endpoint->GetHops() == 0, is_accept ? EndpointConnectReason::incoming : EndpointConnectReason::requested, cid, endpoint->GetUserId());
						service_kv.second->OnEndpointIP(endpoint->GetUserId(), endpoint->GetRemoteIp());
					}
				}
				else
				{
					result = HandshakeResult::busy;
				}
			}
		});
		ready.wait();
	}

void Router::RemoveEndpoint(std::shared_ptr<IEndpoint> endpoint)
{
	m_strand.dispatch([this, self = shared_from_this(), endpoint = std::move(endpoint)]() {
		auto it = m_endpoints.find(endpoint->GetId());
		if (it == m_endpoints.end())
			return;
		assert(endpoint == it->second.lock());
		m_endpoints.erase(it);
		for (auto& service_kv : m_services)
			service_kv.second->OnEndpointDisconnect(endpoint->GetHops() == 0, EndpointConnectReason::unknown, endpoint->GetId(), endpoint->GetUserId());
	});
}

std::shared_ptr<IEndpoint> Router::GetEndpoint(string_view endpoint_name)
{
	assert(m_strand.running_in_this_thread());
	auto it = m_endpoints.find(endpoint_name);
	return it != m_endpoints.end() ? it->second.lock() : nullptr;
}

std::shared_ptr<IEndpoint> Router::GetEndpointByUserId(string_view user_id)
{
	assert(m_strand.running_in_this_thread());
	for (const auto& kv : m_endpoints)
	{
		auto endpoint = kv.second.lock();
		if (endpoint && endpoint->GetUserId() == user_id)
		{
			assert(endpoint->IsAuthorized());
			return endpoint;
		}
	}
	return nullptr;
}

	void Router::TEST_AddEndpoint(std::shared_ptr<IEndpoint> endpoint)
	{
		m_endpoints[std::string(endpoint->GetId())] = endpoint;
	}

uint32_t Router::GetEndpointCount()
{
	uint32_t result = 0;
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		result = m_endpoints.size();
	});
	ready.wait();
	return result;
}

void Router::GetStatistics(uint64_t& total_read_bytes, uint64_t& total_write_bytes, float& read_byterate, float& write_byterate)
{
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT { ready.set(); };
		const auto now = std::chrono::steady_clock::now();
		const auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(now - m_last_report_time);

		total_read_bytes = m_total_read_bytes.load();
		total_write_bytes = m_total_write_bytes.load();
		read_byterate = static_cast<float>(total_read_bytes - m_last_report_read_bytes) / elapsed.count();
		write_byterate = static_cast<float>(total_write_bytes - m_last_report_write_bytes) / elapsed.count();

		m_last_report_read_bytes = total_read_bytes;
		m_last_report_write_bytes = total_write_bytes;
		m_last_report_time = now;
	});
	ready.wait();
}

	const std::string& Router::EndpointName() const
	{
		return m_endpoint_name;
	}

	void Router::GetMonitorInfo(Monitor::TmReply& reply)
	{
		vs::event ready(true);
		m_strand.dispatch([&]() {
			VS_SCOPE_EXIT { ready.set(); };
			for (auto endpoint_iter = m_endpoints.begin(); endpoint_iter != m_endpoints.end(); ++endpoint_iter)
			{
				Monitor::TmReply::Endpoint endpoint;
				auto ptr = endpoint_iter->second.lock();
				if (ptr)
				{
					ptr->FillMonitorStruct(endpoint);
				}
				reply.endpoints.push_back(endpoint);
			}
			for (auto service_iter = m_services.begin(); service_iter != m_services.end(); ++service_iter)
			{
				Monitor::TmReply::Service service;
				service_iter->second->FillMonitorStruct(service);
				reply.services.push_back(service);
			}
		});
		ready.wait();
	}

	void Router::AddMonitorEndpoint(boost::asio::ip::tcp::socket&& /*socket*/, Monitor::TmRequest /*request*/)
	{

	}

void Router::ScheduleRequestsCleanup()
{
	m_requests_cleanup_timer.expires_from_now(c_requests_cleanup_period);
	m_requests_cleanup_timer.async_wait(m_strand.wrap(
		[this, self = shared_from_this()](const boost::system::error_code& ec)
		{
			if (ec == boost::asio::error::operation_aborted)
				return;

			const auto now = std::chrono::steady_clock::now();
			auto& idx = m_requests.get<expiration_tag>();
			auto it = idx.begin();
			while (it != idx.end())
			{
				if (it->expire_time > now)
					break;

#if TRANSPORT_VERBOSE_LOGS
				dstream4 << "TR: Request(" << it->id << "): expired";
#endif
				idx.modify(it, [&](request_info& request) {
					auto p_cb = boost::get<response_cb>(&request.handler);
					if (p_cb && (*p_cb) != nullptr)
					{
						response_cb cb = std::move(*p_cb);
						get_io_service().post([cb]() { cb(Message()); });
						return;
					}
					auto p_promise = boost::get<response_promise>(&request.handler);
					if (p_promise)
					{
						p_promise->set_value(Message());
						return;
					}
				});
				it = idx.erase(it);
			}
			idx.erase(idx.begin(), it);

			ScheduleRequestsCleanup();
		}
	));
}

void Router::SetIsRoamingAllowedFunc(std::function<bool(const char*)> f)
{
	if (!!f)
	{
		m_strand.dispatch([this, f = std::move(f)]() mutable {
			m_funcIsRoamingAllowed = std::move(f);
		});
	}
}

bool Router::IsRoamingAllowed(const char* sid)
{
	return m_funcIsRoamingAllowed(sid);
}

}
