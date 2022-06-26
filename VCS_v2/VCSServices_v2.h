#pragma once
#include "newtransport/Router/Router.h"
#include "newtransport/Router/ServiceBase.h"
#include "transport/Router/VS_TransportRouterServiceBase.h"
#include "streams_v2/Router/Router.h"
#include "http/Router.h"
#include "http_v2/Handler_v2.h"
#include "WebSocket_v2/WsHandler.h"
#include "CheckSrv_v2/Handler.h"
#include "TransceiverLib_v2/TransceiversPool.h"
#include "TransceiverLib_v2/Handler.h"
#include "tools/Server/VS_ServerComponentsInterface.h"
#include "ServerServices/VS_VerificationService.h"
#include "VCS/Services/VS_CheckLicenseService.h"
#include "std-generic/asio_fwd.h"

namespace transport {
	class IService;
};
class VS_RoamingSettings;
class VS_RTSPProxy;
class VS_TorrentStarter;
class VS_TransceiverProxy;
class VS_WebrtcPeerConnectionService;
class VS_TranscoderLogin;
namespace vs { class TLSHandlerFake; }

class VCSServices_v2
{
	boost::asio::io_service& m_ios;
	std::shared_ptr<transport::Router> m_tr;
	std::shared_ptr<stream::Router>	m_sr;
	std::shared_ptr<VS_RoamingSettings> m_roaming_settings;
	std::shared_ptr<vs::TLSHandlerFake> m_tls_handler;
	std::shared_ptr<http::Router> m_http_router;
	std::shared_ptr<http_v2::ACSHandler> m_http_handler;
	std::shared_ptr<ws::Handler>		m_websock_handler;
	std::shared_ptr<VS_RTSPProxy> m_rtsp_proxy;
	std::shared_ptr<ts::ProxiesPoolV2<> >		m_proxiesPool;
	std::shared_ptr<ts::Handler> m_transHandler;
	std::shared_ptr<vs_relaymodule::VS_StreamsRelay>	m_streams_relay;
	std::shared_ptr<VS_WebrtcPeerConnectionService> m_webrtc_pc_srv;
	std::shared_ptr<checksrv::Handler> m_checksrv_handler;

	std::shared_ptr<VS_TranscoderLogin> m_transLogin;
	std::unique_ptr<VS_CheckLicenseService> m_chck_lics;
	std::unique_ptr<VS_VerificationService> m_verify_srv;
	std::vector<std::shared_ptr<transport::ServiceBase>> m_services;												// vector<boost::variant>
	std::map<std::string, std::shared_ptr<VS_TransportRouterServiceBase>> m_old_services;		// [name, ptr]		// todo(kt): must be vector
	std::map<std::string, boost::shared_ptr<VS_TransportRouterServiceBase>> m_old_services_boost;		// [name, ptr]	// todo(kt): must be vector

	void OnTransceiverStarted(const std::string& transceiverName) const;
public:
	VCSServices_v2(boost::asio::io_service& ios);
	bool Init(vs::RegistrationParams&& rp, const std::shared_ptr<transport::Router>& tr,
		const std::shared_ptr<stream::RouterV2>& sr, const std::shared_ptr<acs::Service>& acs_srv,
		VS_RoutersWatchdog* watchdog, string_view product_version);
	void Destroy();
};
