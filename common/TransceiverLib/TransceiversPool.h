#pragma once

#include "TransceiversPoolBase.h"

#include "VS_SetConnectionInterface.h"
#include "VS_TransceiverComponents.h"

/* transceiver */
namespace ts {

	class ProxiesPool : public ProxiesPoolBase<ts::Components,VS_TransceiverNetChannel, VS_RemoteCircuitFrameTransmit>
					  , public VS_SetConnectionInterface {
		VS_AccessConnectionSystem*	m_acs = nullptr;
		boost::shared_ptr<VS_WorkThread> m_workThread;

		bool InitProxyComponents(ts::Components &components, bool createTransceiverProcess) override;
	public:
		ProxiesPool(boost::asio::io_service& io, unsigned minFreeProxyes, unsigned maxAllowedConfByTransceiver, std::chrono::minutes maxTransceiverUnusedDuration);
		ProxiesPool(const ProxiesPool&) = delete;
		ProxiesPool& operator=(const ProxiesPool&) = delete;

		void Init(VS_AccessConnectionSystem* acs);
		bool SetTCPConnection(VS_ConnectionTCP *conn, const void *in_buf, const unsigned long in_len) override;
	};
}