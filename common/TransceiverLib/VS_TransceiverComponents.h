#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>
#include <boost/asio/io_service.hpp>
#include "VS_RemoteCircuitController.h"

class VS_CircuitProcessControl;
class VS_AccessConnectionSystem;
class VS_WorkThread;
class VS_AuthConnectionInterface;
class VS_RemoteCircuitFrameTransmit;
class VS_SetConnectionInterface;
class VS_ConfControlModule;
class VS_RTPModuleControl;
class VS_TransceiverNetChannel;
class VS_TransceiverProxy;

namespace auth {
	class Transceiver;
}

/* transceiver */
namespace ts {
	class Components {
	public:
		bool Init(VS_AccessConnectionSystem *acs,
					const boost::shared_ptr<VS_WorkThread> &work_thread,
					std::function<void(const std::string&)> &transReadyCB,
					boost::asio::io_service& io, bool createProcess = true);
		~Components();
		Components() {}
		Components(const Components&) = delete;

		std::shared_ptr<VS_CircuitProcessControl> GetCircuitProcessControl() const;
		std::shared_ptr<VS_RemoteCircuitController> GetCircuitController() const;
		std::shared_ptr<VS_RemoteCircuitFrameTransmit> GetCircuitFrameTransmit() const;
		boost::shared_ptr<auth::Transceiver> GetAuthenticator() const;
		std::shared_ptr<VS_TransceiverNetChannel> GetNetChannel() const;
		std::shared_ptr<VS_TransceiverProxy> GetTransceiverProxy() const;
		bool HasCreatedTransceiver() const;
		std::string GetTransceiverName() const;
	private:
		std::shared_ptr<VS_CircuitProcessControl> m_circuit_process_ctrl;
		std::shared_ptr<VS_TransceiverNetChannel>	m_ts_net_channel;
		std::shared_ptr<VS_RemoteCircuitController> m_circuit_ctrl;
		std::shared_ptr<VS_RemoteCircuitFrameTransmit> m_frame_transmit;
		std::shared_ptr<VS_ConfControlModule> m_conf_ctrl;
		std::shared_ptr<VS_RTPModuleControl>  m_rtp_ctrl;
		std::shared_ptr<VS_TransceiverProxy> m_proxy;
		boost::shared_ptr<auth::Transceiver> m_ts_auth;
		VS_AccessConnectionSystem* m_acs = nullptr;
	};
}