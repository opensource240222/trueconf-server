#pragma once

#include "TrueGateway/sip/SIPTransportChannel.h"
#include "std/cpplib/event.h"
#include "std-generic/cpplib/scope_exit.h"
#include "net/Endpoint.h"
#include "tests/mocks/asio_socket_mock.h"
#include "acs_v2/Handler.h"

namespace test{

struct FakeChannel : public sip::Channel {

	void Close() override {
		if (auto listener = m_eventListener.lock())
		{
			vs::event done(true);
			m_strand.dispatch([&]()
			{
				VS_SCOPE_EXIT{ done.set(); };
				listener->OnConnectionDie(m_id);
			});
			done.wait();
		}
	};
	void Write(vs::SharedBuffer &&data) override{}
	const std::string& LogID() { return logID; }
	void AcceptMessage(unsigned char* data, size_t size) {
		m_queueIn.PutMessageWithFilters(data, size, e_SIP_CS, VS_SIPInputMessageQueue::FLT_NONE);
	}
	std::shared_ptr<VS_SIPMessage> GetMsg() {
		std::shared_ptr<VS_SIPMessage> res;
		vs::event done(true);
		m_strand.dispatch([&]() {
			VS_SCOPE_EXIT{ done.set(); };
			res = sip::Channel::GetMsg();
		});

		done.wait();
		return res;
	}

	template<class Handler>
	void Accept(std::shared_ptr<test::asio::tcp_socket_mock> && socket, std::shared_ptr<acs::Handler::stream_buffer> && buffer, Handler&& onAccept) {
		vs::event done(true);
		m_strand.dispatch([&]() {
			VS_SCOPE_EXIT{ done.set(); };
			boost::system::error_code ec;
			auto localEP = socket->local_endpoint(ec);
			auto remoteEP = socket->remote_endpoint(ec);

			localEp.addr = localEP.address();
			localEp.port = localEP.port();
			localEp.protocol = net::protocol::TCP;

			remoteEp.addr = remoteEP.address();
			remoteEp.port = remoteEP.port();
			remoteEp.protocol = net::protocol::TCP;

			onAccept(m_id, localEp, remoteEp);
			AcceptMessage(buffer->data(), buffer->size());
			ProcessInputMsgs();
		});

		done.wait();
	}

	template<class Handler>
	void Accept(std::shared_ptr<test::asio::udp_socket_mock> && socket, std::shared_ptr<acs::Handler::packet_buffer> && buffer, Handler&& onAccept) {
		vs::event done(true);
		m_strand.dispatch([&]() {
			VS_SCOPE_EXIT{ done.set(); };
			boost::system::error_code ec;
			auto localEP = socket->local_endpoint(ec);
			auto remoteEP = socket->remote_endpoint(ec);

			localEp.addr = localEP.address();
			localEp.port = localEP.port();
			localEp.protocol = net::protocol::UDP;

			remoteEp.addr = remoteEP.address();
			remoteEp.port = remoteEP.port();
			remoteEp.protocol = net::protocol::UDP;

			onAccept(m_id, localEp, remoteEp);
			while (!buffer->Empty()) {
				m_queueIn.PutMessageWithFilters(reinterpret_cast<unsigned char*>(buffer->Front().Data()), buffer->Front().Size(), e_SIP_CS, VS_SIPInputMessageQueue::DEFALUT_FILTERS);
				buffer->PopFront();
			}
			ProcessInputMsgs();
		});

		done.wait();
	}

	net::Endpoint localEp;
	net::Endpoint remoteEp;
	std::string logID;
protected:
	template<typename ...Args>
	FakeChannel(Args&&... args)
		: sip::Channel(std::forward<Args>(args)...)
	{}
	static void PostConstruct(std::shared_ptr<FakeChannel>&) { /*stub*/ }
};

}