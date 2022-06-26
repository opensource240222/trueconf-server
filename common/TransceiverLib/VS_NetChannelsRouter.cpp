#include "VS_NetChannelsRouter.h"

#include "net/tls/socket.h"
#include "net/Lib.h"
#include "std-generic/cpplib/string_view.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/MakeShared.h"
#include "std/VS_AuthUtils.h"
#include "TransceiverCircuit/ConnectToServer.h"
#include "TransceiverLib_v2/NetChannelImp.h"
#include "VS_NetworkRelayMessage.h"
#include "std/debuglog/VS_Debug.h"

#include <memory>
#include <chrono>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

VS_NetChannelsRouter::VS_NetChannelsRouter(boost::asio::io_service& ios, std::string circuit_name)
	: m_isStopped(false)
	, m_stop_event(true)
	, m_strand(ios)
	, m_circuit_name(std::move(circuit_name))
{
}
VS_NetChannelsRouter::~VS_NetChannelsRouter()
{
}

void VS_NetChannelsRouter::ConnectToServer(string_view addresses, const unsigned char* secretData, const unsigned long sz, const char* channel_id)
{
	int32_t value = 0;
	const bool tlsEnabled = VS_RegistryKey(false, CONFIGURATION_KEY).GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "TransceiverRelayTLS") > 0 && value != 0;

	if (tlsEnabled)
		Connect<net::tls>(addresses, secretData, sz, channel_id);
	else
		Connect<boost::asio::ip::tcp>(addresses, secretData, sz, channel_id);
}

template <class Protocol>
void VS_NetChannelsRouter::Connect(string_view addresses, const unsigned char* secretData, const unsigned long sz, const char* channel_id)
{
	assert(channel_id && *channel_id);
	if (!channel_id || !*channel_id)
		return;

	ts::ConnectToServer<Protocol>(m_strand, addresses, std::chrono::seconds(1), [
		this,
		w_self = this->weak_from_this(),
		temp_password = auth::MakeTempPassword(string_view((const char*)secretData, sz), m_circuit_name),
		channel_id = std::string(channel_id)
	](const boost::system::error_code& ec, typename Protocol::socket&& socket)
	{
		auto self = w_self.lock();
		if (!self)
			return;

		if (m_isStopped)
			return;

		if (ec)
		{
			dstream0 << "Couldn't create connection for channel '" << channel_id << "': " << ec.message();
			return;
		}

		auto channel = vs::MakeShared<ts::NetChannel<typename Protocol::socket>>(socket.get_io_service());
		channel->SetRecvMessageCallBack(
			[this](const boost::shared_ptr<VS_MainRelayMessage>& rcvMess)
			{
				ProcessingRcvMessage(rcvMess);
			}
		);
		if (!channel->SetChannelConnection(std::move(socket)))
		{
			DeleteChannel(channel);
			return;
		}
		channel->RequestRead();
		const auto it = m_channels.find(channel_id);
		if (it != m_channels.end())
		{
			DeleteChannel(it->second);
			it->second = std::move(channel);
		}
		else
			m_channels[channel_id] = std::move(channel);

		SendMsg(channel_id, boost::make_shared<VS_StartFrameTransmitterMess>((const unsigned char*)temp_password.data(), temp_password.size(), channel_id.c_str()));
	});
}

void VS_NetChannelsRouter::SendMsg(const std::string &channel_id, const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess)
{
	m_strand.dispatch([this, channel_id, w_self = this->weak_from_this(), mess]() {
		auto self = w_self.lock();
		if (!self)
			return;
		auto i = m_channels.find(channel_id);
		if (m_channels.end() != i)
			i->second->SendMsg(mess);
	});
}
void VS_NetChannelsRouter::DeleteChannel(const std::shared_ptr<ts::NetChannelInterface> &channel)
{
	assert(m_strand.running_in_this_thread());
	channel->StopActivity();
}
void VS_NetChannelsRouter::CloseChannel(const std::string &id)
{
	m_strand.dispatch([this, w_self = this->weak_from_this(), id]() {
		auto self = w_self.lock();
		if (!self)
			return;

		auto i = m_channels.find(id);
		if (i != m_channels.end())
		{
			auto ch = i->second;
			m_channels.erase(i);
			DeleteChannel(ch);
		}
	});
}
void VS_NetChannelsRouter::Stop()
{
	m_strand.dispatch([this, w_self = this->weak_from_this()]() {
		auto self = w_self.lock();
		if (!self)
			return;

		m_isStopped = true;
		while (m_channels.size())
		{
			auto ch = m_channels.begin()->second;
			m_channels.erase(m_channels.begin());
			DeleteChannel(ch);
		}
		m_stop_event.set();
	});
}
void VS_NetChannelsRouter::WaitForStop()
{
	if (m_strand.running_in_this_thread())
		return;
	m_stop_event.wait();
}

#undef DEBUG_CURRENT_MODULE
