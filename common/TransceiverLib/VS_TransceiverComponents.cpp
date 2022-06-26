#ifdef _WIN32	// not ported

#include "VS_TransceiverComponents.h"
#include "VS_CircuitProcessControl.h"
#include "VS_ConfControlModule.h"
#include "VS_RemoteCircuitFrameTransmit.h"
#include "VS_RTPModuleControl.h"
#include "VS_TransceiverAuthenticator.h"
#include "VS_TransceiverNetChannel.h"
#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"
#include "std/VS_TransceiverInfo.h"
#include "std/cpplib/MakeShared.h"
#include "std/cpplib/VS_WorkThread.h"
#include "VS_TransceiverProxy.h"

#include <boost/make_shared.hpp>

bool ts::Components::Init(VS_AccessConnectionSystem * acs,
	const boost::shared_ptr<VS_WorkThread>& work_thread,
	std::function<void(const std::string&)> &transReadyCB,
	boost::asio::io_service& io,
	bool createProcess)
{
	if (!acs || !acs->IsValid()|| !work_thread) return false;

	m_conf_ctrl = std::make_shared<VS_ConfControlModule>();
	m_rtp_ctrl = std::make_shared<VS_RTPModuleControl>();
	m_ts_auth = boost::make_shared<auth::Transceiver>();

	m_ts_net_channel = std::make_shared<VS_TransceiverNetChannel>(work_thread);
	m_ts_net_channel->SetConnectionAuthenticator(m_ts_auth);

	m_circuit_ctrl = std::make_shared<VS_RemoteCircuitController>(m_ts_net_channel);
	m_frame_transmit = vs::MakeShared<VS_RemoteCircuitFrameTransmit>(work_thread, m_ts_auth);

	if (ts::UseLocalTransceiver() && createProcess) {
		m_circuit_process_ctrl = vs::MakeShared<VS_CircuitProcessControl>("tc_transceiver.exe", io, [acs](std::string& listeners) -> int {
			if (!acs) return 0;

			int res(0);
			std::string listeners_str;
			if ((res = acs->GetListeners(listeners_str)) > 0 && !listeners_str.empty()) listeners = listeners_str;
			return res;
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
	m_ts_net_channel->SetOnTransceiverReady(transReadyCB);
	m_acs = acs;
	return true;
}

ts::Components::~Components()
{
	if (m_circuit_process_ctrl) {
		m_circuit_process_ctrl->Stop();
		m_circuit_process_ctrl->WaitStop();
	}
	if (m_circuit_ctrl) m_circuit_ctrl->Stop();
	if (m_frame_transmit) {
		m_frame_transmit->Close();
		m_frame_transmit->WaitClose();
	}
}

std::shared_ptr<VS_CircuitProcessControl> ts::Components::GetCircuitProcessControl() const
{
	return m_circuit_process_ctrl;
}

std::shared_ptr<VS_RemoteCircuitController> ts::Components::GetCircuitController() const
{
	return m_circuit_ctrl;
}

std::shared_ptr<VS_RemoteCircuitFrameTransmit> ts::Components::GetCircuitFrameTransmit() const
{
	return m_frame_transmit;
}

boost::shared_ptr<auth::Transceiver> ts::Components::GetAuthenticator() const
{
	return m_ts_auth;
}

std::shared_ptr<VS_TransceiverNetChannel> ts::Components::GetNetChannel() const
{
	return m_ts_net_channel;
}

std::shared_ptr<VS_TransceiverProxy> ts::Components::GetTransceiverProxy() const
{
	return m_proxy;
}

bool ts::Components::HasCreatedTransceiver() const
{
	return m_circuit_process_ctrl != nullptr && m_circuit_process_ctrl->IsStarted();
}

std::string ts::Components::GetTransceiverName() const
{
	if (HasCreatedTransceiver()) {
		return m_circuit_process_ctrl->GetTransceiverName();
	}

	return m_proxy->GetTransceiverName();
}

#endif // _WIN32