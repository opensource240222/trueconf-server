#if defined(_WIN32) // Windows only code

#include "FakeClient/VS_FakeEndpointV1.h"
#include "acs/connection/VS_BufferedConnectionBase.h"
#include "acs/connection/VS_ConnectionByte.h"
#include "transport/Message.h"
#include "newtransport/Handshake.h"
#include "transport/Lib/VS_TransportLib.h"
#include "transport/Router/VS_TransportHandler.h"
#include "std/cpplib/VS_WorkThreadIOCP.h"
#include "std/cpplib/event.h"
#include "std/debuglog/VS_Debug.h"

#include "std-generic/cpplib/ignore.h"

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#include <cassert>
#include <chrono>

#define DEBUG_CURRENT_MODULE VS_DM_FAKE_CLIENT

#define FAKECLIENT_VERBOSE_LOGS 0

class VS_FakeEndpointV1::Connection final : public VS_BufferedConnectionBase
{
public:
	Connection();

	bool Init(const boost::shared_ptr<VS_WorkThread>& thread, VS_TransportRouter_SetConnection* router, const char* our_endpoint, const VS_IPPortAddress& bind_addr, const VS_IPPortAddress& peer_addr);
	void Stop();
	bool Send(const transport::Message& message);

private:
	size_t onReceive(const void* data, size_t size) override;
	void onError(unsigned err) override;
	void Timeout() override;

private:
	friend VS_FakeEndpointV1;

	std::weak_ptr<Receiver> m_receiver;
	std::string m_cid;
	std::string m_my_server;
	vs::event m_init_done;
};

VS_FakeEndpointV1::Connection::Connection()
	: m_init_done(true)
{
}

bool VS_FakeEndpointV1::Connection::Init(const boost::shared_ptr<VS_WorkThread>& thread, VS_TransportRouter_SetConnection* router, const char* our_endpoint, const VS_IPPortAddress& bind_addr, const VS_IPPortAddress& peer_addr)
{
	if (!thread)
		return false;
	if (!router)
		return false;

	VS_BufferedConnectionBase::Init();
	VS_ConnectionByte* c = new VS_ConnectionByte();
	VS_ConnectionByte* remote = new VS_ConnectionByte(bind_addr, peer_addr);

	if (!c->Create(vs_pipe_type_duplex) || !remote->Open(c, vs_pipe_type_duplex) || !SetConnection(c, thread) )
	{
		delete c;
		delete remote;
		return false;
	}

	router->SetConnection(c->Name(), VS_CURRENT_TRANSPORT_VERSION, our_endpoint, remote, true, 0, 0, 0, 0, 0, 0, 0, false, true);
	if (!m_init_done.wait_for(std::chrono::seconds(2)))
	{
		dstream1 << "VS_FakeEndpointV1 wait_for 2 sec";
		Shutdown();
		return false;
	}

	assert(!m_cid.empty());

	if (our_endpoint)
		m_my_server = our_endpoint;

	return true;
}

void VS_FakeEndpointV1::Connection::Stop()
{
	Shutdown();
}

bool VS_FakeEndpointV1::Connection::Send(const transport::Message& message)
{
	// TODO: We could extract std::vector holding the data instead of copying it
	vs::SharedBuffer sb(message.Size());
	std::memcpy(sb.data<void>(), message.Data(), message.Size());
	return VS_BufferedConnectionBase::Send(std::move(sb));
}

size_t VS_FakeEndpointV1::Connection::onReceive(const void* data, size_t size)
{
	if (m_cid.empty()) // Waiting for handshake response
	{
		if (size < sizeof(net::HandshakeHeader))
			return 0;

		const auto hs = static_cast<const net::HandshakeHeader*>(data);
		const auto hs_size = sizeof(net::HandshakeHeader) + hs->body_length + 1;
		if (size < hs_size)
			return 0;

		auto res = transport::HandshakeResult::verification_failed;
		const char* cid;
		if (transport::ParseHandshakeReply(hs, vs::ignore<const char*>(), cid, res, vs::ignore<uint16_t>(), vs::ignore<uint8_t>(), vs::ignore<uint8_t>(), vs::ignore<bool>()) && res == transport::HandshakeResult::ok)
		{
			m_cid = cid;
			m_init_done.set();
		}
		else
			m_init_done.set();
		return hs_size;
	}

	if (size < sizeof(transport::MessageFixedPart))
		return 0;

	const auto receiver = m_receiver.lock();
	if (!receiver)
		return 0;

	const auto msg = static_cast<const transport::MessageFixedPart*>(data);
	const auto msg_size = msg->head_length + msg->body_length + 1;
	if (size < msg_size)
		return 0;

	receiver->OnReceive(transport::Message(data, msg_size));
	return msg_size;
}

void VS_FakeEndpointV1::Connection::onError(unsigned err)
{
	if (const auto receiver = m_receiver.lock())
		receiver->OnError(err);
}

void VS_FakeEndpointV1::Connection::Timeout()
{
	VS_BufferedConnectionBase::Timeout();
	if (const auto receiver = m_receiver.lock())
		receiver->Timeout();
}

VS_FakeEndpointV1::~VS_FakeEndpointV1()
{
	Stop();
}

bool VS_FakeEndpointV1::Init(const boost::shared_ptr<VS_WorkThread>& thread, VS_TransportRouter_SetConnection* router, const char* our_endpoint, const VS_IPPortAddress& bind_addr, const VS_IPPortAddress& peer_addr)
{
	assert(!m_connection);
	m_connection = std::make_shared<Connection>();
	if (!m_connection->Init(thread, router, our_endpoint, bind_addr, peer_addr))
	{
		dstream1 << "Failed to create VS_FakeEndpointV1";
		m_connection = nullptr;
		return false;
	}
#if FAKECLIENT_VERBOSE_LOGS
	dstream4 << "VS_FakeEndpointV1(" << m_connection->m_cid << "): new";
#endif
	return true;
}

void VS_FakeEndpointV1::SetReceiver(std::weak_ptr<Receiver> receiver)
{
	assert(m_connection);
	m_connection->m_receiver = std::move(receiver);
}

void VS_FakeEndpointV1::Stop()
{
	if (!m_connection)
		return;
	m_connection->Stop();
#if FAKECLIENT_VERBOSE_LOGS
	dstream4 << "VS_FakeEndpointV1(" << m_connection->m_cid << "): stopped";
#endif
}

const std::string& VS_FakeEndpointV1::CID() const
{
	assert(m_connection);
	return m_connection->m_cid;
}

bool VS_FakeEndpointV1::Send(transport::Message&& message)
{
	assert(m_connection);
	return m_connection->Send(message);
}

void VS_FakeEndpointFactory::InitV1(VS_TransportRouter_SetConnection* router, string_view our_endpoint)
{
	if (s_instance)
		return;
	s_instance = std::make_unique<VS_FakeEndpointFactoryV1>(router, our_endpoint);
}

VS_FakeEndpointFactoryV1::VS_FakeEndpointFactoryV1(VS_TransportRouter_SetConnection* router, string_view our_endpoint)
	: m_router(router)
	, m_our_endpoint(our_endpoint)
	, m_thread(boost::make_shared<VS_WorkThreadIOCP>())
{
	m_thread->Start("FakeEndpoint");
}

VS_FakeEndpointFactoryV1::~VS_FakeEndpointFactoryV1()
{
	Stop();
}

void VS_FakeEndpointFactoryV1::Stop()
{
	m_thread = nullptr;
}

std::unique_ptr<VS_FakeEndpoint> VS_FakeEndpointFactoryV1::Create()
{
	return Create({}, {});
}

std::unique_ptr<VS_FakeEndpoint> VS_FakeEndpointFactoryV1::Create(const VS_IPPortAddress& bind_addr, const VS_IPPortAddress& peer_addr)
{
	auto result = std::make_unique<VS_FakeEndpointV1>();
	if (!result->Init(m_thread, m_router, m_our_endpoint.c_str(), bind_addr, peer_addr))
		result = nullptr;
	return result;
}

#endif
