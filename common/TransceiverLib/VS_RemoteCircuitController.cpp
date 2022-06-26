#include "TransceiverLib/VS_RemoteCircuitController.h"
#include "TransceiverLib/NetChannelInterface.h"
#include "TransceiverLib/VS_ControlRelayMessage.h"
#include "TransceiverLib/VS_RelayModule.h"

#include <boost/make_shared.hpp>

VS_RemoteCircuitController::VS_RemoteCircuitController(std::shared_ptr<ts::NetChannelInterface> ts_net_channel)
	: m_transceiver_net_channel(std::move(ts_net_channel))
	, m_live555_port(0)
	, m_live555_secret(std::make_shared<std::string>())
{
	assert(m_transceiver_net_channel != nullptr);

	m_transceiver_net_channel->SetRecvMessageCallBack([this](boost::shared_ptr<VS_MainRelayMessage>& msg) {
		switch (msg->GetMessageType())
		{
		case VS_MainRelayMessage::e_envelope:
		{
			auto module = GetModule(msg->GetModuleName());
			if (module)
			{
				module->ProcessingMessage(msg);
				msg = boost::make_shared<VS_MainRelayMessage>();
			}
			else
				msg->Clear();
			break;
		}
		case VS_MainRelayMessage::e_live555_info:
		{
			const auto ctrl_msg = static_cast<const VS_ControlRelayMessage*>(msg.get());
			m_live555_port.store(ctrl_msg->GetLive555Port(), std::memory_order_relaxed);
			m_live555_secret.store(std::make_shared<std::string>(ctrl_msg->GetLive555Secret()), std::memory_order_relaxed);
			msg = boost::make_shared<VS_MainRelayMessage>();
			break;
		}
		}
	});
}

VS_RemoteCircuitController::~VS_RemoteCircuitController() = default;

void VS_RemoteCircuitController::Stop()
{
	m_transceiver_net_channel->StopActivity();
}

net::port VS_RemoteCircuitController::GetLive555Port() const
{
	return m_live555_port.load(std::memory_order_relaxed);
}

std::string VS_RemoteCircuitController::GetLive555Secret() const
{
	return *m_live555_secret.load(std::memory_order_relaxed);
}

void VS_RemoteCircuitController::StartConference(const char* conf_name)
{
	auto msg = boost::make_shared<VS_ControlRelayMessage>();
	if (msg->MakeConferenceStart(conf_name))
		m_transceiver_net_channel->SendMsg(msg);
}

void VS_RemoteCircuitController::StartConference(const char* conf_name, const char* owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type)
{
	auto msg = boost::make_shared<VS_ControlRelayMessage>();
	if (msg->MakeConferenceStart(conf_name, owner_name, type, sub_type))
		m_transceiver_net_channel->SendMsg(msg);
}

void VS_RemoteCircuitController::StopConference(const char* conf_name)
{
	auto msg = boost::make_shared<VS_ControlRelayMessage>();
	if (msg->MakeConferencsStop(conf_name))
		m_transceiver_net_channel->SendMsg(msg);
}

void VS_RemoteCircuitController::ParticipantConnect(const char* conf_name, const char* part_name)
{
	auto msg = boost::make_shared<VS_ControlRelayMessage>();
	if (msg->MakeParticipantConnect(conf_name, part_name))
		m_transceiver_net_channel->SendMsg(msg);
}

void VS_RemoteCircuitController::ParticipantDisconnect(const char* conf_name, const char* part_name)
{
	auto msg = boost::make_shared<VS_ControlRelayMessage>();
	if (msg->MakeParticipantDisconnect(conf_name, part_name))
		m_transceiver_net_channel->SendMsg(msg);
}

void VS_RemoteCircuitController::SetParticipantCaps(const char* conf_name, const char* part_name, const void* caps_buf, const unsigned long buf_sz)
{
	auto msg = boost::make_shared<VS_ControlRelayMessage>();
	if (msg->MakeSetPartCaps(conf_name, part_name, caps_buf, buf_sz))
		m_transceiver_net_channel->SendMsg(msg);
}

void VS_RemoteCircuitController::RestrictBitrateSVC(const char* conf_name, const char* part_name, long v_bitrate, long bitrate, long old_bitrate)
{
	auto msg = boost::make_shared<VS_ControlRelayMessage>();
	if (msg->MakeRestrictBitrateSVC(conf_name, part_name, v_bitrate, bitrate, old_bitrate))
		m_transceiver_net_channel->SendMsg(msg);
}

void VS_RemoteCircuitController::RequestKeyFrame(const char* conf_name, const char* part_name)
{
	auto msg = boost::make_shared<VS_ControlRelayMessage>();
	if (msg->MakeRequestKeyFrame(conf_name, part_name))
		m_transceiver_net_channel->SendMsg(msg);
}