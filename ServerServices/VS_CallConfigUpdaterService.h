#pragma once

#include <chrono>
#include <cstdint>

#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "../common/std/cpplib/VS_SimpleStr.h"
#include "../common/std/cpplib/VS_Protocol.h"

class VS_CallConfigUpdaterService :
	public VS_TransportRouterServiceHelper
{
public:
	VS_CallConfigUpdaterService();
	virtual ~VS_CallConfigUpdaterService();

	void SetConfig(const long mode = 0, const char *reg_service = REGISTRATION_SRV);

	bool Init(const char* our_endpoint, const char *our_service, const bool permittedAll) override;
	bool Timer(unsigned long tickcount) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

private:
	bool InitiateUpdate(void);

	static const unsigned long CHECK_GRANULARITY_TIMEOUT_MILLIS = 5 * 1000;
	static const uint64_t UPDATE_CHECK_TIMEOUT_SEC = 1 * 24 * 60 * 60; // once a day

	long m_srv_start_mode;
	VS_SimpleStr m_registration_service;
	VS_SimpleStr m_our_endpoint;
	VS_SimpleStr	m_SM;
};

