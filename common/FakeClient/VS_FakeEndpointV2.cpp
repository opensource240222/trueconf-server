#include "FakeClient/VS_FakeEndpointV2.h"
#include "newtransport/Const.h"
#include "newtransport/Router/EndpointBase.h"
#include "newtransport/Router/Router.h"
#include "std/cpplib/MakeShared.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>

#include "std-generic/compat/memory.h"

#define FAKECLIENT_VERBOSE_LOGS 0

class VS_FakeEndpointV2::RouterEndpoint
	: public transport::EndpointBase
	, public std::enable_shared_from_this<RouterEndpoint>
{
protected:
	explicit RouterEndpoint(const std::shared_ptr<transport::Router>& router);
	static void PostConstruct(std::shared_ptr<RouterEndpoint>& p)
	{
		p->ScheduleTimer(p);
		p->m_id = p->m_router.lock()->RegisterEndpoint(p);
	}

public:

	void Stop();

	void Close() final;
	void Shutdown() final;

	void ProcessMessage(const transport::Message& message) final;
	void SendToPeer(const transport::Message& message) final;
	void SendPing() final;
	void SendDisconnect() final;

	std::string GetRemoteIp() final;

	void FillMonitorStruct(transport::Monitor::TmReply::Endpoint& tmreply_endpoint) final;

private:
	void ScheduleTimer(std::weak_ptr<RouterEndpoint> self_weak);

private:
	friend VS_FakeEndpointV2;

	std::weak_ptr<Receiver> m_receiver;
	boost::asio::io_service::strand m_strand;
	boost::asio::steady_timer m_timer;
};

VS_FakeEndpointV2::RouterEndpoint::RouterEndpoint(const std::shared_ptr<transport::Router>& router)
	: EndpointBase(router, {})
	, m_strand(router->get_io_service())
	, m_timer(router->get_io_service())
{
}

void VS_FakeEndpointV2::RouterEndpoint::Stop()
{
	m_strand.dispatch([self = shared_from_this()]() {
		self->m_timer.cancel();
	});
	auto r = m_router.lock();
	if (r)
		r->RemoveEndpoint(shared_from_this());
}

void VS_FakeEndpointV2::RouterEndpoint::Close()
{
	m_strand.dispatch([self = shared_from_this()]() {
		if (const auto receiver = self->m_receiver.lock())
			receiver->OnError(-1);
		self->m_timer.cancel();
	});
	auto r = m_router.lock();
	if (r)
		r->RemoveEndpoint(shared_from_this());
}

void VS_FakeEndpointV2::RouterEndpoint::Shutdown()
{
	Close();
}

void VS_FakeEndpointV2::RouterEndpoint::ProcessMessage(const transport::Message& message)
{
	auto message_type = message.AddString_sv();
	if (message_type.size() == 1 && message_type[0] == transport::c_disconnect_opcode)
		Close();
}

void VS_FakeEndpointV2::RouterEndpoint::SendToPeer(const transport::Message& message)
{
	m_strand.post([self = shared_from_this(), message = std::move(message)]() {
		if (const auto receiver = self->m_receiver.lock())
			receiver->OnReceive(message);
	});
}

void VS_FakeEndpointV2::RouterEndpoint::SendPing()
{
	// Nothing to do
}

void VS_FakeEndpointV2::RouterEndpoint::SendDisconnect()
{
	// Nothing to do
}

std::string VS_FakeEndpointV2::RouterEndpoint::GetRemoteIp()
{
	return "127.0.0.1";
}

void VS_FakeEndpointV2::RouterEndpoint::FillMonitorStruct(transport::Monitor::TmReply::Endpoint& /*tmreply_endpoint*/)
{
	// TODO
}

void VS_FakeEndpointV2::RouterEndpoint::ScheduleTimer(std::weak_ptr<RouterEndpoint> self_weak)
{
	m_timer.expires_from_now(std::chrono::milliseconds(500));
	m_timer.async_wait(m_strand.wrap([self_weak = std::move(self_weak)](const boost::system::error_code& ec) {
		auto self = self_weak.lock();
		if (!self)
			return;
		if (ec == boost::asio::error::operation_aborted)
			return;

		if (const auto receiver = self->m_receiver.lock())
			receiver->Timeout();
		self->ScheduleTimer(self_weak);
	}));
}

VS_FakeEndpointV2::VS_FakeEndpointV2(const std::shared_ptr<transport::Router>& router)
	: m_ep(vs::MakeShared<RouterEndpoint>(router))
{
#if FAKECLIENT_VERBOSE_LOGS
	dstream4 << "VS_FakeEndpointV2(" << m_ep->m_id << "): new";
#endif
}

VS_FakeEndpointV2::~VS_FakeEndpointV2()
{
	Stop();
}

void VS_FakeEndpointV2::SetReceiver(std::weak_ptr<Receiver> receiver)
{
	m_ep->m_receiver = std::move(receiver);
}

void VS_FakeEndpointV2::Stop()
{
	m_ep->Stop();
#if FAKECLIENT_VERBOSE_LOGS
	dstream4 << "VS_FakeEndpointV2(" << m_ep->m_id << "): stopped";
#endif
}

const std::string& VS_FakeEndpointV2::CID() const
{
	return m_ep->m_id;
}

bool VS_FakeEndpointV2::Send(transport::Message&& message)
{
	m_ep->PreprocessMessage(message);
	auto r = m_ep->m_router.lock();
	if (r)
		r->ProcessMessage(std::move(message));
	return true;
}

void VS_FakeEndpointFactory::InitV2(const std::shared_ptr<transport::Router>& router)
{
	if (s_instance)
		return;
	s_instance = vs::make_unique<VS_FakeEndpointFactoryV2>(router);
}

VS_FakeEndpointFactoryV2::VS_FakeEndpointFactoryV2(const std::shared_ptr<transport::Router>& router)
	: m_router(router)
{
}

void VS_FakeEndpointFactoryV2::Stop()
{
}

std::unique_ptr<VS_FakeEndpoint> VS_FakeEndpointFactoryV2::Create()
{
	auto r = m_router.lock();
	if (!r)
		return nullptr;
	return vs::make_unique<VS_FakeEndpointV2>(r);
}

#if defined(_WIN32)
std::unique_ptr<VS_FakeEndpoint> VS_FakeEndpointFactoryV2::Create(const VS_IPPortAddress&, const VS_IPPortAddress&)
{
	return nullptr;
}
#endif
