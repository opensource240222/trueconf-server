#pragma once
#include "../std/cpplib/VS_UserData.h"
#include "statuslib/status_types.h"
#include "../std/cpplib/VS_Lock.h"
#include "std-generic/cpplib/macro_utils.h"
#include "std-generic/asio_fwd.h"
#include "net/UDPRouter.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include "std/cpplib/ASIOThreadPool.h"

namespace ts { struct IPool; }

class  VS_SignalConnectionsMgr;
class VS_SIPPresenceService;
namespace transport {
	struct IRouter;
}
class VS_TranscoderLogin;
class VS_SIPAddressBookService;
class VS_AccessConnectionSystem;
class VS_TranscoderKeeper;
namespace sip { class TransportLayer; }
class VS_SIPCallResolver;
class VS_SIPVisiProxy;
class VS_VisiSIPProxy;
class VS_GatewayService;
namespace acs{ class Service; }
namespace net { struct Endpoint;  class LoggerInterface; }

enum class GWInitStatus : unsigned
{
	notStarted = 0x00,
	h323Binded = 0x01,
	sipBinded = 0x10,
	started = 0x11
};
VS_ENUM_BITOPS(GWInitStatus, unsigned)

class VS_GatewayStarter : VS_Lock
{
protected:
static VS_GatewayStarter *m_instance;
static VS_Lock	*m_lock_instance;
private:

	static bool				m_isVcs;
	static bool				m_isAS;
	static UserStatusFunction m_get_user_status;

	static std::function<std::string(string_view)> m_get_app_property;
	static std::function<std::string(string_view)> m_get_web_manager_property;

	bool m_componentsCreated;
	std::shared_ptr< VS_SignalConnectionsMgr > m_H323Protocol;
	boost::shared_ptr<VS_GatewayService> m_GatewayService;
	std::shared_ptr<sip::TransportLayer>	m_sipTransport;
	std::shared_ptr<VS_SIPCallResolver> m_sipCallResolver;
	std::shared_ptr<VS_SIPVisiProxy> m_sipVisiProxy;
	std::shared_ptr<VS_VisiSIPProxy> m_visiSIPProxy;
	boost::optional<boost::asio::io_service::strand> m_sipStrand;
	std::shared_ptr<VS_TranscoderKeeper> m_trKeeper;
	boost::optional<boost::asio::io_service::strand> m_strand;
	transport::IRouter *m_tr;
	std::shared_ptr<acs::Service>	m_acsSrv;

	vs::ASIOThreadPool m_threadPool; //TODO:FIXME(for VS_Indentifier)

	std::shared_ptr<net::LoggerInterface> m_logger = nullptr;
														
	void STARTMESS(const char *service);
	VS_GatewayStarter();

	void StopSip();
	bool StartGatewayACS(boost::asio::io_service& ios, std::set<net::Endpoint> &m_localEps, bool &hasSipListeners, bool &hasH323listeners);
	void StopGatewayACS();
public:

	virtual ~VS_GatewayStarter();

	static VS_GatewayStarter *GetInstance();

	GWInitStatus StartGateway(
		boost::asio::io_service& ios,
		transport::IRouter* tr,
		const std::function<bool(const std::string&, const std::string&)>& check_digest, const std::weak_ptr<ts::IPool>& transceiversPool,
		const std::weak_ptr<VS_TranscoderLogin>& transLogin,
		const std::string& serverInfo);
	void StopGateway();
	bool IsStarted() const {return m_componentsCreated;}
	void SetMaxTranscoders(const int max_transcoders);
	void SetServerAddresses(const char *addr);
	bool HasTelCfg() const;
	void OnUserLoginEnd_Event(const VS_UserData& ud, const std::string &cid);
	void OnUserLogoff_Event(const VS_UserData& ud, const std::string &cid);
	void OnNewPeerCfg_Event(string_view callId, const std::vector<VS_ExternalAccount>& v);
	static void SetIsVCS( bool isVcs );
	static bool IsVCS();
	static void SetIsAS(bool isAS);
	static bool IsAS();
	static void SetGetUserStatusFunction(const UserStatusFunction &func);

	static void SetGetAppPropertyFunciton(const std::function<std::string(string_view)> &func);
	static void SetGetWebManagerPropertyFunciton(const std::function<std::string(string_view)> &func);
};