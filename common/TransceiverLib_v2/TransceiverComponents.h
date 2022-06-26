#pragma once

#include <memory>
#include <boost/asio/io_service.hpp>

#include "TransceiverLib/VS_RemoteCircuitController.h"
#include "TransceiverLib_v2/NetChannel.h"

class VS_CircuitProcessControl;
class VS_ConfControlModule;
class VS_RTPModuleControl;
class VS_TransceiverProxy;

namespace auth {
	class Transceiver;
}

namespace acs {
	class Service;
}


/* transceiver */
namespace ts {
	class FrameTransmitter;
	class Handler;

	class ComponentsV2 {
	public:
		bool Init(const std::shared_ptr<acs::Service> &acs, std::function<void(const std::string&)> &transReadyCB, boost::asio::io_service& io, bool createProcess = true);
		~ComponentsV2();

		std::shared_ptr<VS_CircuitProcessControl> GetCircuitProcessControl() const;
		std::shared_ptr<VS_RemoteCircuitController> GetCircuitController() const;
		std::shared_ptr<FrameTransmitter> GetCircuitFrameTransmit() const;
		std::shared_ptr<auth::Transceiver> GetAuthenticator() const;
		std::shared_ptr<NetChannel<>> GetNetChannel() const;
		std::shared_ptr<VS_TransceiverProxy> GetTransceiverProxy() const;
		bool HasCreatedTransceiver() const;
		std::string GetTransceiverName() const;
	private:
		std::shared_ptr<VS_CircuitProcessControl> m_circuit_process_ctrl;
		std::shared_ptr<NetChannel<>>	m_ts_net_channel;
		std::shared_ptr<VS_RemoteCircuitController> m_circuit_ctrl;
		std::shared_ptr<FrameTransmitter> m_frame_transmit;
		std::shared_ptr<Handler> m_circuit_handler;
		std::shared_ptr<VS_ConfControlModule> m_conf_ctrl;
		std::shared_ptr<VS_RTPModuleControl> m_rtp_ctrl;
		std::shared_ptr<auth::Transceiver> m_ts_auth;
		std::shared_ptr<VS_TransceiverProxy> m_proxy;

		std::weak_ptr<acs::Service>  m_acs;
	};
}