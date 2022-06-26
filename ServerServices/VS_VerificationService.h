#pragma once

#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_SimpleStr.h"
#include "tools/Server/RegistrationParams.h"
#include "tools/Watchdog/VS_RoutersWatchdog.h"
#include "transport/Router/VS_TransportRouterServiceHelper.h"

#include <cstdint>
#include <functional>

class VS_Certificate;
class VS_RegistryKey;

class VS_VerificationService :
	public VS_TransportRouterServiceHelper
{
	vs::RegistrationParams m_rp;
	VS_SimpleStr	m_serverName;
	VS_SimpleStr	m_SM;
	VS_SimpleStr	m_Key;
	VS_SimpleStr	m_PrivateKey;
	VS_RouterMessage*	m_recvMess;
	VS_RoutersWatchdog*	m_watchdog = nullptr;

	VS_SimpleStr		m_server_version;
	VS_SimpleStr		m_registration_service;

	bool m_isCertVerifyed;
	unsigned long m_noCertTick;
	unsigned long m_lastCertCheck;
	unsigned long m_check_cert_period;
	std::function<void(const char *)> m_call_cfg_correcto_data_handler;

	void RegisterServer();
	void RegFromFile();
	bool ApplyNewCert(VS_Container &cnt, const bool offline_reg, int &err, std::string &err_str);
	bool GetLicensingInfo(VS_Container& cnt);
	bool ProcessLicense(VS_RegistryKey& l_root,uint64_t id,long type,const void* data,int size);

	void CertificateUpdate();
	bool GenerateCertRequest(string_view server_name, string_view organization_name, string_view country, string_view contact_person, string_view contact_email, VS_SimpleStr &certReq);
	void RegisterServer_Method(VS_Container &cnt);
	void CertificateUpdate_Method(VS_Container &cnt);
	void		PrepareCertDataToLog(VS_Certificate* cert, VS_SimpleStr &str);
	void HandleCallCfgCorrectorData(const char *data);
public:
	VS_VerificationService(vs::RegistrationParams&& rp);
	virtual ~VS_VerificationService();
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	void AsyncDestroy() override;
	bool Timer(unsigned long tickcount) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	// VS_PointConditions
	bool OnPointConnected_Event(const VS_PointParams* prm) override;
	bool OnPointDisconnected_Event(const VS_PointParams* prm) override;

	void ServerVerificationFailed();
	void SetComponents(VS_RoutersWatchdog* watchdog, const char * reg_service = REGISTRATION_SRV, const char *ver = NULL);
	void SetCallCfgCorrectorDataHandler(const std::function<void(const char*)> &handler);
};