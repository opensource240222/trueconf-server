#pragma once

#include "TransceiverLib/TransceiversPoolBase.h"
#include "TransceiverComponents.h"
#include "NetChannelImp.h"
#include "FrameTransmitter.h"
#include "acs_v2/ISetConnection.h"
#include "TransceiverLib/VS_ProtocolConst.h"
#include "net/Handshake.h"
#include "TransceiverLib/VS_TransceiverAuthenticator.h"

#include <boost/asio/ip/tcp.hpp>

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

/* transceiver */
namespace ts {
	template<class Socket = boost::asio::ip::tcp::socket>
	class ProxiesPoolV2
		: public ProxiesPoolBase<ComponentsV2, NetChannel<Socket>, FrameTransmitter>
		, public net::ISetConnection<Socket>
	{
		using base_t = ProxiesPoolBase<ComponentsV2, NetChannel<Socket>, FrameTransmitter>;
		std::weak_ptr<acs::Service> m_acs;

		bool InitProxyComponents(ComponentsV2 &components, bool createTransceiverProcess) override;
	public:
		ProxiesPoolV2(boost::asio::io_service& io, unsigned minFreeProxyes, unsigned maxAllowedConfByTransceiver, std::chrono::minutes maxTransceiverUnusedDuration);
		ProxiesPoolV2(const ProxiesPoolV2&) = delete;
		ProxiesPoolV2& operator=(const ProxiesPoolV2&) = delete;

		void Init(const std::shared_ptr<acs::Service> &acs);
		bool SetTCPConnection(Socket&& socket, acs::Handler::stream_buffer&& buffer) override;
	};

	template<class Socket>
	inline bool ProxiesPoolV2<Socket>::InitProxyComponents(ComponentsV2 & components, bool createTransceiverProcess)
	{
		return components.Init(m_acs.lock(), base_t::onTransceiverReady, base_t::m_ios, createTransceiverProcess);
	}

	template<class Socket>
	inline ProxiesPoolV2<Socket>::ProxiesPoolV2(boost::asio::io_service& io, unsigned minFreeProxyes, unsigned maxAllowedConfByTransceiver, std::chrono::minutes maxTransceiverUnusedDuration)
		: ProxiesPoolBase<ComponentsV2, NetChannel<Socket>, FrameTransmitter>(io ,minFreeProxyes, maxAllowedConfByTransceiver, maxTransceiverUnusedDuration)
	{
	}

	template<class Socket>
	inline void ProxiesPoolV2<Socket>::Init(const std::shared_ptr<acs::Service> &acs)
	{
		m_acs = acs;

		for (unsigned i = 0; i < base_t::m_minFreeProxyes; ++i) {
			if (!base_t::InitNewTransceiverProxy(true))
				break;
			dstream2 << "ProxiesPool::Init push transceiver proxy to pool. Current pool size = '" << base_t::m_proxies.proxies.size() << "'\n";
		}

		base_t::ScheduleTimer();
	}

	template<class Socket>
	inline bool ProxiesPoolV2<Socket>::SetTCPConnection(Socket && socket, acs::Handler::stream_buffer && buffer)
	{
		const auto& hs = *reinterpret_cast<net::HandshakeHeader*>(buffer.data());
		if (hs.version < VS_CIRCUIT_MIN_VERSION)
			return false;

		std::shared_ptr<net::ISetConnection<Socket>> set_conn;
		if (strncmp(hs.primary_field, VS_Circuit_PrimaryField, sizeof(hs.primary_field)) == 0) {
			auto login = auth::Transceiver::GetLoginFromHandshake(string_view((const char*)buffer.data(), buffer.size()));
			auto ch = base_t::GetFreeNetChannel(login);
			if (!ch)
				return false;

			ch->SetOnConnDie([login = std::string(login), w_this = this->weak_from_this()]() {
				if (auto self = w_this.lock())
					self->OnNetChannelDie(login);
			});
			set_conn = std::static_pointer_cast<net::ISetConnection<Socket>>(ch);
		}
		else if (strncmp(hs.primary_field, VS_FrameTransmit_PrimaryField, sizeof(hs.primary_field)) == 0) {
			const char *conf_name(nullptr);
			VS_StartFrameTransmitterMess mess;
			if (!mess.SetMessage(static_cast<const unsigned char*>(buffer.data()), buffer.size()) || !(conf_name = mess.GetConferenceName())) return false;
			set_conn = std::static_pointer_cast<net::ISetConnection<Socket>>(base_t::GetFrameTransmit(conf_name));
		}

		if (!set_conn || !set_conn->SetTCPConnection(std::move(socket), std::move(buffer))) {
			return false;
		}

		return true;
	}
}

#undef DEBUG_CURRENT_MODULE
