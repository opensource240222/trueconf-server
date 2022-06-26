#pragma once
#include "AppServer/Services/VS_PresenceService.h"
#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "std/cpplib/VS_SimpleStr.h"
#include "std-generic/cpplib/VS_Container.h"
#include "tools/Watchdog/VS_RoutersWatchdog.h"
#include "std/cpplib/VS_Protocol.h"
#include "ServerServices/VS_TorrentStarter.h"
#include "std/cpplib/event.h"
#include "std-generic/asio_fwd.h"

namespace ts { struct IPool; }
class VS_TranscoderLogin;

#include <cstdint>

enum class GWInitStatus : unsigned;
namespace transport {
	struct IRouter;
}
class VS_AccessConnectionSystem;
class VS_RegistryKey;
class VS_TorrentStarterBase;

class VS_CheckLicenseService :
	public VS_TransportRouterServiceHelper
	, public VS_PresenceServiceMember
{
	VS_SimpleStr		m_regSrv;
	VS_RoutersWatchdog*		m_watchdog = nullptr;
	std::string			m_server_version;
	VS_RouterMessage*	m_recvMess;
	boost::asio::io_service& m_ios;
	transport::IRouter *m_tr;
	std::string			m_acs_listeners_tcp;

	std::shared_ptr<VS_TorrentStarterBase>	m_torrent_starter;

	VS_Container		m_arm_key_cnt;
	unsigned long		m_reg_disconnect_tick;


	VS_Server_States	m_state;

	VS_SimpleStr		m_my_id;
	unsigned int		m_CurrFlags;
  std::chrono::steady_clock::time_point		m_lastLicRefresh;

	unsigned long			m_LicExpTime,m_LicExpPeriod;		//< license expiration time & period in milliseconds
	unsigned long			m_LicUpdTime,m_LicUpdPeriod;		//< license update time in milliseconds
	static const unsigned long	MaxLicExpPeriod=24*60*59*1000;	//< maximum period of reload
	static const unsigned long	LicUpdShift=10*60*1000;					//< time before expiration to update licenses from server
	static const unsigned long	LicExpAdd=60*1000;							//< in milliseconds
	bool							m_ReloadLicense;
	vs::event		m_registrationEvent { true };
	std::weak_ptr<ts::IPool>	m_transceiversPool;
	std::weak_ptr<VS_TranscoderLogin> m_transLogin;

	unsigned int		m_licLimitCheckTime;
	uint32_t			m_licFeatures = 0;

	void SendLicensingInfo();
	void GetLicensingInfo(VS_Container& cnt);
	void ProcessLicense(VS_RegistryKey& l_root,uint64_t id,long type,const void* data,int size);
	void OnTimer_RefreshLicense(unsigned long ticks);
	void	WriteLicDataToReg(VS_Container &cnt);
	void NotifyOnGWStartError(GWInitStatus status);

	bool GetArmKeyFromCnt( VS_Container &cnt, VS_SimpleStr &arm_key,uint32_t&licFeatures);
	bool VerifyDataSign(const unsigned char *data,const unsigned long data_sz, const unsigned char*sign, const unsigned long sign_sz);
public:
	VS_CheckLicenseService(boost::asio::io_service& ios, std::unique_ptr<VS_TorrentStarterBase> torrentStarter);
	~VS_CheckLicenseService();
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	void Destroy(const char* our_endpoint, const char* our_service) override;
	void AsyncDestroy() override;
	bool Timer(unsigned long tickcount) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	// VS_PointConditions
	bool OnPointConnected_Event(const VS_PointParams* prm) override;
	bool OnPointDisconnected_Event(const VS_PointParams* prm) override;
	void ExternalIPDetermined_Handler(const char* ip);

	void SetComponents(VS_RoutersWatchdog* watchdog, transport::IRouter *tr, string_view acs_listeners_tcp, string_view ver, const std::weak_ptr<VS_TranscoderLogin>& transLogin);
	void SetTransceiversPool(const std::shared_ptr<ts::IPool> &pool);
	void ServerVerificationFailed();
	bool WaitForRegistrationEvent();

	// for functor to bind to dbStorage::CheckDigestByRegistry
	bool CheckDigest(const std::string& login, const std::string& pass);

	std::weak_ptr<VS_TorrentStarterBase> GetTorrentStarter() const;
};

