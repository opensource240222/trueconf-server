#include "TransceiverComponents.h"
#include "TransceiverLib/VS_TransceiverAuthenticator.h"
#include "NetChannelImp.h"
#include "FrameTransmitter.h"
#include "Handler.h"
#include "acs_v2/Service.h"
#include "std/VS_TransceiverInfo.h"
#include "std/cpplib/MakeShared.h"
#include "TransceiverLib/VS_CircuitProcessControl.h"
#include "TransceiverLib/VS_ConfControlModule.h"
#include "TransceiverLib/VS_RTPModuleControl.h"
#include "TransceiverLib/VS_TransceiverProxy.h"

bool ts::ComponentsV2::Init(const std::shared_ptr<acs::Service> &acs, std::function<void(const std::string&)>& transReadyCB, boost::asio::io_service & io, bool createProcess)
{
	if (!acs) return false;

	m_ts_auth = std::make_shared<auth::Transceiver>();
	m_ts_net_channel = vs::MakeShared<NetChannel<>>(io, m_ts_auth, transReadyCB);
	m_circuit_ctrl = std::make_shared<VS_RemoteCircuitController>(m_ts_net_channel);
	m_frame_transmit = vs::MakeShared<FrameTransmitter>(m_ts_auth);
	m_conf_ctrl = std::make_shared<VS_ConfControlModule>();
	m_rtp_ctrl = std::make_shared<VS_RTPModuleControl>();

	if (ts::UseLocalTransceiver() && createProcess) {
		m_circuit_process_ctrl = vs::MakeShared<VS_CircuitProcessControl>(ts::PROCESS_EXE_NAME, io, [acs](std::string& listeners) -> int {
			if (!acs) return 0;

			// get list
			acs::Service::address_list listener_list;
			for (const auto& protocol : { net::protocol::TCP ,net::protocol::UDP ,net::protocol::TLS })
			{
				acs::Service::address_list list;
				acs->GetListenerList(list, protocol);
				listener_list.insert(listener_list.end(), list.begin(), list.end());
			}

			// get addr string
			for (const auto& endp : listener_list)
			{
				boost::system::error_code ec;
				auto addr_str = endp.first.to_string(ec);
				if (ec != boost::system::errc::success) continue;

				char port_str[64] = {};
				snprintf(port_str, 64, "%u", endp.second);
				addr_str += ':'; addr_str += port_str;

				listeners += addr_str; listeners += ',';
			}

			return listener_list.size();
		});

		if (!m_circuit_ctrl->RegisterModule(m_circuit_process_ctrl)) return false;
		m_circuit_process_ctrl->SetMessageSender(m_ts_net_channel);
		m_circuit_process_ctrl->SheduleTimer(std::chrono::seconds(1));
	}

	m_proxy = std::make_shared<VS_TransceiverProxy>(m_circuit_ctrl, m_ts_net_channel, m_frame_transmit, m_conf_ctrl, m_rtp_ctrl);
	std::weak_ptr<VS_TransceiverProxy> w_proxy(m_proxy);
	m_ts_net_channel->SetTransceiverName([w_proxy](const std::string & name) {
		if (auto proxy = w_proxy.lock())
			proxy->SetTransceiverName(name);
	});
	m_acs = acs;
	return true;
}

ts::ComponentsV2::~ComponentsV2()
{
	if (m_circuit_process_ctrl) {
		m_circuit_process_ctrl->Stop();
		m_circuit_process_ctrl->WaitStop();
	}
	if (m_circuit_ctrl) m_circuit_ctrl->Stop();
}

std::shared_ptr<VS_CircuitProcessControl> ts::ComponentsV2::GetCircuitProcessControl() const
{
	return m_circuit_process_ctrl;
}

std::shared_ptr<VS_RemoteCircuitController> ts::ComponentsV2::GetCircuitController() const
{
	return m_circuit_ctrl;
}

std::shared_ptr<ts::FrameTransmitter> ts::ComponentsV2::GetCircuitFrameTransmit() const
{
	return m_frame_transmit;
}

std::shared_ptr<auth::Transceiver> ts::ComponentsV2::GetAuthenticator() const
{
	return m_ts_auth;
}

std::shared_ptr<ts::NetChannel<>> ts::ComponentsV2::GetNetChannel() const
{
	return m_ts_net_channel;
}

std::shared_ptr<VS_TransceiverProxy> ts::ComponentsV2::GetTransceiverProxy() const
{
	return m_proxy;
}

bool ts::ComponentsV2::HasCreatedTransceiver() const
{
	return m_circuit_process_ctrl != nullptr && m_circuit_process_ctrl->IsStarted();
}

std::string ts::ComponentsV2::GetTransceiverName() const
{
	if (HasCreatedTransceiver()) {
		return m_circuit_process_ctrl->GetTransceiverName();
	}

	return m_proxy->GetTransceiverName();
}
